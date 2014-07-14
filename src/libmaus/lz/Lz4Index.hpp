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
#if !defined(LIBMAUS_LZ_LZ4INDEX_HPP)
#define LIBMAUS_LZ_LZ4INDEX_HPP

#include <istream>
#include <libmaus/util/utf8.hpp>
#include <libmaus/util/NumberSerialisation.hpp>

namespace libmaus
{
	namespace lz
	{
		struct Lz4Index
		{
			std::istream & stream;
			
			uint64_t blocksize;
			uint64_t metapos;
			uint64_t payloadbytes;
			uint64_t numblocks;
			uint64_t indexpos;
			
			Lz4Index(std::istream & rstream)
			: stream(rstream), blocksize(libmaus::util::NumberSerialisation::deserialiseNumber(stream))
			{	
				// read block size
				stream.clear();
				stream.seekg(0,std::ios::beg);
				blocksize = libmaus::util::UTF8::decodeUTF8(stream);
				
				// read meta position
				stream.clear();
				stream.seekg(-static_cast<ssize_t>(sizeof(uint64_t)),std::ios::end);
				metapos = libmaus::util::NumberSerialisation::deserialiseNumber(stream);
				
				// read payloadbytes
				stream.clear();
				stream.seekg(metapos,std::ios::beg);
				payloadbytes = libmaus::util::NumberSerialisation::deserialiseNumber(stream);
				numblocks = libmaus::util::NumberSerialisation::deserialiseNumber(stream);
				
				indexpos = metapos + 2*sizeof(uint64_t);
			}
			
			uint64_t operator[](uint64_t const i) const
			{
				stream.clear();
				stream.seekg(indexpos + i*sizeof(uint64_t),std::ios::beg);
				return libmaus::util::NumberSerialisation::deserialiseNumber(stream);
			}
		};
	}
}
#endif
