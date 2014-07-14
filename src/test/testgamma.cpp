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
#include <iostream>
#include <cassert>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <libmaus/timing/RealTimeClock.hpp>
#include <libmaus/gamma/GammaEncoder.hpp>
#include <libmaus/gamma/GammaDecoder.hpp>

#include <libmaus/gamma/SparseGammaGapEncoder.hpp>
#include <libmaus/gamma/SparseGammaGapDecoder.hpp>
#include <libmaus/gamma/SparseGammaGapMerge.hpp>
#include <libmaus/gamma/SparseGammaGapFile.hpp>
#include <libmaus/gamma/SparseGammaGapFileSet.hpp>
#include <libmaus/gamma/SparseGammaGapFileLevelSet.hpp>

template<typename T>
struct VectorPut : public std::vector<T>
{
	VectorPut() {}
	void put(T const v)
	{
		std::vector<T>::push_back(v);
	}
};

struct CountPut
{
	uint64_t cnt;

	CountPut()
	: cnt(0)
	{
	
	}
	
	template<typename T>
	void put(T const)
	{
		++cnt;	
	}
};

template<typename T>
struct VectorGet
{
	typename std::vector<T>::const_iterator it;
	
	VectorGet(typename std::vector<T>::const_iterator rit)
	: it(rit)
	{}
	
	uint64_t get()
	{
		return *(it++);
	}
};

void testRandom(unsigned int const n)
{
	srand(time(0));
	std::vector<uint64_t> V(n);
	for ( uint64_t i = 0; i <n; ++i )
		V[i] = rand();

	::libmaus::timing::RealTimeClock rtc; 
	
	rtc.start();
	CountPut CP;
	::libmaus::gamma::GammaEncoder< CountPut > GCP(CP);	
	for ( uint64_t i = 0; i < n; ++i )
		GCP.encode(V[i]);
	GCP.flush();
	double const cencsecs = rtc.getElapsedSeconds();
	std::cerr << "[V] count encoded " << n << " numbers in time " << cencsecs 
		<< " rate " << (n / cencsecs)/(1000*1000) << " m/s"
		<< " output words " << CP.cnt
		<< std::endl;
	
	VectorPut<uint64_t> VP;
	rtc.start();
	::libmaus::gamma::GammaEncoder< VectorPut<uint64_t> > GE(VP);	
	for ( uint64_t i = 0; i < n; ++i )
		GE.encode(V[i]);
	GE.flush();
	double const encsecs = rtc.getElapsedSeconds();
	std::cerr << "[V] encoded " << n << " numbers to dyn growing vector in time " << encsecs 
		<< " rate " << (n / encsecs)/(1000*1000) << " m/s"
		<< std::endl;

	rtc.start();
	VectorGet<uint64_t> VG(VP.begin());
	::libmaus::gamma::GammaDecoder < VectorGet<uint64_t> > GD(VG);
	bool ok = true;
	for ( uint64_t i = 0; ok && i < n; ++i )
	{
		uint64_t const v = GD.decode();
		ok = ok && (v==V[i]);
		if ( ! ok )
		{
			std::cerr << "expected " << V[i] << " got " << v << std::endl;
		}
	}
	double const decsecs = rtc.getElapsedSeconds();
	std::cerr << "[V] decoded " << n << " numbers in time " << decsecs
		<< " rate " << (n / decsecs)/(1000*1000) << " m/s"
		<< std::endl;
	
	if ( ok )
	{
		std::cout << "Test of gamma coding with " << n << " random numbers ok." << std::endl;
	}
	else
	{
		std::cout << "Test of gamma coding with " << n << " random numbers failed." << std::endl;
	}
}

void testLow()
{
	unsigned int const n = 127;
	VectorPut<uint64_t> VP;
	::libmaus::gamma::GammaEncoder< VectorPut<uint64_t> > GE(VP);	
	for ( uint64_t i = 0; i < n; ++i )
		GE.encode(i);
	GE.flush();
	
	VectorGet<uint64_t> VG(VP.begin());
	::libmaus::gamma::GammaDecoder < VectorGet<uint64_t> > GD(VG);
	bool ok = true;
	for ( uint64_t i = 0; ok && i < n; ++i )
		ok = ok && ( GD.decode() == i );

	if ( ok )
	{
		std::cout << "Test of gamma coding with numbers up to 126 ok." << std::endl;
	}
	else
	{
		std::cout << "Test of gamma coding with numbers up to 126 failed." << std::endl;
	}
}

