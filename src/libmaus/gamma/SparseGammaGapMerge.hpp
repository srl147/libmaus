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
#if ! defined(LIBMAUS_GAMMA_SPARSEGAMMAGAPMERGE_HPP)
#define LIBMAUS_GAMMA_SPARSEGAMMAGAPMERGE_HPP

#include <libmaus/gamma/GammaEncoder.hpp>
#include <libmaus/gamma/GammaDecoder.hpp>
#include <libmaus/gamma/SparseGammaGapConcatDecoder.hpp>
#include <libmaus/gamma/SparseGammaGapBlockEncoder.hpp>
#include <libmaus/aio/SynchronousGenericOutput.hpp>
#include <libmaus/aio/SynchronousGenericInput.hpp>
#include <libmaus/util/shared_ptr.hpp>

namespace libmaus
{
	namespace gamma
	{
		struct SparseGammaGapMerge
		{
			static std::vector<std::string> merge(
				std::vector<std::string> const & fna,
				std::vector<std::string> const & fnb,
				std::string const fnpref,
				uint64_t tparts
			)
			{
				std::vector<uint64_t> sp = libmaus::gamma::SparseGammaGapFileIndexMultiDecoder::getSplitKeys(fna,fnb,tparts);
				uint64_t const parts = sp.size();
				bool const aempty = libmaus::gamma::SparseGammaGapFileIndexMultiDecoder(fna).isEmpty();
				bool const bempty = libmaus::gamma::SparseGammaGapFileIndexMultiDecoder(fnb).isEmpty();				
				uint64_t const maxa = aempty ? 0 : libmaus::gamma::SparseGammaGapFileIndexMultiDecoder(fna).getMaxKey();
				uint64_t const maxb = bempty ? 0 : libmaus::gamma::SparseGammaGapFileIndexMultiDecoder(fnb).getMaxKey();
				uint64_t const maxv = std::max(maxa,maxb);
				sp.push_back(maxv+1);
				
				std::vector<std::string> outputfilenames(parts);
				for ( uint64_t p = 0; p < parts; ++p )
				{
					std::ostringstream fnostr;
					fnostr << fnpref << "_" << std::setw(6) << std::setfill('0') << p << std::setw(0);
					std::string const fn = fnostr.str();
					outputfilenames[p] = fn;
					std::string const indexfn = fn + ".idx";
					libmaus::util::TempFileRemovalContainer::addTempFile(indexfn);

					libmaus::aio::CheckedOutputStream COS(fn);
					std::fstream indexstr(indexfn.c_str(),std::ios::in|std::ios::out|std::ios::binary|std::ios::trunc);
					
					// std::cerr << fn << "\t" << indexfn << "\t" << sp.at(p) << "\t" << sp.at(p+1) << std::endl;
					
					merge(fna,fnb,sp.at(p),sp.at(p+1),COS,indexstr);
					
					remove(indexfn.c_str());
				}
				
				return outputfilenames;
			}
		
			static void merge(
				std::vector<std::string> const & fna,
				std::vector<std::string> const & fnb,
				uint64_t const klow,  // inclusive
				uint64_t const khigh, // exclusive
				std::string const & outputfilename
			)
			{
				std::string const indexfilename = outputfilename + ".idx";
				libmaus::util::TempFileRemovalContainer::addTempFile(indexfilename);
				
				libmaus::aio::CheckedOutputStream COS(outputfilename);
				std::fstream indexstr(indexfilename.c_str(),std::ios::in|std::ios::out|std::ios::binary|std::ios::trunc);
				
				merge(fna,fnb,0,std::numeric_limits<uint64_t>::max(),COS,indexstr);
				
				remove(indexfilename.c_str());
			}
		
