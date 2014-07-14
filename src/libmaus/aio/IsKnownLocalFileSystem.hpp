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
#if ! defined(LIBMAUS_AIO_ISKNOWNLOCALFILESYSTEM_HPP)
#define LIBMAUS_AIO_ISKNOWNLOCALFILESYSTEM_HPP

#include <libmaus/LibMausConfig.hpp>

#if defined(LIBMAUS_HAVE_STATFS)
#include <sys/vfs.h>
#endif

#include <libmaus/aio/CheckedOutputStream.hpp>
#include <libmaus/util/GetFileSize.hpp>
#include <libmaus/util/TempFileRemovalContainer.hpp>

namespace libmaus
{
	namespace aio
	{
		struct IsKnownLocalFileSystem
		{
			static bool isKnownLocalFileSystem(std::string const & filename)
			{
				#if defined(LIBMAUS_HAVE_STATFS) && defined(__linux__)
				struct statfs sfs;
				int const r = statfs(filename.c_str(),&sfs);
				if ( r < 0 )
				{
					int const error = errno;
					libmaus::exception::LibMausException lme;
					lme.getStream() << "isKnownLocalFileSystem(): statfs failed with error " << strerror(error) << std::endl;
					lme.finish();
					throw lme;
				}
				
				switch ( sfs.f_type )
				{
					case 0xEF53: // EXT{2,3,4}
					case 0x58465342: // XFS
						return true;
					default:
						return false;		                                                                      
				}
				#else
				return false;
				#endif
			}

			static bool isKnownLocalFileSystemCreate(std::string const & filename)
			{
				if ( libmaus::util::GetFileSize::fileExists(filename) )
				{
					return isKnownLocalFileSystem(filename);
				}
				else
				{
					libmaus::util::TempFileRemovalContainer::addTempFile(filename);
					libmaus::aio::CheckedOutputStream COS(filename);
					COS.flush();
					COS.close();
					bool const local = isKnownLocalFileSystem(filename);
					remove(filename.c_str());
					return local;
				}
			}
		};
	}
}
#endif