#include <libmaus/gamma/GammaGapEncoder.hpp>
#include <libmaus/huffman/IndexDecoderDataArray.hpp>
#include <libmaus/gamma/GammaGapDecoder.hpp>
#include <libmaus/util/TempFileRemovalContainer.hpp>

void testgammagap()
{
	unsigned int n = 512*1024+199481101;
	std::vector<uint64_t> V (n);
	std::vector<uint64_t> V2(n);
	for ( uint64_t i = 0; i < V.size(); ++i )
	{
		V[i] = i & 0xFFull;
		V2[i] = rand() % 0xFFull;
	}

	std::string const fn("tmpfile");
	std::string const fn2("tmpfile2");
	std::string const fnm("tmpfile.merged");

	::libmaus::util::TempFileRemovalContainer::setup();
	::libmaus::util::TempFileRemovalContainer::addTempFile(fn);
	::libmaus::util::TempFileRemovalContainer::addTempFile(fn2);
	::libmaus::util::TempFileRemovalContainer::addTempFile(fnm);

	::libmaus::gamma::GammaGapEncoder GGE(fn);
	GGE.encode(V.begin(),V.end());
	::libmaus::gamma::GammaGapEncoder GGE2(fn2);
	GGE2.encode(V2.begin(),V2.end());
	
	::libmaus::huffman::IndexDecoderData IDD(fn);
	
	::libmaus::gamma::GammaGapDecoder GGD(std::vector<std::string>(1,fn));
	
	bool ok = true;
	for ( uint64_t i = 0; i < n; ++i )
	{
		uint64_t const v = GGD.decode();
		ok = ok && (v == V[i]);
	}
	std::cout << "decoding " << (ok ? "ok" : "fail") << std::endl;

	std::vector < std::vector<std::string> > merin;
	merin.push_back(std::vector<std::string>(1,fn));
	merin.push_back(std::vector<std::string>(1,fn2));
	::libmaus::gamma::GammaGapEncoder::merge(merin,fnm);
}

#include <libmaus/gamma/GammaRLEncoder.hpp>
#include <libmaus/gamma/GammaRLDecoder.hpp>

void testgammarl()
{
	srand(time(0));
	unsigned int n = 128*1024*1024;
	unsigned int n2 = 64*1024*1024;
	std::vector<uint64_t> V (n);
	std::vector<uint64_t> V2 (n2);
	std::vector<uint64_t> Vcat;
	unsigned int const albits = 3;
	uint64_t const almask = (1ull << albits)-1;
	
	for ( uint64_t i = 0; i < V.size(); ++i )
	{
		V[i]  = rand() & almask;
		Vcat.push_back(V[i]);
	}
	for ( uint64_t i = 0; i < V2.size(); ++i )
	{
		V2[i] = rand() & almask;
		Vcat.push_back(V2[i]);
	}

	std::string const fn("tmpfile");
	std::string const fn2("tmpfile2");
	std::string const fn3("tmpfile3");
	::libmaus::util::TempFileRemovalContainer::setup();
	::libmaus::util::TempFileRemovalContainer::addTempFile(fn);
	::libmaus::util::TempFileRemovalContainer::addTempFile(fn2);
	::libmaus::util::TempFileRemovalContainer::addTempFile(fn3);

	::libmaus::gamma::GammaRLEncoder GE(fn,albits,V.size(),256*1024);	
	for ( uint64_t i = 0; i < V.size(); ++i )
		GE.encode(V[i]);
	GE.flush();

	::libmaus::gamma::GammaRLEncoder GE2(fn2,albits,V2.size(),256*1024);
	for ( uint64_t i = 0; i < V2.size(); ++i )
		GE2.encode(V2[i]);
	GE2.flush();

	#if 0
	::libmaus::huffman::IndexDecoderData IDD(fn);
	for ( uint64_t i = 0; i < IDD.numentries+1; ++i )
		std::cerr << IDD.readEntry(i) << std::endl;	
	#endif

	::libmaus::gamma::GammaRLDecoder GD(std::vector<std::string>(1,fn));
	assert ( GD.getN() == n );

	for ( uint64_t i = 0; i < n; ++i )
		assert ( GD.decode() == static_cast<int64_t>(V[i]) );
	
	uint64_t const off = n / 2 + 1031;
	::libmaus::gamma::GammaRLDecoder GD2(std::vector<std::string>(1,fn),off);
	for ( uint64_t i = off; i < n; ++i )
		assert ( GD2.decode() == static_cast<int64_t>(V[i]) );
		
	std::vector<std::string> fnv;
	fnv.push_back(fn);
	fnv.push_back(fn2);
	::libmaus::gamma::GammaRLEncoder::concatenate(fnv,fn3);
	
	for ( uint64_t off = 0; off < Vcat.size(); off += 18521 )
	{
		::libmaus::gamma::GammaRLDecoder GD3(std::vector<std::string>(1,fn3),off);	
		assert ( GD3.getN() == Vcat.size() );
		
		for ( uint64_t i = 0; i < std::min(static_cast<uint64_t>(1024ull),Vcat.size()-off); ++i )
			assert ( GD3.decode() == static_cast<int64_t>(Vcat[off+i]) );
	}

	remove ( fn.c_str() );
	remove ( fn2.c_str() );
	remove ( fn3.c_str() );	
}

