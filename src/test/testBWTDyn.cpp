/*
    libmaus
    Copyright (C) 2009-2013 German Tischler
    Copyright (C) 2011-2013 Genome Research Limited

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <libmaus/wavelet/DynamicWaveletTree.hpp>
#include <libmaus/cumfreq/SlowCumFreq.hpp>
#include <libmaus/huffman/huffman.hpp>
#include <libmaus/eta/LinearETA.hpp>
#include <libmaus/math/bitsPerNum.hpp>

#include <fstream>
#include <vector>

uint64_t getFileLength(std::string const & textfilename)
{
	std::ifstream istr(textfilename.c_str(),std::ios::binary);
	
	if ( ! istr.is_open() )
		throw std::runtime_error("Failed to open file.");
	
	istr.seekg(0,std::ios_base::end);
	uint64_t const n = istr.tellg();

	return n;
}

::libmaus::autoarray::AutoArray<uint64_t> getSymbolFrequencies(std::string const & textfilename)
{
	uint64_t const n = getFileLength(textfilename);

	std::ifstream istr(textfilename.c_str(),std::ios::binary);
	
	if ( ! istr.is_open() )
		throw std::runtime_error("Failed to open file.");

	::libmaus::autoarray::AutoArray<uint64_t> F(256);
	for ( unsigned int i = 0; i < 256; ++i )
		F[i] = 0;

	uint64_t const s = 1024*1024;
	::libmaus::autoarray::AutoArray<uint8_t> buf(s,false);

	for ( uint64_t j = 0; j < (n + (s-1))/s; ++j )
	{
		uint64_t const a = j*s;
		uint64_t const b = std::min( a+s , n );
		uint64_t const c = b-a;
		
		istr.read ( reinterpret_cast<char *>(buf.get()), c );
		
		for ( uint64_t i = 0; i < c; ++i )
			F[ buf[i] ] ++;
	}
	
	assert ( static_cast<int64_t>(istr.tellg()) == static_cast<int64_t>(n) );
		
	return F;
}

void reorderHuffmanTreeZero(::libmaus::huffman::HuffmanTreeNode * hnode)
{

	std::map < ::libmaus::huffman::HuffmanTreeNode *, ::libmaus::huffman::HuffmanTreeInnerNode * > hparentmap;
	hnode->fillParentMap(hparentmap);
	std::map < int64_t, ::libmaus::huffman::HuffmanTreeLeaf * > hleafmap;
	hnode->fillLeafMap(hleafmap);

	::libmaus::huffman::HuffmanTreeNode * hcur = hleafmap.find(-1)->second;
	
	while ( hparentmap.find(hcur) != hparentmap.end() )
	{
		::libmaus::huffman::HuffmanTreeInnerNode * hparent = hparentmap.find(hcur)->second;
		
		if ( hcur == hparent->right )
			std::swap(hparent->left, hparent->right);
		
		hcur = hparent;
	}
}

void applyRankMap(::libmaus::huffman::HuffmanTreeNode * hnode, std::map<int64_t,uint64_t> const & rankmap)
{
	std::map < int64_t, ::libmaus::huffman::HuffmanTreeLeaf * > hleafmap;
	hnode->fillLeafMap(hleafmap);

	for ( std::map < int64_t, ::libmaus::huffman::HuffmanTreeLeaf * >::iterator ita = hleafmap.begin(); ita != hleafmap.end(); ++ita )
	{
		int const srcsym = ita->first;
		uint64_t const dstsym = rankmap.find(srcsym)->second;
		assert ( ita->second->symbol == srcsym );
		ita->second->symbol = dstsym;
	}
}

void printCodeLength( ::libmaus::huffman::HuffmanTreeNode const * const hnode, uint64_t const * const F)
{
	::libmaus::huffman::EncodeTable<4> enctable(hnode);
	
	uint64_t l = 0;
	uint64_t n = 0;
	for ( uint64_t i = 0; i < 256; ++i )
		if ( F[i] )
		{
			l += F[i] * enctable[i].second;
			n += F[i];
		}
			
	std::cerr << "Total code length " << l << " average bits per symbol " << static_cast<double>(l) / n << std::endl;
}

void computeBWT(std::string const & textfilename, std::ostream & output)
{
	unsigned int const k = 8;
	unsigned int const w = 32;
	
	uint64_t const n = getFileLength(textfilename);
	if ( ! n )
		return;

	::libmaus::autoarray::AutoArray<uint64_t> F = getSymbolFrequencies(textfilename);
		
	std::map<int64_t,uint64_t> freq;
	for ( unsigned int i = 0; i < 256; ++i )
		if ( F[i] )
			freq[i] = F[i];
	freq[-1] = 1;

	double ent = 0;
	for ( unsigned int i = 0; i < 256; ++i )
		if ( F[i] )
		{
			ent -= F[i] * (log(static_cast<double>(F[i])/n)/log(2));
		}
	std::cerr << "Entropy of text is " << ent/n << std::endl;

	::libmaus::util::shared_ptr < ::libmaus::huffman::HuffmanTreeNode >::type ahnode = ::libmaus::huffman::HuffmanBase::createTree( freq );
	
	printCodeLength(ahnode.get(), F.get());
	
	reorderHuffmanTreeZero(ahnode.get());

	std::map<int64_t,uint64_t> rankmap = ::libmaus::huffman::EncodeTable<4>(ahnode.get()).symsOrderedByCodeMap();
	
	applyRankMap(ahnode.get(), rankmap);
	
	::libmaus::autoarray::AutoArray<uint64_t> M(256);
	::libmaus::autoarray::AutoArray<uint64_t> R(257);
	uint64_t const a = rankmap.size();
	for ( std::map<int64_t,uint64_t>::const_iterator ita = rankmap.begin(); ita != rankmap.end(); ++ita )
	{
		// std::cerr << ita->first << " " << ita->second << std::endl;
		if ( ita->first >= 0 )
		{
			M[ita->first] = ita->second;
			R[ita->second] = ita->first;
		}
	}

	unsigned int const bpn = ::libmaus::math::bitsPerNum(a-1);

	::libmaus::wavelet::DynamicWaveletTree<k,w> B(bpn);
	::libmaus::cumfreq::SlowCumFreq scf(a);
	
	uint64_t const s = 1024*1024;
	::libmaus::autoarray::AutoArray<uint8_t> buf(s,false);
	uint64_t const numblocks = (n + (s-1))/s;

	std::ifstream istr(textfilename.c_str(),std::ios::binary);
	
	if ( ! istr.is_open() )
	{
		std::cerr << "Failed to open file " << textfilename << std::endl;
		throw std::runtime_error("Failed to open file.");
	}

	istr.seekg ( 0 , std::ios_base::end );
	
	uint64_t p = 0;
	for ( uint64_t b = 0; b < numblocks; ++b )
	{
		uint64_t const low = (numblocks-b-1)*s;
		uint64_t const high = std::min(low+s,n);
		uint64_t const len = high-low;
		
		istr.seekg ( - static_cast<int64_t>(len), std::ios_base::cur );
		
		std::cerr << "Reading len=" << len << " at position " << istr.tellg() << std::endl;
		istr.read ( reinterpret_cast<char *>(buf.get()), len );
		
		istr.seekg ( - static_cast<int64_t>(len), std::ios_base::cur );
		
		std::reverse ( buf.get(), buf.get() + len );
		
		for ( uint64_t i = 0; i < len; ++i )
			buf[i] = M[buf[i]];
			
		for ( uint64_t i = 0; i < len; ++i )
		{
			uint64_t const c = buf[i];
			uint64_t const f = scf[c]+1; scf.inc(c);
			p = B.insertAndRank(c,p) - 1 + f;
		}
	}
	B.insert ( 0, p );
	scf.inc(0);

	::libmaus::autoarray::AutoArray<uint8_t> obuf(n,false);	

	// follow LF to skip terminator
	p = scf[ B[p] ] + (p ? B.rank(B[p],p-1) : 0);

	for ( uint64_t i = 0; i < n; ++i )
	{
		uint64_t const sym = B[p];
		p = scf[sym] + (p ? B.rank(sym,p-1) : 0);
		obuf[n-i-1] = R[sym];
	}
	output.write ( reinterpret_cast<char const *>(obuf.get()), n );
}

int main(int argc, char * argv[])
{
	if ( argc < 3 )
	{
		std::cerr << "usage: " << argv[0] << " <text> <out>" << std::endl;
		return EXIT_FAILURE;
	}
	
	std::string const textfilename = argv[1];
	std::string const outputfilename = argv[2];
	
	try
	{
		std::ofstream out(outputfilename.c_str(), std::ios::binary);
		computeBWT(textfilename,out);
		out.close();
		
	}
	catch(std::exception const & ex)
	{
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
