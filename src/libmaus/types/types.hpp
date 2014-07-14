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

#if ! defined(LIBMAUS_TYPES__TYPES_HPP)
#define LIBMAUS_TYPES__TYPES_HPP

#if defined(__GNUC__) && __GNUC__ >= 3
#define expect_true(x)      __builtin_expect (x, 1)
#define expect_false(x)     __builtin_expect (x, 0)
#else
#define expect_true(x) x
#define expect_false(x) x
#endif

#include <libmaus/LibMausConfig.hpp>
#include <cstdlib>

#if defined(LIBMAUS_HAVE_CSTDINT) || defined(_MSC_VER)
#include <cstdint>
#elif defined(LIBMAUS_HAVE_STDINT_H)
#include <stdint.h>
#elif defined(LIBMAUS_HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#if defined(__APPLE__)
#include <stdint.h>
#endif

#if defined(LIBMAUS_HAVE_UNSIGNED_INT128)
namespace libmaus
{
	typedef unsigned __int128 uint128_t;
}
#endif

#endif