void testgammasparse()
{
	std::ostringstream o0;
	libmaus::gamma::SparseGammaGapEncoder SE0(o0);
	std::ostringstream o1;
	libmaus::gamma::SparseGammaGapEncoder SE1(o1);
	
	SE0.encode(4, 7);
	SE0.encode(6, 3);
	SE0.term();
	
	SE1.encode(0, 1);
	SE1.encode(2, 5);
	SE1.encode(6, 2);
	SE1.encode(8, 7);
	SE1.term();
	
	std::cerr << "o0.size()=" << o0.str().size() << std::endl;
	std::cerr << "o1.size()=" << o1.str().size() << std::endl;
	
	std::istringstream i0(o0.str());
	libmaus::gamma::SparseGammaGapDecoder SD0(i0);
	std::istringstream i1(o1.str());
	libmaus::gamma::SparseGammaGapDecoder SD1(i1);
	
	for ( uint64_t i = 0; i < 10; ++i )
		std::cerr << SD0.decode() << ";";
	std::cerr << std::endl;
	for ( uint64_t i = 0; i < 10; ++i )
		std::cerr << SD1.decode() << ";";
	std::cerr << std::endl;

	std::istringstream mi0(o0.str());
	std::istringstream mi1(o1.str());
	std::ostringstream mo;
	
	libmaus::gamma::SparseGammaGapMerge::merge(mi0,mi1,mo);
	
	std::istringstream mi(mo.str());
	libmaus::gamma::SparseGammaGapDecoder SDM(mi);

	for ( uint64_t i = 0; i < 10; ++i )
		std::cerr << SDM.decode() << ";";
	std::cerr << std::endl;
}


void testsparsegammalevelmerge()
{
	libmaus::util::TempFileNameGenerator tmpgen("tmp",3);
	libmaus::gamma::SparseGammaGapFileLevelSet SGGF(tmpgen);
	std::map<uint64_t,uint64_t> refM;
	
	for ( uint64_t i = 0; i < 25;  ++i )
	{
		std::string const fn = tmpgen.getFileName();
		libmaus::aio::CheckedOutputStream COS(fn);
		libmaus::gamma::SparseGammaGapEncoder SGE(COS);
		
		SGE.encode(2*i,i+1);   refM[2*i]   += (i+1);
		SGE.encode(2*i+2,i+1); refM[2*i+2] += (i+1);
		SGE.encode(2*i+4,i+1); refM[2*i+4] += (i+1);
		SGE.term();
		
		SGGF.addFile(fn);
	}
	
	std::string const ffn = tmpgen.getFileName();
	SGGF.merge(ffn);
	
	libmaus::aio::CheckedInputStream CIS(ffn);
	libmaus::gamma::SparseGammaGapDecoder SGGD(CIS);
	for ( uint64_t i = 0; i < 60; ++i )
	{
		uint64_t dv = SGGD.decode();
		
		std::cerr << dv;
		if ( refM.find(i) != refM.end() )
		{
			std::cerr << "(" << refM.find(i)->second << ")";
			assert ( refM.find(i)->second == dv );
		}
		else
		{
			std::cerr << "(0)";
			assert ( dv == 0 );
		}
		std::cerr << ";";
	}
	std::cerr << std::endl;
	
	remove(ffn.c_str());
}


