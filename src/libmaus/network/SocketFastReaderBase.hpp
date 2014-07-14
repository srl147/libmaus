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

#if ! defined(SOCKETFASTREADERBASE_HPP)
#define SOCKETFASTREADERBASE_HPP

#include <libmaus/exception/LibMausException.hpp>
#include <libmaus/fastx/CharBuffer.hpp>
#include <libmaus/network/Socket.hpp>
#include <libmaus/types/types.hpp>

#include <string>
#include <vector>

namespace libmaus
{
	namespace network
	{
		struct SocketFastReaderBase
		{
			typedef SocketFastReaderBase this_type;
			typedef ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
		
			private:
			::libmaus::network::SocketBase * socket;
			::libmaus::autoarray::AutoArray<uint8_t> B;
			uint8_t * const pa;
			uint8_t * pc;
			uint8_t * pe;
			uint64_t c;
			
			::libmaus::fastx::CharBuffer cb;
			
			uint64_t readNumber1()
			{
				int const v = getNextCharacter();
				if ( v < 0 )
				{
					::libmaus::exception::LibMausException ex;
					ex.getStream() << "Failed to read number in ::libmaus::aio::SocketFastReaderBase::readNumber1().";
					ex.finish();
					throw ex;
				}
				return v;
			}
			uint64_t readNumber2()
			{
				uint64_t const v0 = readNumber1();
				uint64_t const v1 = readNumber1();
				return (v0<<8)|v1;
			}
			uint64_t readNumber4()
			{
				uint64_t const v0 = readNumber2();
				uint64_t const v1 = readNumber2();
				return (v0<<16)|v1;
			}
			uint64_t readNumber8()
			{
				uint64_t const v0 = readNumber4();
				uint64_t const v1 = readNumber4();
				return (v0<<32)|v1;
			}

			public:
			SocketFastReaderBase(::libmaus::network::SocketBase * rsocket, uint64_t const bufsize)
			: 
				socket(rsocket),
				B(bufsize),
				pa(B.get()),
				pc(pa),
				pe(pc),
				c(0)
			{
			}

			uint64_t getC() const
			{
				return c;
			}


			int get()
			{
				return getNextCharacter();
			}

			int getNextCharacter()
			{
				if ( pc == pe )
				{
					ssize_t red = socket->read ( reinterpret_cast<char *>(pa), B.size() );
					
					if (  red > 0 )
					{
						pc = pa;
						pe = pc+red;
					}
					else
					{
						return -1;	
					}
				}
				
				c += 1;
				return *(pc++);
			}
			
			std::pair < char const *, uint64_t > getLineRaw()
			{
				int c;
				cb.reset();
				while ( (c=getNextCharacter()) >= 0 && c != '\n' )
					cb.bufferPush(c);
				
				if ( cb.length == 0 && c == -1 )
					return std::pair<char const *, uint64_t>(reinterpret_cast<char const *>(0),0);
				else
					return std::pair<char const *, uint64_t>(cb.buffer,cb.length);
			}
			
			bool getLine(std::string & s)
			{
				std::pair < char const *, uint64_t > P = getLineRaw();
				
				if ( P.first )
				{
					s = std::string(P.first,P.first+P.second);
					return true;
				}
				else
				{
					return false;
				}
			}			
		};
	}
}
#endif
