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
#if ! defined(LIBMAUS_UTIL_UTF8STRING_HPP)
#define LIBMAUS_UTIL_UTF8STRING_HPP

#include <libmaus/rank/ImpCacheLineRank.hpp>
#include <libmaus/select/ImpCacheLineSelectSupport.hpp>
#include <libmaus/util/GetObject.hpp>
#include <libmaus/util/utf8.hpp>
#include <libmaus/util/Histogram.hpp>
#include <libmaus/suffixsort/divsufsort.hpp>
#include <libmaus/util/CountPutObject.hpp>
#include <libmaus/util/iterator.hpp>
#include <libmaus/util/SimpleCountingHash.hpp>
#include <libmaus/util/MemUsage.hpp>
#include <libmaus/parallel/PosixThread.hpp>
#include <libmaus/parallel/PosixMutex.hpp>
#include <libmaus/util/PutObject.hpp>

namespace libmaus
{
	namespace util
	{		
		struct Utf8String
		{
			typedef Utf8String this_type;
			typedef ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef ::libmaus::util::shared_ptr<this_type>::type shared_ptr_type;
			typedef ::libmaus::util::ConstIterator<this_type,wchar_t> const_iterator;
			
			const_iterator begin() const
			{
				return const_iterator(this);
			}

			const_iterator end() const
			{
				return const_iterator(this,size());
			}

			::libmaus::autoarray::AutoArray<uint8_t> A;
			::libmaus::rank::ImpCacheLineRank::unique_ptr_type I;
			::libmaus::select::ImpCacheLineSelectSupport::unique_ptr_type S;

			template<typename stream_type>
			static uint64_t computeOctetLengthFromFile(std::string const & fn, uint64_t const len)
			{
				stream_type stream(fn);
				return computeOctetLength(stream,len);
			}
			
			static uint64_t computeOctetLength(std::wistream & stream, uint64_t const len);
			static shared_ptr_type constructRaw(
				std::string const & filename, 
				uint64_t const offset = 0, 
				uint64_t const blength = std::numeric_limits<uint64_t>::max()
			);
			void setup();
			
			Utf8String(
				std::string const & filename, 
				uint64_t offset = 0, 
				uint64_t blength = std::numeric_limits<uint64_t>::max());
			Utf8String(std::istream & CIS, uint64_t blength);
			Utf8String(std::wistream & CIS, uint64_t const octetlength, uint64_t const symlength);
			
			uint64_t size() const
			{
				if ( A.size() )
					return I->rank1(A.size()-1);
				else
					return 0;
			}
			
			wchar_t operator[](uint64_t const i) const
			{
				uint64_t const p = I->select1(i);
				::libmaus::util::GetObject<uint8_t const *> G(A.begin()+p);
				return ::libmaus::util::UTF8::decodeUTF8(G);
			}
			wchar_t get(uint64_t const i) const
			{
				return (*this)[i];
			}
			
			static ::libmaus::autoarray::AutoArray<uint64_t> computePartStarts(
				::libmaus::autoarray::AutoArray<uint8_t> const & A, uint64_t const tnumparts
			);
			static ::libmaus::autoarray::AutoArray<uint64_t> computePartStarts(std::string const & fn, uint64_t const tnumparts);
			static ::libmaus::autoarray::AutoArray< std::pair<int64_t,uint64_t> > getHistogramAsArray(::libmaus::autoarray::AutoArray<uint8_t> const & A);
			static ::libmaus::autoarray::AutoArray< std::pair<int64_t,uint64_t> > getHistogramAsArray(std::string const & fn);
			static ::libmaus::util::Histogram::unique_ptr_type getHistogram(::libmaus::autoarray::AutoArray<uint8_t> const & A);
			::libmaus::util::Histogram::unique_ptr_type getHistogram() const;
			std::map<int64_t,uint64_t> getHistogramAsMap() const;
			static std::map<int64_t,uint64_t> getHistogramAsMap(::libmaus::autoarray::AutoArray<uint8_t> const & A);

			// suffix sorting class
			typedef ::libmaus::suffixsort::DivSufSort<32,uint8_t *,uint8_t const *,int32_t *,int32_t const *,256,true> sort_type_parallel;
			typedef ::libmaus::suffixsort::DivSufSort<32,uint8_t *,uint8_t const *,int32_t *,int32_t const *,256,false> sort_type_serial;
			typedef sort_type_serial::saidx_t saidx_t;
		
			::libmaus::autoarray::AutoArray<saidx_t,::libmaus::autoarray::alloc_type_c> 
				computeSuffixArray32(bool const parallel = false) const;
		};
	}
}
#endif
