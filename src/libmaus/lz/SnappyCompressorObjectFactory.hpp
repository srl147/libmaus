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
#if ! defined(LIBMAUS_LZ_SNAPPYCOMPRESSOROBJECTFACTORY_HPP)
#define LIBMAUS_LZ_SNAPPYCOMPRESSOROBJECTFACTORY_HPP

#include <libmaus/lz/SnappyCompressorObject.hpp>
#include <libmaus/lz/CompressorObjectFactory.hpp>

namespace libmaus
{
	namespace lz
	{
		struct SnappyCompressorObjectFactory : public CompressorObjectFactory
		{
			typedef SnappyCompressorObjectFactory this_type;
			typedef libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			
			virtual ~SnappyCompressorObjectFactory() {}
			virtual CompressorObject::unique_ptr_type operator()()
			{
				CompressorObject::unique_ptr_type ptr(new SnappyCompressorObject);
				return UNIQUE_PTR_MOVE(ptr);
			}
			virtual std::string getDescription() const
			{
				return "SnappyCompressorObjectFactory";
			}
		};
	}
}
#endif
