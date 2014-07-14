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
#if ! defined(LIBMAUS_NETWORK_FTPSOCKETINPUTSTREAMBUFFER_HPP)
#define LIBMAUS_NETWORK_FTPSOCKETINPUTSTREAMBUFFER_HPP

#include <libmaus/network/FtpSocketWrapper.hpp>
#include <libmaus/network/SocketInputStreamBuffer.hpp>

namespace libmaus
{
	namespace network
	{
		struct FtpSocketInputStreamBuffer : public FtpSocketWrapper, public SocketInputStreamBuffer
		{
			typedef FtpSocketInputStreamBuffer this_type;
			typedef libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef libmaus::util::shared_ptr<this_type>::type shared_ptr_type;
		
			FtpSocketInputStreamBuffer(
				std::string const & url,
				uint64_t const bufsize, 
				uint64_t const pushbacksize = 0
			) 
			:
				libmaus::network::FtpSocketWrapper(url),
				libmaus::network::SocketInputStreamBuffer(getFtpSocket(),bufsize,pushbacksize)
			{
			
			}
		};
	}
}
#endif
