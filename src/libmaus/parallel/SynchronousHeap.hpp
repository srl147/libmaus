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

#if ! defined(SYNCHRONOUSHEAP_HPP)
#define SYNCHRONOUSHEAP_HPP

#include <libmaus/LibMausConfig.hpp>
#include <libmaus/parallel/TerminatableSynchronousQueue.hpp>

#if defined(LIBMAUS_HAVE_PTHREADS)
#include <libmaus/parallel/PosixMutex.hpp>
#include <libmaus/parallel/PosixSpinLock.hpp>
#include <libmaus/parallel/PosixSemaphore.hpp>
#include <deque>
#include <queue>

namespace libmaus
{
        namespace parallel
        {
                template<typename value_type, typename compare = ::std::less<value_type> >
                struct SynchronousHeap
                {
                        std::priority_queue < value_type, std::vector<value_type>, compare > Q;
                        PosixSpinLock lock;
                        PosixSemaphore semaphore;
                        
                        SynchronousHeap()
                        {
                        
                        }
                        
                        SynchronousHeap(compare const & comp)
                        : Q(comp)
                        {
                        
                        }
                        
                        unsigned int getFillState()
                        {
                                lock.lock();
                                unsigned int const fill = Q.size();
                                lock.unlock();
                                return fill;
                        }
                        
                        void enque(value_type const q)
                        {
                        	{
                        	libmaus::parallel::ScopePosixSpinLock llock(lock);
                                Q.push(q);
                                }
                                semaphore.post();
                        }
                        value_type deque()
                        {
                                semaphore.wait();
                        	libmaus::parallel::ScopePosixSpinLock llock(lock);
                                value_type const v = Q.top();
                                Q.pop();
                                return v;
                        }
                        
                        std::vector<value_type> pending()
                        {
	                        std::priority_queue < value_type, std::vector<value_type>, compare > C;
	                        {
	                        	libmaus::parallel::ScopePosixSpinLock llock(lock);
	                        	C = Q;
				}
	                        std::vector<value_type> V;
	                        while ( ! C.empty() )
	                        {
	                        	V.push_back(C.top());
	                        	C.pop();
				}
				
				return V;
                        }
                };
                
                template<typename _value_type>
                struct SynchronousConsecutiveHeapDefaultInfo
                {
                	typedef _value_type value_type;
                	typedef SynchronousConsecutiveHeapDefaultInfo<value_type> this_type;
                	typedef typename libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
                
                	value_type operator()(value_type const i) const
                	{
                		return i;
                	}
                };

                template<typename value_type, typename info_type = SynchronousConsecutiveHeapDefaultInfo<value_type>, typename compare = ::std::less<value_type> >
                struct SynchronousConsecutiveHeap
                {
                	info_type const & info;
                
                        std::priority_queue < value_type, std::vector<value_type>, compare > preQ;
                        value_type next;
                        std::deque<value_type> Q;
                        PosixSpinLock lock;
                        PosixSemaphore semaphore;
                        value_type readyfor;
                        
                        SynchronousConsecutiveHeap(
                        	compare const & comp,
                        	info_type const & rinfo
			)
                        : info(rinfo), preQ(comp), next(value_type()), readyfor(value_type())
                        {
                        
                        }
                        
                        unsigned int getFillState()
                        {
                                lock.lock();
                                unsigned int const fill = Q.size();
                                lock.unlock();
                                return fill;
                        }
                        
                        template<typename glob_queue_type>
                        void drainPreQueue(glob_queue_type * const globlist)
                        {
                        	uint64_t postcnt = 0;
                        
                        	{
					libmaus::parallel::ScopePosixSpinLock llock(lock);
					while ( 
						preQ.size() && 
						info(preQ.top()) == next && 
						info(preQ.top()) <= readyfor 
					)
					{
						Q.push_back(preQ.top());
						if ( globlist )
							globlist->enque(preQ.top());
						preQ.pop();
						++next;
						++postcnt;
					}                                
                                }

                                for ( uint64_t i = 0; i < postcnt; ++i )
	                                semaphore.post();
                        }

			void enque(value_type const q)
			{
				enque< libmaus::parallel::TerminatableSynchronousQueue<value_type> >(q,0);
			}

                        // template<typename glob_queue_type = libmaus::parallel::TerminatableSynchronousQueue<value_type> >
                        template<typename glob_queue_type>
                        void enque(
                        	value_type const q, 
                        	glob_queue_type * const globlist = 0
			)
                        {
                        	{
					libmaus::parallel::ScopePosixSpinLock llock(lock);
        	                        preQ.push(q);
                                }

				drainPreQueue(globlist);
                        }

			void setReadyFor(value_type const rreadyfor)
			{
				setReadyFor< libmaus::parallel::TerminatableSynchronousQueue<value_type> > (rreadyfor,0);
			}
                        
                        // template<typename glob_queue_type = libmaus::parallel::TerminatableSynchronousQueue<value_type> >
                        template<typename glob_queue_type>
                        void setReadyFor(
                        	value_type const rreadyfor, 
                        	glob_queue_type * const globlist = 0
			)
                        {
                        	{
					libmaus::parallel::ScopePosixSpinLock llock(lock);
        	                	readyfor = rreadyfor;
                        	}

				drainPreQueue(globlist);
                        }

                        value_type deque()
                        {
                                semaphore.wait();
				libmaus::parallel::ScopePosixSpinLock llock(lock);
                                value_type const v = Q.front();
                                Q.pop_front();
                                return v;
                        }

                        void putback(value_type const i)
                        {
                        	{
					libmaus::parallel::ScopePosixSpinLock llock(lock);
        	                	Q.push_front(i);
                        	}
                        	semaphore.post();
                        }
                };                
        }
}
#endif
#endif
