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
#if ! defined(LIBMAUS_BAMBAM_MERGEQUEUEELEMENT_HPP)
#define LIBMAUS_BAMBAM_MERGEQUEUEELEMENT_HPP

#include <libmaus/bambam/BamAlignment.hpp>
#include <cstring>

namespace libmaus
{
	namespace bambam
	{
		struct MergeQueueElement
		{
			::libmaus::bambam::BamAlignment::shared_ptr_type algn;
			uint64_t index;
			
			MergeQueueElement()
			{
			}
			
			MergeQueueElement(::libmaus::bambam::BamAlignment::shared_ptr_type ralgn,
				uint64_t const rindex)
			: algn(ralgn), index(rindex) {}

			static bool compare(
				::libmaus::bambam::BamAlignment::shared_ptr_type const & A,
				::libmaus::bambam::BamAlignment::shared_ptr_type const & B)
			{
				int r = strcmp(A->getName(),B->getName());
				
				if ( r )
					return r > 0;

				return A->isRead2();
			}
						
			bool operator<(MergeQueueElement const & o) const
			{
				return compare(algn,o.algn);
			}
		};
	}
}
#endif
