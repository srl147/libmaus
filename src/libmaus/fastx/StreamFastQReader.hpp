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

#if ! defined(LIBMAUS_FASTX_STREAMFASTQREADERWRAPPER_HPP)
#define LIBMAUS_FASTX_STREAMFASTQREADERWRAPPER_HPP

#include <libmaus/fastx/FastQReader.hpp>
#include <libmaus/fastx/StreamFastReaderBase.hpp>

namespace libmaus
{
	namespace fastx
	{
                typedef FastQReaderTemplate< ::libmaus::fastx::StreamFastReaderBase > StreamFastQReader;
                
                struct StreamFastQReaderWrapper : public StreamFastQReader
                {
                	StreamFastQReaderWrapper(std::istream & in) : StreamFastQReader(&in,getDefaultQualityOffset()) {}
                	StreamFastQReaderWrapper(std::istream & in, int const qualityOffset) : StreamFastQReader(&in,qualityOffset) {}
                	StreamFastQReaderWrapper(std::istream & in, libmaus::fastx::FastInterval const & FI) : StreamFastQReader(&in,getDefaultQualityOffset(),FI) {}
                	StreamFastQReaderWrapper(std::istream & in, int const qualityOffset, libmaus::fastx::FastInterval const & FI) : StreamFastQReader(&in,qualityOffset,FI) {}
                };
	}
}
#endif
