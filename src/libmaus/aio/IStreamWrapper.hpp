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
#if ! defined(LIBMAUS_UTIL_ISTREAMWRAPPER_HPP)
#define LIBMAUS_UTIL_ISTREAMWRAPPER_HPP

#include <istream>

namespace libmaus
{
	namespace aio
	{
		/**
		 * wrapper class for object of type std::istream
		 **/
		struct IStreamWrapper
		{
			//! wrapped object
			std::istream & in;
			
			/**
			 * constructor
			 *
			 * @param rin object to e wrapped
			 **/
			IStreamWrapper(std::istream & rin) : in(rin) {}
			
			/**
			 * call wrapped object's read function
			 *
			 * @param p input buffer
			 * @param c number of bytes to be read
			 * @return number of bytes read
			 **/
			ssize_t read(char * p, size_t c)
			{
				in.read(p,c);
				return in.gcount();
			}
		};
	}
}
#endif