#include <libmaus/gamma/SparseGammaGapBlockEncoder.hpp>
#include <libmaus/gamma/SparseGammaGapConcatDecoder.hpp>

void testSparseGammaConcat()
{
	for ( uint64_t An = 1; An <= 512; ++An )
		for ( uint64_t z = 0; z < 4; ++z )
		{
			// set up array with random numbers
			uint64_t const Amod = (An+1)/2 + (rand() % An);
			libmaus::autoarray::AutoArray<uint64_t> A(An,false);
			for ( uint64_t i = 0; i < A.size(); ++i )
				A[i] = rand() % Amod;

			// file name prefix
			std::string const fnpref = "tmp_g";
			
			// encode array
			std::vector<std::string> concfn = libmaus::gamma::SparseGammaGapBlockEncoder::encodeArray(
				&A[0], &A[An], fnpref, 7 /* parts */, 5 /* block size */);
				
			// store data in map for reference
			std::map<uint64_t,uint64_t> M;
			for ( uint64_t i = 0; i < An; ++i )
				M[A[i]]++;

			uint64_t maxv = 0;		
			std::vector<uint64_t> const splitkeys = libmaus::gamma::SparseGammaGapFileIndexMultiDecoder::getSplitKeys(
				concfn,concfn,/* Amod, */8,maxv);
			
			#if 0
			std::cerr << "splitkeys: ";
			for ( uint64_t i = 0; i < splitkeys.size(); ++i )
				std::cerr << splitkeys[i] << ";";
			std::cerr << std::endl;
			
			for ( std::map<uint64_t,uint64_t>::const_iterator ita = M.begin(); ita != M.end(); ++ita )
				std::cerr << "(k" << ita->first << ",v=" << ita->second << ")";
			std::cerr << std::endl;
			#endif
		
			// test reading back starting from beginning
			libmaus::gamma::SparseGammaGapFileIndexMultiDecoder concindex(concfn);	
			libmaus::gamma::SparseGammaGapConcatDecoder SGGCD(concfn);
			int64_t prevkey = -1;
			for ( uint64_t i = 0; i < Amod; ++i )
			{
				uint64_t const v = SGGCD.decode();
				// std::cerr << i << "\t" << v << "\t" << M[i] << std::endl;
				assert ( v == M[i] );
				
				bool const prevok = ( prevkey == libmaus::gamma::SparseGammaGapConcatDecoder::getPrevKey(concindex,i) );
				
				#if 0
				if ( ! prevok )
				{
					std::cerr << "expected " << prevkey << " got " << libmaus::gamma::SparseGammaGapConcatDecoder::getPrevKey(concindex,i) << std::endl;
				}
				#endif
				
				assert ( prevok );
				
				if ( v )
					prevkey = i;
			}
			
			// test reading back from given starting position
			for ( uint64_t j = 0; j < Amod; ++j )
			{
				// std::cerr << "*** j=" << j << " ***" << std::endl;
				libmaus::gamma::SparseGammaGapConcatDecoder SGGCD(concfn,j);
			
				for ( uint64_t i = j; i < Amod; ++i )
				{
					uint64_t const v = SGGCD.decode();
					if ( v != M[i] )
						std::cerr << j << "\t" << i << "\t" << v << "\t" << M[i] << std::endl;
					assert ( v == M[i] );
				}
			}
			
			// remove files
			for ( uint64_t i = 0; i < concfn.size(); ++i )
				remove(concfn[i].c_str());
			
			std::cerr << An << "\t" << z+1 << std::endl;
		}
}

void testSparseGammaIndexing()
{
	std::stringstream datastr;
	std::stringstream indexstr;
	libmaus::gamma::SparseGammaGapBlockEncoder SGGBE(datastr,indexstr,-1 /* prevkey */,2);
	SGGBE.encode(2,3);
	SGGBE.encode(7,2);
	SGGBE.encode(11,1);
	SGGBE.encode(12,5);
	SGGBE.encode(13,4);
	SGGBE.term();
	
	std::istringstream istr(datastr.str());
	libmaus::gamma::SparseGammaGapFileIndexDecoder SGGFID(istr);
	
	std::cerr << SGGFID;
	
	std::cerr << std::string(80,'-') << std::endl;
			
	// std::cerr << SGGFID.getBlockIndex(1) << std::endl;
	std::cerr << SGGFID.getBlockIndex(2) << std::endl;
	std::cerr << SGGFID.getBlockIndex(7) << std::endl;
	std::cerr << SGGFID.getBlockIndex(11) << std::endl;
	std::cerr << SGGFID.getBlockIndex(12) << std::endl;
	std::cerr << SGGFID.getBlockIndex(13) << std::endl;
	std::cerr << SGGFID.getBlockIndex(512) << std::endl;
}


