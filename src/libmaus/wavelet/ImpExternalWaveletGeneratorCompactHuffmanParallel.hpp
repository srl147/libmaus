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

#if ! defined(LIBMAUS_WAVELET_IMPEXTERNALWAVELETGENERATORCOMPACTHUFFMANPARALLEL_HPP)
#define LIBMAUS_WAVELET_IMPEXTERNALWAVELETGENERATORCOMPACTHUFFMANPARALLEL_HPP

#include <libmaus/util/TempFileNameGenerator.hpp>
#include <libmaus/bitio/BufferIterator.hpp>
#include <libmaus/bitio/BitVectorConcat.hpp>
#include <libmaus/util/NumberSerialisation.hpp>
#include <libmaus/rank/ERankBase.hpp>
#include <libmaus/huffman/HuffmanTreeNode.hpp>
#include <libmaus/huffman/HuffmanTreeInnerNode.hpp>
#include <libmaus/huffman/HuffmanTreeLeaf.hpp>
#include <libmaus/rank/ImpCacheLineRank.hpp>
#include <libmaus/aio/CheckedOutputStream.hpp>
#include <libmaus/aio/CheckedInputStream.hpp>
#include <libmaus/util/TempFileContainer.hpp>
#include <libmaus/huffman/HuffmanTree.hpp>
#include <libmaus/aio/SynchronousGenericOutput.hpp>

#include <libmaus/util/unordered_map.hpp>

namespace libmaus
{
	namespace wavelet
	{
		struct ImpExternalWaveletGeneratorCompactHuffmanParallel : ::libmaus::rank::ERankBase
		{
			typedef ImpExternalWaveletGeneratorCompactHuffmanParallel this_type;
			typedef ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			
			// rank type
			typedef ::libmaus::rank::ImpCacheLineRank rank_type;
			// bit output stream contexts
			typedef rank_type::WriteContextExternal context_type;
			// pointer to output context
			typedef context_type::unique_ptr_type context_ptr_type;
			// vector of contexts
			typedef ::libmaus::autoarray::AutoArray<context_ptr_type> context_vector_type;
			
			// pair of context pointer and bit
			typedef std::pair < uint64_t , bool > bit_type;
			// vector of bit type
			typedef ::libmaus::autoarray::AutoArray < bit_type > bit_vector_type;
			// vector of bit vector type
			typedef ::libmaus::autoarray::AutoArray < bit_vector_type > bit_vectors_type;

			private:
			// huffman tree		
			libmaus::huffman::HuffmanTree const & H;
			// huffman encode table
			libmaus::huffman::HuffmanTree::EncodeTable const E;
			// temp file container
			::libmaus::util::TempFileNameGenerator & tmpcnt;
			// number of threads
			uint64_t const numthreads;
			// symbol to rank (position of leaf in H)
			// libmaus::autoarray::AutoArray<uint64_t> symtorank;
			// symbol to offset in bv
			libmaus::autoarray::AutoArray<uint64_t> symtooffset;

			// (contexts,bit) vector
			bit_vector_type bv;

			typedef libmaus::aio::SynchronousGenericOutput<uint64_t> sync_out_type;
			typedef sync_out_type::unique_ptr_type sync_out_ptr_type;
			std::vector<std::string> outstreamfilenames;
			libmaus::autoarray::AutoArray< sync_out_ptr_type > outstreams;
			libmaus::autoarray::AutoArray< sync_out_type::iterator_type > outstreamiterators;
			libmaus::autoarray::AutoArray<uint64_t> outstreambitcnt;
			typedef libmaus::bitio::FastWriteBitWriterBuffer64Sync bit_writer_type;
			typedef bit_writer_type::unique_ptr_type bit_writer_ptr_type;
			libmaus::autoarray::AutoArray<bit_writer_ptr_type> outstreambitwriters;
			libmaus::autoarray::AutoArray<bit_writer_type *> outstreambitwritersp;

			public:
			struct BufferType
			{
				libmaus::huffman::HuffmanTree::EncodeTable const * E;
				bit_type const * bv;
				uint64_t const * symtooffset;
				bit_writer_type ** contexts;
				uint64_t * bitcnt;
				uint64_t symbols;
				
				BufferType() : E(0), bv(0), symtooffset(0), contexts(0), bitcnt(0), symbols(0) {}
			
				void putSymbol(int64_t const s)
				{
					assert ( E->hasSymbol(s) );
					unsigned int codelen = E->getCodeLength(s);
					bit_type const * btp = bv + symtooffset[s-E->minsym];
					
					for ( unsigned int i = 0; i < codelen; ++i )
					{
						contexts[btp[i].first]->writeBit(btp[i].second);
						bitcnt[btp[i].first]++;
					}
					
					symbols += 1;
				}
			};
			
			BufferType & operator[](uint64_t const thread)
			{
				return buffers[thread];
			}
			
