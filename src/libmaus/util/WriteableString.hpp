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

#if ! defined(WRITEABLESTRING_HPP)
#define WRITEABLESTRING_HPP

#include <libmaus/util/unique_ptr.hpp>
#include <libmaus/autoarray/AutoArray.hpp>

namespace libmaus
{
	namespace util
	{
		struct WriteableString
		{
			typedef WriteableString this_type;
			typedef ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;

			::libmaus::autoarray::AutoArray<char> A;
			
			WriteableString()
			{
			
			}
			
			WriteableString(std::string const & s)
			: A(s.size()+1)
			{
				std::copy(s.begin(),s.end(),A.get());
			}
		};
	}
}
#endif
