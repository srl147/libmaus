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


#if ! defined(LIBMAUS_PARALLEL_LOCK_HPP)
#define LIBMAUS_PARALLEL_LOCK_HPP

#if defined(_OPENMP)
#include <omp.h>
#endif

#include <libmaus/util/unique_ptr.hpp>
#include <libmaus/util/shared_ptr.hpp>

namespace libmaus
{
	namespace parallel
	{
		struct OMPLock
		{
			typedef OMPLock this_type;
			typedef libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef libmaus::util::shared_ptr<this_type>::type shared_ptr_type;
		
#if defined(_OPENMP)
			omp_lock_t lock_obj;
#endif

			OMPLock()
#if defined(_OPENMP)
			: lock_obj()
#endif
			{
#if defined(_OPENMP)
				omp_init_lock(&lock_obj);
#endif
			}
			~OMPLock()
			{
#if defined(_OPENMP)
				omp_destroy_lock(&lock_obj);
#endif
			}
			void lock()
			{
#if defined(_OPENMP)
				omp_set_lock(&lock_obj);
#endif		
			}
			void unlock()
			{
#if defined(_OPENMP)
				omp_unset_lock(&lock_obj);
#endif	
			}
		};
		
		struct ScopeLock
		{
			OMPLock & lock;
		
			ScopeLock(OMPLock & rlock)
			: lock(rlock)
			{
				lock.lock();
			}
			
			~ScopeLock()
			{
				lock.unlock();
			}
		};
	}
}
#endif