			private:
			::libmaus::autoarray::AutoArray<BufferType> buffers;

			void flush()
			{
			}
			
			static uint64_t const bufsize = 1024;
			
			public:
			ImpExternalWaveletGeneratorCompactHuffmanParallel(
				libmaus::huffman::HuffmanTree const & rH,
				::libmaus::util::TempFileNameGenerator & rtmpcnt,
				uint64_t const rnumthreads
			)
			: H(rH), E(H), tmpcnt(rtmpcnt), numthreads(rnumthreads),
			  // symtorank(E.maxsym-E.minsym+1,false), 
			  symtooffset(E.maxsym-E.minsym+1,false),
			  outstreamfilenames(numthreads * H.inner()),
			  outstreams(numthreads * H.inner()),
			  outstreamiterators(numthreads * H.inner()),
			  outstreambitcnt(numthreads * H.inner()),
			  outstreambitwriters(numthreads * H.inner()),
			  outstreambitwritersp(numthreads * H.inner()),
			  buffers(numthreads)
			{
				// compute parent map
				libmaus::autoarray::AutoArray<uint32_t> P(H.leafs()+H.inner(),false);
				for ( uint64_t i = 0; i < H.inner(); ++i )
				{
					P[H.leftChild(H.leafs()+i)] = H.leafs()+i;
					P[H.rightChild(H.leafs()+i)] = H.leafs()+i;					
				}
				P[H.root()] = H.root();
				
				uint64_t totalbits = 0;
				for ( uint64_t i = 0; i < H.leafs(); ++i )
				{
					int64_t const sym = H.getSymbol(i);
					totalbits += E.getCodeLength(sym);
				}

				
				bv = bit_vector_type(totalbits,false);
				
				uint64_t offset = 0;

				for ( uint64_t i = 0; i < H.leafs(); ++i )
				{
					// get symbol
					int64_t const sym = H.getSymbol(i);
					// store offset in bv table
					symtooffset[sym-E.minsym] = offset;
					// code length
					unsigned int const numbits = E.getCodeLength(sym);
					// code
					uint64_t const code = E.getCode(sym);
					
					uint64_t node = i;
					for ( uint64_t j = 0; j < numbits; ++j )
					{
						node = P[node];
						assert ( ! H.isLeaf(node) );
						bv[offset + numbits - j - 1] = bit_type(
							node-H.leafs(),
							code&(1ull<<j)
						);
					}
					
					offset += numbits;
				}

				// set up contexts
				for ( uint64_t i = 0; i < numthreads * H.inner(); ++i )
				{	
					outstreamfilenames[i] = tmpcnt.getFileName();
					libmaus::util::TempFileRemovalContainer::addTempFile(outstreamfilenames[i]);
					sync_out_ptr_type tstream(new sync_out_type(outstreamfilenames[i],bufsize));
					outstreams[i] = UNIQUE_PTR_MOVE(tstream);
					outstreamiterators[i].SGOP = outstreams[i].get();
					outstreambitcnt[i] = 0;
					bit_writer_ptr_type twriter(new bit_writer_type(outstreamiterators[i]));
					outstreambitwriters[i] = UNIQUE_PTR_MOVE(twriter);
					outstreambitwritersp[i] = outstreambitwriters[i].get();
				}
				
				for ( uint64_t i = 0; i < numthreads; ++i )
				{
					buffers[i].E = &E;
					buffers[i].bv = bv.get();
					buffers[i].symtooffset = symtooffset.begin();
					buffers[i].contexts = outstreambitwritersp.begin() + (i*H.inner());
					buffers[i].bitcnt = outstreambitcnt.begin() + (i*H.inner());
				}
			}

