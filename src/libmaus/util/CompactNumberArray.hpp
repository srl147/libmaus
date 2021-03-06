/**
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
**/

#if ! defined(LIBMAUS_UTIL_COMPACTNUMBERARRAY_HPP)
#define LIBMAUS_UTIL_COMPACTNUMBERARRAY_HPP

#include <libmaus/wavelet/HuffmanWaveletTree.hpp>
#include <libmaus/math/bitsPerNum.hpp>

namespace libmaus
{
	namespace util
	{
		struct CompactNumberArray
		{
			::libmaus::wavelet::HuffmanWaveletTree::unique_ptr_type H;
			::libmaus::autoarray::AutoArray < ::libmaus::bitio::CompactArray::unique_ptr_type > C;

			template<typename iterator>
			static ::libmaus::autoarray::AutoArray<uint8_t> getBitArray(iterator a, iterator e)
			{
				uint64_t const n = e-a;
				::libmaus::autoarray::AutoArray<uint8_t> B(n,false);
				for ( uint64_t i = 0; i < n; ++i )
					B[i] = ::libmaus::math::bitsPerNum(*(a++));
				return B;
			}
			template<typename iterator>
			static ::libmaus::wavelet::HuffmanWaveletTree::unique_ptr_type getBitWT(iterator a, iterator e)
			{
				::libmaus::autoarray::AutoArray<uint8_t> B = getBitArray(a,e);
				::libmaus::wavelet::HuffmanWaveletTree::unique_ptr_type H (
					new ::libmaus::wavelet::HuffmanWaveletTree(B.begin(),B.end())
					);
				
				for ( uint64_t i = 0; i < B.size(); ++i )
					assert ( B[i] == (*H)[i] );
					
				return H;
			}
			
			template<typename iterator>
			CompactNumberArray(iterator a, iterator e)
			: H(getBitWT(a,e)), C(H->enctable.maxsym+1)
			{
				for ( int i = H->enctable.minsym; i <= H->enctable.maxsym; ++i )
					if ( i && H->enctable.checkSymbol(i) )
					{
						uint64_t const numsyms = H->rank(i, (e-a)-1);
						C[i] = ::libmaus::bitio::CompactArray::unique_ptr_type(new ::libmaus::bitio::CompactArray(numsyms,i));
						std::cerr << numsyms << " symbols use " << i << " bits " << std::endl;
					}
				
				
				uint64_t j = 0;
				for ( iterator i = a; i != e; ++i, ++j )
				{
					uint64_t const bits = ::libmaus::math::bitsPerNum(*i);
					assert ( (*H)[j] == bits );
					
					if ( bits )
					{
						assert ( bits < C.size() );
						assert ( C[bits].get() );
						uint64_t const rank = H->rank(bits,j)-1;
						C[bits]->set( rank , *i );
					}
				}

				j = 0;
				for ( iterator i = a; i != e; ++i, ++j )
					assert ( (*this)[j] == (*i) );
			}
			
			uint64_t operator[](uint64_t const i) const
			{
				uint64_t const bits = (*H)[i];
				
				if ( bits )
				{
					uint64_t const rank = H->rank(bits,i)-1;
					return C[bits]->get(rank);
				}
				else
				{
					return 0;
				}
			}
		};
	}
}
#endif
