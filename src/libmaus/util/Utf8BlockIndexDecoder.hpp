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
#if ! defined(LIBMAUS_UTIL_UTF8BLOCKINDEXDECODER_HPP)
#define LIBMAUS_UTIL_UTF8BLOCKINDEXDECODER_HPP

#include <libmaus/util/GetFileSize.hpp>
#include <libmaus/util/NumberSerialisation.hpp>

namespace libmaus
{
	namespace util
	{
		struct Utf8BlockIndexDecoder
		{
			uint64_t blocksize;
			uint64_t lastblocksize;
			uint64_t maxblockbytes;
			uint64_t numblocks;
			
			::libmaus::aio::CheckedInputStream CIS;
			
			Utf8BlockIndexDecoder(std::string const & filename);
			
			uint64_t operator[](uint64_t const i);
		};
	}
}
#endif
