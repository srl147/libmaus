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
#if ! defined(LIBMAUS_BAMBAM_BAMSEQENCODETABLE_HPP)
#define LIBMAUS_BAMBAM_BAMSEQENCODETABLE_HPP

#include <libmaus/autoarray/AutoArray.hpp>

namespace libmaus
{
	namespace bambam
	{
		struct BamSeqEncodeTable
		{
			::libmaus::autoarray::AutoArray<uint8_t> A;
		
			BamSeqEncodeTable()
			: A(256)
			{
				char const * s = "=ACMGRSVTWYHKDBN";
				for ( uint64_t i = 0; i < A.size(); ++i )
					A[i] = strlen(s);
				for ( uint64_t i = 0; i < strlen(s); ++i )
					A [ s[i] ] = i;
			}
			
			uint8_t operator[](uint8_t const i) const
			{
				return A[i];
			}
		};
	}
}
#endif
