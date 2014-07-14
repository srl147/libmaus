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
#if ! defined(LIBMAUS_BAMBAM_BAMMERGECOORDINATE_HPP)
#define LIBMAUS_BAMBAM_BAMMERGECOORDINATE_HPP

#include <libmaus/bambam/BamCatHeader.hpp>
#include <libmaus/bambam/BamMergeTemplate.hpp>

namespace libmaus
{
	namespace bambam
	{
		struct BamMergeCoordinateHeapComparator
		{
			libmaus::bambam::BamAlignment ** algns;
			
			BamMergeCoordinateHeapComparator(libmaus::bambam::BamAlignment ** ralgns) : algns(ralgns) {}
		
			bool operator()(uint64_t const a, uint64_t const b) const
			{
				libmaus::bambam::BamAlignment const * A = algns[a];
				libmaus::bambam::BamAlignment const * B = algns[b];
			
				uint32_t const refida = A->getRefID();
				uint32_t const refidb = B->getRefID();
				
				if ( refida != refidb )
					return refida > refidb;
				
				uint32_t const posa = A->getPos();
				uint32_t const posb = B->getPos();
				
				if ( posa != posb )
					return posa > posb;
					
				return a > b;
			}
		};

		typedef BamMergeTemplate<BamMergeCoordinateHeapComparator,BamCatHeader::IsCoordinateSorted> BamMergeCoordinate;
		
		struct BamMergeCoordinateWrapper
		{
			BamMergeCoordinate object;
			
			BamMergeCoordinateWrapper(std::vector<std::string> const & filenames, bool const putrank = false)
			: object(filenames,putrank) {}
		};
	}
}
#endif
