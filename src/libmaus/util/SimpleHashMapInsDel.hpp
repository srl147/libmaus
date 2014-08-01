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
#if ! defined(LIBMAUS_UTIL_SIMPLEHASHMAPINSDEL_HPP)
#define LIBMAUS_UTIL_SIMPLEHASHMAPINSDEL_HPP

#include <libmaus/util/SimpleHashMapNumberCast.hpp>
#include <libmaus/util/SimpleHashMapHashCompute.hpp>
#include <libmaus/util/SimpleHashMapKeyPrint.hpp>
#include <libmaus/util/SimpleHashMapConstants.hpp>
#include <libmaus/exception/LibMausException.hpp>
#include <libmaus/autoarray/AutoArray.hpp>
#include <libmaus/hashing/hash.hpp>
#include <libmaus/parallel/OMPLock.hpp>
#include <libmaus/math/primes16.hpp>
#include <libmaus/util/NumberSerialisation.hpp>
#include <libmaus/parallel/SynchronousCounter.hpp>

namespace libmaus
{
	namespace util
	{
		template<typename _key_type, typename _value_type>
		struct SimpleHashMapInsDel : 
			public SimpleHashMapConstants<_key_type>, 
			public SimpleHashMapKeyPrint<_key_type>, 
			public SimpleHashMapHashCompute<_key_type>,
			public SimpleHashMapNumberCast<_key_type>
		{
			typedef _key_type key_type;
			typedef _value_type value_type;

			typedef SimpleHashMapConstants<key_type> base_type;
			typedef std::pair<key_type,value_type> pair_type;
			typedef SimpleHashMapInsDel<key_type,value_type> this_type;
			typedef typename ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef typename ::libmaus::util::shared_ptr<this_type>::type shared_ptr_type;

			protected:
			unsigned int slog;
			uint64_t hashsize;
			uint64_t hashmask;
			uint64_t fill;
			uint64_t deleted;
			
			// hash array
			::libmaus::autoarray::AutoArray<pair_type> H;
			// shrink array
			::libmaus::autoarray::AutoArray<pair_type> R;

			inline uint64_t hash(key_type const & v) const
			{
				return SimpleHashMapHashCompute<_key_type>::hash(v) & hashmask;
			}
			
			inline uint64_t displace(uint64_t const p, key_type const & k) const
			{
				return (p + primes16[SimpleHashMapNumberCast<key_type>::cast(k)&0xFFFFu]) & hashmask;
			}

			void extendInternal()
			{
				if ( hashsize == H.size() )
				{
					uint64_t o = 0;
					for ( uint64_t i = 0; i < hashsize; ++i )
						if ( base_type::isInUse(H[i].first) )
						{
							R[o++] = H[i];
						}
						
					slog++;
					hashsize <<= 1;
					hashmask = hashsize-1;
					fill = 0;
					deleted = 0;
					
					H = ::libmaus::autoarray::AutoArray<pair_type>(hashsize,false);
					for ( uint64_t i = 0; i < hashsize; ++i )
						H[i].first = base_type::unused();
					
					for ( uint64_t i = 0; i < o; ++i )
						insert(R[i].first,R[i].second);
						
					R = ::libmaus::autoarray::AutoArray<pair_type>(hashsize,false);					
				}
				else
				{
					assert ( hashsize < H.size() );
					assert ( 2*hashsize <= H.size() );
					
					uint64_t o = 0;
					for ( uint64_t i = 0; i < hashsize; ++i )
					{
						if ( base_type::isInUse(H[i].first) )
							R[o++] = H[i];
						H[i].first = base_type::unused();
					}
					for ( uint64_t i = hashsize; i < 2*hashsize; ++i )
						H[i].first = base_type::unused();

					hashsize <<= 1;
					slog += 1;
					hashmask = hashsize-1;
					fill = 0;
					deleted = 0;

					for ( uint64_t i = 0; i < o; ++i )
						insert(R[i].first,R[i].second);
				}
			}
			
			void shrink()
			{
				assert ( fill <= hashsize/2 );

				if ( slog )
				{
					uint64_t o = 0;
					for ( uint64_t i = 0; i < hashsize; ++i )
					{
						if ( base_type::isInUse(H[i].first) )
							R[o++] = H[i];
						H[i].first = base_type::unused();
					}
						
					hashsize >>= 1;
					slog -= 1;
					hashmask = hashsize-1;
					fill = 0;
					deleted = 0;

					for ( uint64_t i = 0; i < o; ++i )
						insert(R[i].first,R[i].second);						
				}
			}

			public:			
			SimpleHashMapInsDel(unsigned int const rslog)
			: slog(rslog), hashsize(1ull << slog), hashmask(hashsize-1), fill(0), H(hashsize,false), R(hashsize,false)
			{
				std::fill(H.begin(),H.end(),pair_type(base_type::unused(),value_type()));
			}
			virtual ~SimpleHashMapInsDel() {}

			pair_type const * begin() const { return H.begin(); }
			pair_type const * end() const { return begin()+hashsize; }
			pair_type * begin() { return H.begin(); }
			pair_type * end() { return begin()+hashsize; }
			
			uint64_t getTableSize() const
			{
				return H.size();
			}
						
			uint64_t size() const
			{
				return fill;
			}
		
			double loadFactor() const
			{
				return static_cast<double>(fill+deleted) / hashsize;
			}
			
			
			value_type const & getValue(uint64_t const i) const
			{
				return H[i].second;
			}

			value_type & getValue(uint64_t const i)
			{
				return H[i].second;
			}

			void insertExtend(key_type const & v, value_type const & w, double const loadthres)
			{
				if ( loadFactor() >= loadthres || (fill == hashsize) )
					extendInternal();
				
				insert(v,w);
			}

			// returns true if value v is contained
			bool contains(key_type const & v) const
			{
				uint64_t const p0 = hash(v);
				uint64_t p = p0;

				do
				{
					// correct value stored
					if ( H[p].first == v )
					{
						return true;
					}
					// position in use?
					else if ( H[p].first == base_type::unused() )
					{
						return false;
					}
					else
					{
						p = displace(p,v);
					}
				} while ( p != p0 );
				
				return false;
			}

			// 
			uint64_t getIndex(key_type const & v) const
			{
				uint64_t const p0 = hash(v);
				uint64_t p = p0;

				do
				{
					// correct value stored
					if ( H[p].first == v )
					{
						return p;
					}
					// position in use?
					else if ( H[p].first == base_type::unused() )
					{
						// break loop and fall through to exception below
						p = p0;
					}
					else
					{
						p = displace(p,v);
					}
				} while ( p != p0 );
				
				libmaus::exception::LibMausException lme;
				lme.getStream() << "SimpleHashMapInsDel::getIndex called for non-existing key ";
				SimpleHashMapKeyPrint<_key_type>::printKey(lme.getStream(),v);
				lme.getStream() << std::endl;
				lme.finish();
				throw lme;
			}

			// returns true if value v is contained
			bool contains(key_type const & v, value_type & r) const
			{
				uint64_t const p0 = hash(v);
				uint64_t p = p0;

				do
				{
					// correct value stored
					if ( H[p].first == v )
					{
						r = H[p].second;
						return true;
					}
					// position in use?
					else if ( H[p].first == base_type::unused() )
					{
						return false;
					}
					else
					{
						p = displace(p,v);
					}
				} while ( p != p0 );
				
				return false;
			}
			
			// get count for value v
			value_type get(key_type const & v) const
			{
				uint64_t const p0 = hash(v);
				uint64_t p = p0;

				do
				{
					// position in use?
					// correct value stored
					if ( H[p].first == v )
					{
						return H[p].second;
					}
					else if ( H[p].first == base_type::unused() )
					{
						// trigger exception below
						p = p0;
					}
					else
					{
						p = displace(p,v);
					}
				} while ( p != p0 );
				
				::libmaus::exception::LibMausException se;
				se.getStream() << "SimpleHashMapInsDel::get() called for key ";
				SimpleHashMapKeyPrint<_key_type>::printKey(se.getStream(),v);
				se.getStream() << " which is not contained." << std::endl;
				se.finish();
				throw se;
			}

			// get count for value v without checks
			value_type getUnchecked(key_type const & v) const
			{
				uint64_t p = hash(v);

				while ( true )
					// correct value stored
					if ( H[p].first == v )
						return H[p].second;
					else
						p = displace(p,v);
			}

			// 
			uint64_t getIndexUnchecked(key_type const & v) const
			{
				uint64_t p = hash(v);

				while ( true )
				{
					if ( H[p].first == v )
					{
						return p;
					}
					else
					{
						p = displace(p,v);
					}
				}				
			}

			// 
			int64_t getIndexChecked(key_type const & v) const
			{
				uint64_t const p0 = hash(v);
				uint64_t p = p0;

				do
				{
					// correct value stored
					if ( H[p].first == v )
					{
						return p;
					}
					// position in use?
					else if ( H[p].first == base_type::unused() )
					{
						// break loop and fall through to exception below
						p = p0;
					}
					else
					{
						p = displace(p,v);
					}
				} while ( p != p0 );
				
				return -1;
			}

			// insert key value pair
			void insert(key_type const & v, value_type const & w)
			{
				uint64_t const p0 = hash(v);
				uint64_t p = p0;
				
				do
				{
					// position in use?
					if ( base_type::isInUse(H[p].first) )
					{
						// key already present, replace value
						if ( H[p].first == v )
						{
							H[p].second = w;
							return;
						}
						// in use but by other value (collision)
						else
						{
							p = displace(p,v);
						}
					}
					// position is not currently in use
					else
					{
						// replacing deleted index
						if ( H[p].first == base_type::deleted() )
						{
							assert ( deleted );
							deleted--;
						}
						else
						{
							assert ( H[p].first == base_type::unused() );
						}
					
						H[p].first = v;
						H[p].second = w;						
						fill += 1;
						return;
					}
				} while ( p != p0 );
				
				::libmaus::exception::LibMausException se;
				se.getStream() << "SimpleHashMapInsDel::insert(): unable to insert, table is full." << std::endl;
				se.finish();
				throw se;
			}

			// 
			void erase(key_type const & v)
			{
				uint64_t const p0 = hash(v);
				uint64_t p = p0;

				do
				{
					// correct value found
					if ( H[p].first == v )
					{
						H[p].first = base_type::deleted();
						fill -= 1;
						deleted += 1;

						if ( deleted >= (hashsize/2) && slog )
							shrink();
							
						return;
					}
					// position in use?
					else if ( H[p].first == base_type::unused() )
					{
						// break loop and fall through to exception below
						p = p0;
					}
					else
					{
						p = displace(p,v);
					}
				} while ( p != p0 );
				
				libmaus::exception::LibMausException lme;
				lme.getStream() << "SimpleHashMapInsDel::erase called for non-existing key ";
				SimpleHashMapKeyPrint<_key_type>::printKey(lme.getStream(),v);
				lme.getStream() << std::endl;
				lme.finish();
				throw lme;
			}

			void eraseIndex(uint64_t const i)
			{
				H[i].first = base_type::deleted();
				deleted++;
				fill--;
				
				if ( deleted >= (hashsize/2) && slog )
					shrink();
			}

		};
	}
}
#endif