void testSparseGammaGapMergingSmall(uint64_t * A, uint64_t const An, uint64_t * B, uint64_t const Bn)
{
	// file name prefix
	std::string const fnprefa = "tmp_a";
	std::string const fnprefb = "tmp_b";
	std::string const fnout = "tmp_o";
			
	// encode array
	std::vector<std::string> concafn = libmaus::gamma::SparseGammaGapBlockEncoder::encodeArray(&A[0], &A[An], fnprefa, 7 /* parts */, 2 /* block size */);
	std::vector<std::string> concbfn = libmaus::gamma::SparseGammaGapBlockEncoder::encodeArray(&B[0], &B[Bn], fnprefb, 7 /* parts */, 2 /* block size */);
	
	// libmaus::gamma::SparseGammaGapMerge::merge(concafn,concbfn,0,std::numeric_limits<uint64_t>::max(),fnout);
	std::vector<std::string> const concofn = libmaus::gamma::SparseGammaGapMerge::merge(concafn,concbfn,fnout,10);

	#if 0
	for ( uint64_t i = 0; i < concafn.size(); ++i )
		std::cerr << "concafn[i]=" << concafn[i] << std::endl;
	for ( uint64_t i = 0; i < concbfn.size(); ++i )
		std::cerr << "concbfn[i]=" << concbfn[i] << std::endl;
	for ( uint64_t i = 0; i < concofn.size(); ++i )
		std::cerr << "concofn[i]=" << concofn[i] << std::endl;
	#endif
	
	std::map<uint64_t,uint64_t> M;
	for ( uint64_t i = 0; i < An; ++i ) M[A[i]]++;
	for ( uint64_t i = 0; i < Bn; ++i ) M[B[i]]++;
	uint64_t const maxv = M.size() ? M.rbegin()->first : 0;
	
	libmaus::gamma::SparseGammaGapConcatDecoder dec(concofn);
	
	for ( uint64_t i = 0; i <= maxv; ++i )
	{
		uint64_t const v = dec.decode();
		uint64_t const Mi = (M.find(i) != M.end()) ? M.find(i)->second : 0;
		
		if ( v != Mi )
			std::cerr << i << "\t" << v << "\t" << Mi << std::endl;
		
		assert ( v == Mi );
	}

	// remove files
	for ( uint64_t i = 0; i < concafn.size(); ++i )
		remove(concafn[i].c_str());
	for ( uint64_t i = 0; i < concbfn.size(); ++i )
		remove(concbfn[i].c_str());
	for ( uint64_t i = 0; i < concofn.size(); ++i )
		remove(concofn[i].c_str());
}

libmaus::autoarray::AutoArray<uint64_t> randomArray(uint64_t const n, uint64_t const klow, uint64_t const khigh)
{
	if ( ! n )
		return libmaus::autoarray::AutoArray<uint64_t>(0);
	
	assert ( khigh-klow );
	
	libmaus::autoarray::AutoArray<uint64_t> A(n,false);
	
	for ( uint64_t i = 0; i < n; ++i )
		A[i] = klow + ( libmaus::random::Random::rand64() % (khigh-klow) );
	
	return A;
}

void testSparseGammaGapMergingRandom(
	uint64_t const nl,
	uint64_t const kll,
	uint64_t const klh,
	uint64_t const nr,
	uint64_t const krl,
	uint64_t const krh
)
{
	libmaus::autoarray::AutoArray<uint64_t> Al = randomArray(nl,kll,klh);
	libmaus::autoarray::AutoArray<uint64_t> Ar = randomArray(nr,krl,krh);
	testSparseGammaGapMergingSmall(Al.begin(),Al.size(),Ar.begin(),Ar.size());
}

