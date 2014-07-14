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
#if ! defined(LIBMAUS_LZ_BGZFDEFLATEBLOCKIDCOMPARATOR_HPP)
#define LIBMAUS_LZ_BGZFDEFLATEBLOCKIDCOMPARATOR_HPP

#include <libmaus/lz/BgzfDeflateBase.hpp>
#include <libmaus/lz/BgzfThreadOpBase.hpp>

namespace libmaus
{
	namespace lz
	{
		struct BgzfDeflateBlockIdComparator
		{
			libmaus::autoarray::AutoArray<libmaus::lz::BgzfDeflateBase::unique_ptr_type> const & deflateB;
					
			BgzfDeflateBlockIdComparator(libmaus::autoarray::AutoArray<libmaus::lz::BgzfDeflateBase::unique_ptr_type> const & rdeflateB) : deflateB(rdeflateB)
			{
			
			}

			bool operator()(uint64_t const i, uint64_t const j) const
			{
				return deflateB[i]->blockid > deflateB[j]->blockid;
			}		

			bool operator()(BgzfThreadQueueElement const & i, BgzfThreadQueueElement const & j) const
			{
				return deflateB[i.objectid]->blockid > deflateB[j.objectid]->blockid;
			}		
		};
	}
}
#endif
