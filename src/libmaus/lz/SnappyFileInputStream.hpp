/*
    libmaus
    Copyright (C) 2009-2014 German Tischler
    Copyright (C) 2011-2014 Genome Research Limited

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
#if ! defined(LIBMAUS_LZ_SNAPPYFILEINPUTSTREAM_HPP)
#define LIBMAUS_LZ_SNAPPYFILEINPUTSTREAM_HPP

#include <libmaus/lz/SnappyInputStream.hpp>

namespace libmaus
{
	namespace lz
	{
		struct SnappyFileInputStream
		{
			libmaus::aio::CheckedInputStream istr;
			SnappyInputStream instream;
			
			SnappyFileInputStream(std::string const & filename)
			: istr(filename), instream(istr,0,true) {}
			int get() { return instream.get(); }
			uint64_t read(char * c, uint64_t const n) { return instream.read(c,n); }
			uint64_t gcount() const { return instream.gcount(); }
		};
	}
}
#endif