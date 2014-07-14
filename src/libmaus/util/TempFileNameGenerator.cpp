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
#include <libmaus/util/TempFileNameGenerator.hpp>

libmaus::util::TempFileNameGenerator::TempFileNameGenerator(std::string const rprefix, unsigned int const rdepth)
: state(rdepth,rprefix), startstate(state)
{
	::libmaus::util::TempFileRemovalContainer::setup();
}
libmaus::util::TempFileNameGenerator::~TempFileNameGenerator()
{
	cleanupDirs();
}

std::string libmaus::util::TempFileNameGenerator::getFileName()
{
	lock.lock();
	
	std::string const fn = state.getFileName();

	lock.unlock();
	
	return fn;
}

void libmaus::util::TempFileNameGenerator::cleanupDirs()
{
	TempFileNameGeneratorState rmdirstate = startstate;
	
	while ( rmdirstate != state )
	{
		rmdirstate.removeDirs();
	}
}