void testSparseGammaGapMergingRandom()
{
	// test full overlapping
	uint64_t const loops = 32;
	for ( uint64_t i = 0; i < loops; ++i )
	{
		uint64_t const k0 = libmaus::random::Random::rand64() % 1024;
		uint64_t const k1 = k0 + (libmaus::random::Random::rand64() % 1024+1);
		uint64_t const nl = libmaus::random::Random::rand64() % 1024;
		uint64_t const nr = libmaus::random::Random::rand64() % 1024;
		
		testSparseGammaGapMergingRandom(nl,k0,k1,nr,k0,k1);
		testSparseGammaGapMergingRandom(nr,k0,k1,nr,k0,k1);
		
		std::cerr << "(" << nl << "," << nr << "," << k0 << "," << k1 << ")";
	}
	std::cerr << std::endl;

	// test part overlapping
	for ( uint64_t i = 0; i < loops; ++i )
	{
		uint64_t const k0 = libmaus::random::Random::rand64() % 1024;
		uint64_t const k1 = k0 + (libmaus::random::Random::rand64() % 1024+1);
		uint64_t const ksl = k1-(k1-k0)/2;
		uint64_t const k2 = k1 + (libmaus::random::Random::rand64() % 1024+1);
		uint64_t const ksr = k2-(k2-k1)/2;
		uint64_t const nl = libmaus::random::Random::rand64() % 1024;
		uint64_t const nr = libmaus::random::Random::rand64() % 1024;
		
		testSparseGammaGapMergingRandom(nl,k0,ksl,nr,ksr,k2);
		testSparseGammaGapMergingRandom(nr,ksr,k2,nl,k0,ksl);
		
		std::cerr << "(" << nl << "," << nr << "," << k0 << "," << k1 << "," << k2 << ")";
	}
	std::cerr << std::endl;

	// test non overlapping
	for ( uint64_t i = 0; i < loops; ++i )
	{
		uint64_t const k0 = libmaus::random::Random::rand64() % 1024;
		uint64_t const k1 = k0 + (libmaus::random::Random::rand64() % 1024+1);
		uint64_t const k2 = k1 + (libmaus::random::Random::rand64() % 1024+1);
		uint64_t const nl = libmaus::random::Random::rand64() % 1024;
		uint64_t const nr = libmaus::random::Random::rand64() % 1024;
		
		testSparseGammaGapMergingRandom(nl,k0,k1,nr,k1,k2);
		testSparseGammaGapMergingRandom(nr,k1,k2,nl,k0,k1);
		
		std::cerr << "(" << nl << "," << nr << "," << k0 << "," << k1 << "," << k2 << ")";
	}
	std::cerr << std::endl;

}

void testSparseGammaGapMergingSmall()
{
	uint64_t const A0[] = { 1,6,1,7,21,1,6,6,7,4,42,14,16,25,28,100,83,70,75 };
	uint64_t const A1[] = { 1,3,50,52,75,77,1000,1002,2000,3000 };
	uint64_t const A2[] = { 11,13,60,62,82,83 };
	uint64_t const A3[] = { };
	uint64_t const *A[] = { &A0[0], &A1[0], &A2[0], &A3[0], 0 };
	uint64_t const An[] = { sizeof(A0)/sizeof(A0[0]), sizeof(A1)/sizeof(A1[0]), sizeof(A2)/sizeof(A2[0]), sizeof(A3)/sizeof(A3[0]), 0 };
	
	uint64_t const B0[] = { 1,6,1,7,21,1,6,6,7,5,43,13,18,24,29,95,86,72,77 };
	uint64_t const B1[] = { 11,13,60,62,82,83 };
	uint64_t const B2[] = { 1000,1002,1004,1006,1008,1010,1012, 1014, 1017, 1020 };
	uint64_t const B3[] = { };
	uint64_t const B4[] = { 1,6,1,7,21,1,6,6,7,4,42,14,16,25,28,100,83,70,75 };
	uint64_t const *B[] = { &B0[0], &B1[0], &B2[2], &B3[0], &B4[0], 0 };
	uint64_t const Bn[] = { sizeof(B0)/sizeof(B0[0]), sizeof(B1)/sizeof(B1[0]), sizeof(B2)/sizeof(B2[0]), sizeof(B3)/sizeof(B3[0]), sizeof(B4)/sizeof(B4[0]), 0 };
	
	for ( uint64_t a = 0; A[a]; ++a )
		for ( uint64_t b = 0; B[b]; ++b )
		{
			libmaus::autoarray::AutoArray<uint64_t> At(An[a],false);
			libmaus::autoarray::AutoArray<uint64_t> Bt(Bn[b],false);
			std::copy(A[a],A[a]+An[a],At.begin());
			std::copy(B[b],B[b]+Bn[b],Bt.begin());			
			testSparseGammaGapMergingSmall(At.begin(),An[a],Bt.begin(),Bn[b]);
		}
}

