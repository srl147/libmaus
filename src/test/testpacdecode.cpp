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
#include <iostream>
#include <libmaus/bitio/PacDecoderBuffer.hpp>
#include <libmaus/util/ArgInfo.hpp>

int main(int argc, char * argv[])
{
	try
	{
		::libmaus::util::ArgInfo const arginfo(argc,argv);
		std::string const fn = arginfo.getRestArg<std::string>(0);
		
		#if 0
		::libmaus::bitio::PacDecoderWrapper PDW(fn);
		::libmaus::autoarray::AutoArray<char> C(64*1024);
		
		while ( PDW )
		{
			PDW.read(C.begin(),C.size());
			int64_t const r = PDW.gcount();
			std::cout.write(C.begin(),r);
		}

		std::cout.flush();
		#endif

		{
		::libmaus::bitio::PacDecoderTermWrapper PDW(fn,16*1024);
		int r = -1;
		while ( (r = PDW.get()) >= 0 )
			std::cout << r;
		std::cout << std::endl;
		}
		{
		::libmaus::bitio::PacDecoderWrapper PDW(fn,16*1024);
		int r = -1;
		while ( (r = PDW.get()) >= 0 )
			std::cout << r;
		std::cout << std::endl;
		}
	}
	catch(std::exception const & ex)
	{
		std::cerr << ex.what() << std::endl;
	}
}
