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
#include <libmaus/util/TempFileRemovalContainer.hpp>

bool ::libmaus::util::SignalHandlerContainer::setupComplete = false;

std::vector < std::string > libmaus::util::TempFileRemovalContainer::tmpfilenames;
std::vector < std::string > libmaus::util::TempFileRemovalContainer::tmpdirectories;
std::vector < std::string > libmaus::util::TempFileRemovalContainer::tmpsemaphores;
bool ::libmaus::util::TempFileRemovalContainer::setupComplete = false;
libmaus::util::TempFileRemovalContainer::sighandler_t libmaus::util::TempFileRemovalContainer::siginthandler = 0;
libmaus::util::TempFileRemovalContainer::sighandler_t libmaus::util::TempFileRemovalContainer::sigtermhandler = 0;
libmaus::util::TempFileRemovalContainer::sighandler_t libmaus::util::TempFileRemovalContainer::sighuphandler = 0;
libmaus::util::TempFileRemovalContainer::sighandler_t libmaus::util::TempFileRemovalContainer::sigpipehandler = 0;
::libmaus::parallel::OMPLock libmaus::util::TempFileRemovalContainer::lock;