#include <libmaus/gamma/SparseGammaGapMultiFileSet.hpp>

void testsparsegammamerge()
{
	libmaus::util::TempFileNameGenerator tmpgen("tmp",3);
	libmaus::gamma::SparseGammaGapFileSet SGGF(tmpgen);
	std::map<uint64_t,uint64_t> refM;
	
	for ( uint64_t i = 0; i < 25;  ++i )
	{
		std::string const fn = tmpgen.getFileName();
		libmaus::aio::CheckedOutputStream COS(fn);
		libmaus::gamma::SparseGammaGapEncoder SGE(COS);
		
		SGE.encode(2*i,i+1);   refM[2*i]   += (i+1);
		SGE.encode(2*i+2,i+1); refM[2*i+2] += (i+1);
		SGE.encode(2*i+4,i+1); refM[2*i+4] += (i+1);
		SGE.term();
		
		SGGF.addFile(fn);
	}
	
	std::string const ffn = tmpgen.getFileName();
	SGGF.merge(ffn);
	
	libmaus::aio::CheckedInputStream CIS(ffn);
	libmaus::gamma::SparseGammaGapDecoder SGGD(CIS);
	for ( uint64_t i = 0; i < 60; ++i )
	{
		uint64_t dv = SGGD.decode();
		
		std::cerr << dv;
		if ( refM.find(i) != refM.end() )
		{
			std::cerr << "(" << refM.find(i)->second << ")";
			assert ( refM.find(i)->second == dv );
		}
		else
		{
			std::cerr << "(0)";
			assert ( dv == 0 );
		}
		std::cerr << ";";
	}
	std::cerr << std::endl;
	
	remove(ffn.c_str());
}


void testsparsegammamultimerge()
{
	libmaus::util::TempFileNameGenerator tmpgen("tmp",3);
	libmaus::gamma::SparseGammaGapMultiFileSet SGGF(tmpgen,4);
	std::map<uint64_t,uint64_t> refM;
	
	for ( uint64_t i = 0; i < 25;  ++i )
	{
		std::string const fn = tmpgen.getFileName();
		std::string const indexfn = fn+".idx";
		libmaus::aio::CheckedOutputStream COS(fn);
		libmaus::aio::CheckedInputOutputStream indexCIOS(indexfn);
		libmaus::gamma::SparseGammaGapBlockEncoder SGE(COS,indexCIOS);
		remove(indexfn.c_str());
		
		SGE.encode(2*i,i+1);   refM[2*i]   += (i+1);
		SGE.encode(2*i+2,i+1); refM[2*i+2] += (i+1);
		SGE.encode(2*i+4,i+1); refM[2*i+4] += (i+1);
		SGE.term();
		
		SGGF.addFile(fn);
	}
	
	std::string const ffn = tmpgen.getFileName();
	std::vector<std::string> const fno = SGGF.merge(ffn);
	
	// libmaus::aio::CheckedInputStream CIS(ffn);
	libmaus::gamma::SparseGammaGapConcatDecoder SGGD(fno);
	for ( uint64_t i = 0; i < 60; ++i )
	{
		uint64_t dv = SGGD.decode();
		
		std::cerr << dv;
		if ( refM.find(i) != refM.end() )
		{
			std::cerr << "(" << refM.find(i)->second << ")";
			assert ( refM.find(i)->second == dv );
		}
		else
		{
			std::cerr << "(0)";
			assert ( dv == 0 );
		}
		std::cerr << ";";
	}
	std::cerr << std::endl;

	for ( uint64_t i = 0; i < fno.size(); ++i )
		remove(fno[i].c_str());
}

#include <libmaus/gamma/SparseGammaGapMultiFileLevelSet.hpp>

