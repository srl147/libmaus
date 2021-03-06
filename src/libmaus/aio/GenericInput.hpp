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


#if ! defined(GENERICINPUT_HPP)
#define GENERICINPUT_HPP

#include <libmaus/types/types.hpp>
#include <libmaus/aio/AsynchronousBufferReader.hpp>
#include <libmaus/autoarray/AutoArray.hpp>
#include <libmaus/util/GetFileSize.hpp>
#include <libmaus/util/NumberSerialisation.hpp>
#include <libmaus/util/StringSerialisation.hpp>
#include <libmaus/util/Demangle.hpp>
#include <libmaus/util/unique_ptr.hpp>
#include <libmaus/util/ConcatRequest.hpp>
#include <libmaus/aio/FileFragment.hpp>
#include <libmaus/util/GetFileSize.hpp>
#include <limits>
#include <set>

namespace libmaus
{
	namespace aio
	{
		template < typename input_type >
		struct GenericInput
		{
			typedef input_type value_type;
		
			uint64_t const bufsize;
			::libmaus::aio::AsynchronousBufferReader ABR;
			uint64_t curbufleft;
			std::pair < char const *, ssize_t > curbuf;
			input_type const * curword;

			uint64_t const totalwords;
			uint64_t totalwordsread;
			
			typedef GenericInput<input_type> this_type;
			typedef typename ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;

			static ::libmaus::autoarray::AutoArray<input_type> readArray(std::string const & inputfilename)
			{
				uint64_t const fs =
					::libmaus::util::GetFileSize::getFileSize(inputfilename);
				assert ( fs % sizeof(input_type) == 0 );
				uint64_t const n = fs/sizeof(input_type);

				::libmaus::aio::GenericInput<input_type> in(inputfilename,64*1024);
				::libmaus::autoarray::AutoArray<input_type> A(n,false);
			
				for ( uint64_t i = 0; i < n; ++i )
				{
					input_type v;
					bool ok = in.getNext(v);
					assert ( ok );
					A[i] = v;
				}
				
				return A;
			}

			GenericInput(std::string const & filename, uint64_t const rbufsize, uint64_t const roffset = 0)
			: bufsize(rbufsize), 
				ABR(filename, 16, bufsize*sizeof(input_type), roffset * sizeof(input_type)), 
				curbufleft(0),
				curbuf(reinterpret_cast<char const *>(0),0),
				curword(0),
				totalwords ( ::libmaus::util::GetFileSize::getFileSize(filename) / sizeof(input_type) - roffset ),
				totalwordsread(0)
			{

			}
			~GenericInput()
			{
				ABR.flush();
			}
			bool getNext(input_type & word)
			{
				if ( ! curbufleft )
				{
					if ( curbuf.first )
					{
						ABR.returnBuffer();
						curbuf.first = reinterpret_cast<char const *>(0);
						curbuf.second = 0;
					}
					if ( ! ABR.getBuffer(curbuf) )
						return false;
					if ( ! curbuf.second )
						return false;
					assert ( curbuf.second % sizeof(input_type) == 0 );
					
					curbufleft = curbuf.second / sizeof(input_type);
					assert ( curbufleft );
					curword = reinterpret_cast<input_type const *>(curbuf.first);
				}

				word = *(curword++);
				curbufleft -= 1;
				totalwordsread++;

				return true;
			}
		};
	}
}
#endif