			template<typename stream_type>
			uint64_t createFinalStream(stream_type & out)
			{	
				// add padding bit
				for ( uint64_t i = 0; i < H.inner(); ++i )
				{
					outstreambitwriters[(numthreads-1)*H.inner() + i]->writeBit(false);
					outstreambitcnt    [(numthreads-1)*H.inner() + i] += 1;
				}
				for ( uint64_t i = 0; i < outstreambitwriters.size(); ++i )
				{
					outstreambitwriters[i]->flush();
					outstreambitwriters[i].reset();
				}
				for ( uint64_t i = 0; i < outstreams.size(); ++i )
				{
					outstreams[i]->flush();
					outstreams[i].reset();
				}
				
				std::vector<std::string> concatTempFileNames(H.inner());
				::libmaus::autoarray::AutoArray<uint64_t> wordsv(H.inner());
				#if defined(_OPENMP)
				#pragma omp parallel for
				#endif
				for ( int64_t i = 0; i < static_cast<int64_t>(H.inner()); ++i )
				{
					std::vector < std::pair<std::string,uint64_t> > infiles;
					for ( uint64_t j = 0; j < numthreads; ++j )
					{
						infiles.push_back(
							std::pair<std::string,uint64_t>(
								outstreamfilenames[j*H.inner()+i],
								outstreambitcnt[j*H.inner()+i]
							)
						);
					}
					concatTempFileNames[i] = tmpcnt.getFileName();
					libmaus::util::TempFileRemovalContainer::addTempFile(concatTempFileNames[i]);
					libmaus::aio::CheckedOutputStream COS(concatTempFileNames[i]);
					
					// round up
					wordsv[i] = libmaus::bitio::BitVectorConcat::concatenateBitVectors(infiles,COS,6);
					
					// flush and close file
					COS.flush();
					COS.close();
				}

				uint64_t symbols = 0;
				for ( uint64_t i = 0; i < buffers.size(); ++i )
					symbols += buffers[i].symbols;

				uint64_t p = 0;

				// number of symbols in sequence
				p += ::libmaus::util::NumberSerialisation::serialiseNumber(out,symbols); // n
				// huffman tree
				p += H.serialise(out);
				// number of inner nodes in tree
				p += ::libmaus::util::NumberSerialisation::serialiseNumber(out,H.inner()); // number of bit vectors

				std::vector<uint64_t> nodeposvec;	
				/*
				 * concatenate nodes in final output stream
				 */
				for ( int64_t i = 0; i < static_cast<int64_t>(H.inner()); ++i )
				{
					nodeposvec.push_back(p);
				
					libmaus::aio::CheckedInputStream istr(concatTempFileNames[i]);
					uint64_t inwords;
					::libmaus::serialize::Serialize<uint64_t>::deserialize(istr,&inwords);
					assert ( inwords == wordsv[i] );
					assert ( inwords % 6 == 0 );
					
					uint64_t const bufblocks = 4096;
					uint64_t const bufpaywords = bufblocks*6;
					uint64_t const buftotalwords = bufblocks*8;
					uint64_t const numblocks = (inwords + bufpaywords - 1) / bufpaywords;
					
					::libmaus::autoarray::AutoArray<uint64_t> inbuf(bufpaywords);
					::libmaus::autoarray::AutoArray<uint64_t> outbuf(buftotalwords);
					uint64_t s = 0;

					p += ::libmaus::serialize::Serialize<uint64_t>::serialize(out,inwords*64); // number of bits
					p += ::libmaus::serialize::Serialize<uint64_t>::serialize(out,(inwords/6)*8); // number of words
					
					for ( uint64_t j = 0; j < numblocks; ++j )
					{
						uint64_t const low = std::min(j * bufpaywords,inwords);
						uint64_t const high = std::min(low + bufpaywords,inwords);
						uint64_t const size = high-low;
						assert ( size % 6 == 0 );
						uint64_t const bufs = size/6;
						
						uint64_t const readbytes = bufs * 6 * sizeof(uint64_t);
						istr.read(reinterpret_cast<char *>(inbuf.get()), readbytes);
						assert ( istr.gcount() == static_cast<int64_t>(readbytes) );
						
						uint64_t const * inptr = inbuf.begin();
						uint64_t * outptr = outbuf.begin();
						for ( uint64_t k = 0; k < bufs; ++k )
						{
							(*(outptr++)) = s;
							uint64_t * subptr = outptr++;
							*subptr = 0;
							unsigned int shift = 0;
							
							uint64_t m = 0;
							for ( uint64_t l = 0; l < 6; ++l, shift+=9 )
							{
								*subptr |= (m << shift);
								uint64_t const v = *(inptr++);
								(*(outptr++)) = v;
								m += ::libmaus::rank::ERankBase::popcount8(v);
							}
							*subptr |= (m << shift); // set complete

							s += m;
						}
						
						assert ( inptr == inbuf.begin() + 6 * bufs );
						assert ( outptr == outbuf.begin() + 8 * bufs );

						uint64_t const writebytes = bufs * 8 * sizeof(uint64_t);
						out.write(reinterpret_cast<char const *>(outbuf.get()),  writebytes );
						assert ( out );
						p += writebytes;
					}
					
					// remove ( tmpfilename.c_str() );
				}

				uint64_t const indexpos = p;
				p += ::libmaus::util::NumberSerialisation::serialiseNumberVector(out,nodeposvec);
				p += ::libmaus::util::NumberSerialisation::serialiseNumber(out,indexpos);
		
				out.flush();

				for ( uint64_t i = 0; i < H.inner(); ++i )
					remove ( concatTempFileNames[i].c_str() );
				
				for ( uint64_t i = 0; i < outstreamfilenames.size(); ++i )
					remove(outstreamfilenames[i].c_str());
				
				return p;
			}
			
			void createFinalStream(std::string const & filename)
			{
				::libmaus::aio::CheckedOutputStream ostr(filename);
				createFinalStream(ostr);
				ostr.flush();
				ostr.close();
			}
		};
	}
}
#endif