void testsparsegammamultifilesetmerge()
{
	libmaus::util::TempFileNameGenerator tmpgen("tmp",3);
	libmaus::gamma::SparseGammaGapMultiFileLevelSet SGGF(tmpgen,4);
	std::map<uint64_t,uint64_t> refM;
	
	for ( uint64_t i = 0; i < 25;  ++i )
	{
		std::string const fn = tmpgen.getFileName();
		std::string const indexfn = fn+".idx";
		libmaus::aio::CheckedOutputStream COS(fn);
		libmaus::aio::CheckedInputOutputStream indexCIOS(indexfn);
		libmaus::gamma::SparseGammaGapBlockEncoder SGE(COS,indexCIOS);
		remove(indexfn.c_str());
		
		SGE.encode(2*i,i+1);   refM[2*i]   += (i+1);
		SGE.encode(2*i+2,i+1); refM[2*i+2] += (i+1);
		SGE.encode(2*i+4,i+1); refM[2*i+4] += (i+1);
		SGE.term();
		
		SGGF.addFile(fn);
	}
	
	std::string const ffn = tmpgen.getFileName();
	std::vector<std::string> const fno = SGGF.merge(ffn);
	
	// libmaus::aio::CheckedInputStream CIS(ffn);
	libmaus::gamma::SparseGammaGapConcatDecoder SGGD(fno);
	for ( uint64_t i = 0; i < 60; ++i )
	{
		uint64_t dv = SGGD.decode();
		
		std::cerr << dv;
		if ( refM.find(i) != refM.end() )
		{
			std::cerr << "(" << refM.find(i)->second << ")";
			assert ( refM.find(i)->second == dv );
		}
		else
		{
			std::cerr << "(0)";
			assert ( dv == 0 );
		}
		std::cerr << ";";
	}
	std::cerr << std::endl;

	for ( uint64_t i = 0; i < fno.size(); ++i )
		remove(fno[i].c_str());
}

void testsparsegammamultifilesetmergedense()
{
	libmaus::util::TempFileNameGenerator tmpgen("tmp",3);
	libmaus::gamma::SparseGammaGapMultiFileLevelSet SGGF(tmpgen,4);
	std::map<uint64_t,uint64_t> refM;
	
	for ( uint64_t i = 0; i < 25;  ++i )
	{
		std::string const fn = tmpgen.getFileName();
		std::string const indexfn = fn+".idx";
		libmaus::aio::CheckedOutputStream COS(fn);
		libmaus::aio::CheckedInputOutputStream indexCIOS(indexfn);
		libmaus::gamma::SparseGammaGapBlockEncoder SGE(COS,indexCIOS);
		remove(indexfn.c_str());
		
		SGE.encode(2*i,i+1);   refM[2*i]   += (i+1);
		SGE.encode(2*i+2,i+1); refM[2*i+2] += (i+1);
		SGE.encode(2*i+4,i+1); refM[2*i+4] += (i+1);
		SGE.term();
		
		SGGF.addFile(fn);
	}
	
	uint64_t const maxval = refM.size() ? (refM.rbegin())->first : 0;
	
	std::string const ffn = tmpgen.getFileName();
	std::vector<std::string> const fno = SGGF.mergeToDense(ffn,maxval+1);
	
	// libmaus::aio::CheckedInputStream CIS(ffn);
	libmaus::gamma::GammaGapDecoder SGGD(fno);
	for ( uint64_t i = 0; i < maxval+1; ++i )
	{
		uint64_t dv = SGGD.decode();
		
		std::cerr << dv;
		if ( refM.find(i) != refM.end() )
		{
			std::cerr << "(" << refM.find(i)->second << ")";
			assert ( refM.find(i)->second == dv );
		}
		else
		{
			std::cerr << "(0)";
			assert ( dv == 0 );
		}
		std::cerr << ";";
	}
	std::cerr << std::endl;

	for ( uint64_t i = 0; i < fno.size(); ++i )
	{
		// std::cerr << fno[i] << std::endl;
		remove(fno[i].c_str());
	}
}

int main()
{
	try
	{
		srand(time(0));

		testsparsegammamultifilesetmergedense();		
		testsparsegammamultifilesetmerge();				
		testsparsegammamultimerge();
		testSparseGammaGapMergingRandom();
		testSparseGammaGapMergingSmall();		
		testSparseGammaConcat();
		testSparseGammaIndexing();
		testsparsegammalevelmerge();		
		testsparsegammamerge();
		testgammasparse();
		testgammarl();
		testLow();
		testRandom(256*1024*1024);
		testgammagap();
	}
	catch(std::exception const & ex)
	{
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
}
