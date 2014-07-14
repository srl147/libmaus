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
#if ! defined(LIBMAUS_LZ_BGZFDEFLATEBLOCKIDINFO_HPP)
#define LIBMAUS_LZ_BGZFDEFLATEBLOCKIDINFO_HPP

#include <libmaus/lz/BgzfDeflateBase.hpp>
#include <libmaus/lz/BgzfThreadOpBase.hpp>

namespace libmaus
{
	namespace lz
	{
		struct BgzfDeflateBlockIdInfo
		{
			libmaus::autoarray::AutoArray<libmaus::lz::BgzfDeflateBase::unique_ptr_type> const & deflateB;
					
			BgzfDeflateBlockIdInfo(libmaus::autoarray::AutoArray<libmaus::lz::BgzfDeflateBase::unique_ptr_type> const & rdeflateB) : deflateB(rdeflateB)
			{
			
			}

			uint64_t operator()(uint64_t const i) const
			{
				return deflateB[i]->blockid;
			}		

			BgzfThreadQueueElement const & operator()(BgzfThreadQueueElement const & i) const
			{
				return i;
			}		
		};
	}
}
#endif
