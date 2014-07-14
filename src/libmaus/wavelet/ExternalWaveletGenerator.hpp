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

#if ! defined(EXTERNALWAVELETGENERATOR_HPP)
#define EXTERNALWAVELETGENERATOR_HPP

#include <libmaus/bitio/BitVectorConcat.hpp>
#include <libmaus/bitio/BufferIterator.hpp>
#include <libmaus/bitio/FastWriteBitWriter.hpp>
#include <libmaus/util/TempFileNameGenerator.hpp>
#include <libmaus/aio/SynchronousGenericInput.hpp>
#include <libmaus/rank/ERank222B.hpp>
#include <libmaus/wavelet/WaveletTree.hpp>
#include <libmaus/rank/ImpCacheLineRank.hpp>
#include <libmaus/util/NumberSerialisation.hpp>
#include <libmaus/huffman/HuffmanTreeNode.hpp>
#include <libmaus/huffman/HuffmanTreeInnerNode.hpp>
#include <libmaus/huffman/EncodeTable.hpp>

#include <libmaus/util/unordered_map.hpp>

namespace libmaus
{
	namespace wavelet
	{
		/**
		 * external memory wavelet tree bit sequence
		 * generator for small alphabets
		 **/
		struct ExternalWaveletGenerator : public ::libmaus::bitio::BitVectorConcat
		{
			typedef ExternalWaveletGenerator this_type;
			typedef ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
		
			private:
		
			uint64_t const b;
			::libmaus::util::TempFileNameGenerator & tmpgen;
			
			
			typedef ::libmaus::bitio::BufferIterator<uint64_t> output_file_type;
			typedef ::libmaus::util::unique_ptr<output_file_type>::type output_file_ptr_type;
			typedef ::libmaus::autoarray::AutoArray<output_file_ptr_type> output_file_ptr_array_type;
			typedef output_file_ptr_array_type::unique_ptr_type output_file_ptr_array_ptr_type;
						
			::libmaus::autoarray::AutoArray< output_file_ptr_array_ptr_type > outputfiles;
			std::vector < std::pair<std::string,uint64_t> > filenames;
			uint64_t symbols;
			::libmaus::autoarray::AutoArray< uint64_t > freq;
			
			void flush()
			{
				uint64_t k = 0;
				for ( uint64_t i = 0; i < b; ++i )
				{
					uint64_t const numfiles = 1ull<<i;
				
					for ( uint64_t j = 0; j < numfiles; ++j, ++k )
					{
						(*outputfiles[i])[j]->flush();
						uint64_t const lbits = (*outputfiles[i])[j]->bits;
						filenames[k].second = lbits;
						(*outputfiles[i])[j].reset();
					}
					
					outputfiles[i].reset();
				}
			}

			void removeFiles()
			{
				for ( uint64_t i = 0; i < filenames.size(); ++i )
					remove ( filenames[i].first.c_str() );			
			}
			
			public:
			ExternalWaveletGenerator(uint64_t const rb, ::libmaus::util::TempFileNameGenerator & rtmpgen)
			: b(rb), tmpgen(rtmpgen), outputfiles(b), filenames(), symbols(0), freq(1ull<<b)
			{
				for ( uint64_t i = 0; i < b; ++i )
				{
					uint64_t const numfiles = 1ull<<i;
					output_file_ptr_array_ptr_type outputfilesi(
                                                new output_file_ptr_array_type(numfiles)
                                                );
					outputfiles[i] = UNIQUE_PTR_MOVE(outputfilesi);
				
					for ( uint64_t j = 0; j < numfiles; ++j )
					{
						std::string const fn = tmpgen.getFileName();
						filenames.push_back(std::pair<std::string,uint64_t>(fn,0));
						output_file_ptr_type toutputfilesij(
                                                        new output_file_type(fn,bufsize)
                                                        );
						(*outputfiles[i])[j] = UNIQUE_PTR_MOVE(toutputfilesij);
						#if 0
						std::cerr << "i=" << i << " j=" << j << std::endl;
						#endif
					}
				}
			}
			
			
			uint64_t createFinalStream(std::ostream & out)
			{			
				flush();
							
				// number of symbols				
				::libmaus::serialize::Serialize<uint64_t>::serialize(out,symbols);
				// number of bits per symbol
				::libmaus::serialize::Serialize<uint64_t>::serialize(out,b);
				
				out.flush();
				concatenateBitVectors(filenames,out);
				
				removeFiles();
				
				return symbols;
			}
			
			uint64_t createFinalStream(std::string const & filename)
			{
				std::ofstream out(filename.c_str(),std::ios::binary);
				return createFinalStream(out);
			}
			
			
			void putSymbol(uint64_t const s)
			{
				freq[s]++;
			
				for ( uint64_t i = 0; i < b; ++i )
				{
					uint64_t const prefix = s>>(b-i);
					uint64_t const bit = (s>>(b-i-1))&1;
					#if 0
					std::cerr << "Symbol " << s
						<< " i=" << i
						<< " prefix=" << prefix
						<< " bit=" << bit << std::endl;
					#endif
					(*outputfiles[i])[prefix]->writeBit(bit);
				}
				
				symbols += 1;
			}
			
			::libmaus::autoarray::AutoArray<uint64_t> getFreq() const
			{
				return freq.clone();
			}
		};
	}
}
#endif