			static void merge(
				std::vector<std::string> const & fna,
				std::vector<std::string> const & fnb,
				uint64_t const klow,  // inclusive
				uint64_t const khigh, // exclusive
				std::ostream & stream_out,
				std::iostream & index_str
			)
			{
				// true if a contains any relevant keys
				bool const aproc = libmaus::gamma::SparseGammaGapConcatDecoder::hasKeyInRange(fna,klow,khigh);
				// true if b contains any relevant keys
				bool const bproc = libmaus::gamma::SparseGammaGapConcatDecoder::hasKeyInRange(fnb,klow,khigh);
								
				// first key in stream a (or 0 if none)
				uint64_t const firstkey_a = aproc ? libmaus::gamma::SparseGammaGapConcatDecoder::getNextKey(fna,klow) : std::numeric_limits<uint64_t>::max();
				// first key in stream b (or 0 if none)
				uint64_t const firstkey_b = bproc ? libmaus::gamma::SparseGammaGapConcatDecoder::getNextKey(fnb,klow) : std::numeric_limits<uint64_t>::max();
				// first key in output block
				uint64_t const firstkey_ab = std::min(firstkey_a,firstkey_b);
				
				// previous non zero key (or -1 if none)
				int64_t const prevkey_a = libmaus::gamma::SparseGammaGapConcatDecoder::getPrevKey(fna,klow);
				int64_t const prevkey_b = libmaus::gamma::SparseGammaGapConcatDecoder::getPrevKey(fnb,klow);
				int64_t const prevkey_ab = std::max(prevkey_a,prevkey_b);
				
				// set up encoder
				libmaus::gamma::SparseGammaGapBlockEncoder oenc(stream_out,index_str,prevkey_ab);
				// set up decoders
				libmaus::gamma::SparseGammaGapConcatDecoder adec(fna,firstkey_a);
				libmaus::gamma::SparseGammaGapConcatDecoder bdec(fnb,firstkey_b);

				// current key,value pairs for stream a and b
				std::pair<uint64_t,uint64_t> aval(firstkey_a,adec.p.second);
				std::pair<uint64_t,uint64_t> bval(firstkey_b,bdec.p.second);
				
				// while both streams have keys in range
				while ( aval.second && aval.first < khigh && bval.second && bval.first < khigh )
				{
					if ( aval.first == bval.first )
					{
						oenc.encode(aval.first,aval.second+bval.second);
						aval.first += adec.nextFirst() + 1;
						aval.second = adec.nextSecond();
						bval.first += bdec.nextFirst() + 1;
						bval.second = bdec.nextSecond();
					}
					else if ( aval.first < bval.first )
					{
						oenc.encode(aval.first,aval.second);						
						aval.first += adec.nextFirst() + 1;
						aval.second = adec.nextSecond();
					}
					else // if ( bval.first < aval.first )
					{
						oenc.encode(bval.first,bval.second);						
						bval.first += bdec.nextFirst() + 1;
						bval.second = bdec.nextSecond();
					}				
				}

				// rest keys in stream a
				while ( aval.second && aval.first < khigh )
				{
					oenc.encode(aval.first,aval.second);						
					aval.first += adec.nextFirst() + 1;
					aval.second = adec.nextSecond();				
				}

				// rest keys in stream b
				while ( bval.second && bval.first < khigh )
				{
					oenc.encode(bval.first,bval.second);						
					bval.first += bdec.nextFirst() + 1;
					bval.second = bdec.nextSecond();
				}
				
				oenc.term();
			}
		
			static void merge(
				std::istream & stream_in_a,
				std::istream & stream_in_b,
				std::ostream & stream_out
			)
			{
				libmaus::aio::SynchronousGenericInput<uint64_t> SGIa(stream_in_a,64*1024);
				libmaus::aio::SynchronousGenericInput<uint64_t> SGIb(stream_in_b,64*1024);
				libmaus::aio::SynchronousGenericOutput<uint64_t> SGO(stream_out,64*1024);
				
				libmaus::gamma::GammaDecoder< libmaus::aio::SynchronousGenericInput<uint64_t> > adec(SGIa);
				libmaus::gamma::GammaDecoder< libmaus::aio::SynchronousGenericInput<uint64_t> > bdec(SGIb);
				libmaus::gamma::GammaEncoder< libmaus::aio::SynchronousGenericOutput<uint64_t> > oenc(SGO);
				
				std::pair<uint64_t,uint64_t> aval;
				std::pair<uint64_t,uint64_t> bval;

				aval.first = adec.decode();
				aval.second = adec.decode();

				bval.first = bdec.decode();
				bval.second = bdec.decode();
				
				int64_t prevkey = -1;
				
				while ( aval.second && bval.second )
				{
					if ( aval.first == bval.first )
					{
						oenc.encode(static_cast<int64_t>(aval.first) - prevkey - 1);
						oenc.encode(aval.second + bval.second);
						
						prevkey = aval.first;
						
						aval.first += adec.decode() + 1;
						aval.second = adec.decode();
						bval.first += bdec.decode() + 1;
						bval.second = bdec.decode();
					}
					else if ( aval.first < bval.first )
					{
						oenc.encode(static_cast<int64_t>(aval.first) - prevkey - 1);
						oenc.encode(aval.second);
						
						prevkey = aval.first;
						
						aval.first += adec.decode() + 1;
						aval.second = adec.decode();
					}
					else // if ( bval.first < aval.first )
					{
						oenc.encode(static_cast<int64_t>(bval.first) - prevkey - 1);
						oenc.encode(bval.second);
						
						prevkey = bval.first;
						
						bval.first += bdec.decode() + 1;
						bval.second = bdec.decode();
					}
				}
				
				while ( aval.second )
				{
					oenc.encode(static_cast<int64_t>(aval.first) - prevkey - 1);
					oenc.encode(aval.second);
						
					prevkey = aval.first;
						
					aval.first += adec.decode() + 1;
					aval.second = adec.decode();				
				}
				
				while ( bval.second )
				{
					oenc.encode(static_cast<int64_t>(bval.first) - prevkey - 1);
					oenc.encode(bval.second);
						
					prevkey = bval.first;
						
					bval.first += bdec.decode() + 1;
					bval.second = bdec.decode();
				}
				
				oenc.encode(0);
				oenc.encode(0);
				oenc.flush();
				SGO.flush();
				stream_out.flush();
			}		
		};
	}
}
#endif
