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

#if ! defined(LIBMAUS_AIO_SINGLEFILEFRAGMENTMERGE_HPP)
#define LIBMAUS_AIO_SINGLEFILEFRAGMENTMERGE_HPP

#include <libmaus/aio/CheckedInputStream.hpp>
#include <libmaus/aio/CheckedOutputStream.hpp>
#include <libmaus/util/TempFileRemovalContainer.hpp>
#include <queue>
#include <vector>

namespace libmaus
{
	namespace aio
	{
		template<typename _element_type>
		struct SingleFileFragmentMerge
		{
			typedef _element_type element_type;
			typedef SingleFileFragmentMerge<element_type> this_type;

			template<typename _type>
			struct SingleFileFragmentMergeHeapComparator
			{
				typedef _type type;

				bool operator()(
					std::pair<uint64_t,type>  const & A,
					std::pair<uint64_t,type>  const & B)
				{
					bool const A_lt_B = A.second < B.second;
					bool const B_lt_A = B.second < A.second;
					
					// if A.second != B.second
					if ( A_lt_B || B_lt_A )
						return !(A_lt_B);
					else
						return A.first > B.first;
				}
			};


			std::string const & fn;
			libmaus::aio::CheckedInputStream CIS;
			std::vector< std::pair<uint64_t,uint64_t> > frags;
			
			std::priority_queue<
				std::pair< uint64_t, element_type >,
				std::vector< std::pair< uint64_t, element_type > >,
				SingleFileFragmentMergeHeapComparator<element_type>
			> Q;
			
			SingleFileFragmentMerge(std::string const & rfn, std::vector< std::pair<uint64_t,uint64_t> > const & rfrags)
			: fn(rfn), CIS(fn), frags(rfrags)
			{
				for ( uint64_t i = 0; i < frags.size(); ++i )
					if ( frags[i].first != frags[i].second )
					{
						CIS.seekg(frags[i].first);
						element_type BC;
						CIS.read(reinterpret_cast<char *>(&BC), sizeof(element_type));
						frags[i].first += sizeof(element_type);
						Q.push(std::pair< uint64_t, element_type >(i,BC));
					}
			}
			
			bool getNext(element_type & BC)
			{
				if ( ! Q.size() )
					return false;
				
				std::pair< uint64_t, element_type > const P = Q.top();
				BC = P.second;
				Q.pop();
				
				uint64_t const fragindex = P.first;
				if ( frags[fragindex].first < frags[fragindex].second )
				{
					CIS.seekg(frags[fragindex].first);
					element_type NBC;

					CIS.read(reinterpret_cast<char *>(&NBC), sizeof(element_type));
					Q.push(std::pair< uint64_t, element_type >(fragindex,NBC));
					
					frags[fragindex].first += sizeof(element_type);
				}
				
				return true;
			}

			//! perform merge in temporary file and move resulting file to original file name
			static void merge(
				std::string const & infn, std::vector< std::pair<uint64_t,uint64_t> > const & frags
			)
			{
				this_type BCM(infn,frags);
				libmaus::util::TempFileRemovalContainer::addTempFile(infn + ".tmp");
				libmaus::aio::CheckedOutputStream COS(infn + ".tmp");
				element_type BC;
				while ( BCM.getNext(BC) )
					COS.write(reinterpret_cast<char const *>(&BC),sizeof(element_type));
				COS.flush();
				COS.close();
				
				rename((infn + ".tmp").c_str(),infn.c_str());
			}
		};
	}
}
#endif
