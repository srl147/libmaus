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
#include <libmaus/autoarray/AutoArray.hpp>

#if defined(_OPENMP)
#include <omp.h>
#endif

namespace libmaus
{
	namespace rank
	{
		template<typename _value_type, typename _rank_type>
		struct FastRank
		{
			typedef _value_type value_type;
			typedef _rank_type rank_type;
			typedef FastRank<value_type,rank_type> this_type;
			typedef typename libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			
			uint64_t n;
			libmaus::autoarray::AutoArray<value_type> AV;
			value_type * V;	
			libmaus::autoarray::AutoArray<rank_type> R;
			int64_t maxsym;
			uint64_t mod;
			uint64_t mask;
			uint64_t invmask;
			
			uint64_t rank(int64_t const sym, uint64_t const i) const
			{
				if ( expect_false(sym > maxsym) )
					return 0;

				uint64_t j, r;
				if ( expect_false(static_cast<int64_t>(i) < sym) )
				{
					j = 0;
					r = (V[0]==sym);
				}
				else
				{
					j = ((i-sym)&invmask)+sym;
					r = R[j];
				}
				
				while ( j != i )
					r += (V[++j] == sym);
				
				return r;
			}
			
			template<typename iterator>
			FastRank(
				iterator it, 
				uint64_t const rn, 
				int64_t rmaxsym = std::numeric_limits<int64_t>::min()
			)
			: n(rn), AV(n,false), V(AV.begin())
			{
				#if defined(_OPENMP)
				uint64_t const numthreads = omp_get_max_threads();
				#else
				uint64_t const numthreads = 1;
				#endif
				
				libmaus::parallel::OMPLock lock;

				if ( rmaxsym == std::numeric_limits<int64_t>::min() )
				{
					uint64_t nperpack = (n+numthreads-1)/numthreads;
					int64_t  packs = (n + nperpack - 1)/nperpack;
					
					#if defined(_OPENMP)
					#pragma omp parallel for
					#endif
					for ( int64_t p = 0; p < packs; ++p )
					{
						lock.lock();
						int64_t lmax = std::numeric_limits<int64_t>::min();
						lock.unlock();
						
						uint64_t const low = p * nperpack;
						uint64_t const high = std::min(low+nperpack,n);
						
						iterator itc = it + low;
						iterator ite = it + high;
						value_type * O = V + low;
						
						for ( ; itc != ite; ++itc )
						{
							lmax = std::max(
								lmax,static_cast<int64_t>(*itc)
								);
							*(O++) = *itc;
						}
							
						lock.lock();
						rmaxsym = std::max(rmaxsym,lmax);
						lock.unlock();
					}
					
					maxsym = rmaxsym;
				
					if ( n > 0 && maxsym < 0 )
					{
						libmaus::exception::LibMausException lme;
						lme.getStream() << "FastRank: maximum symbol code is smaller than 0" << std::endl;
						lme.finish();
						throw lme;
					}

					#if 0
					// std::cerr << "maxsym=" << maxsym << std::endl;
					for ( uint64_t i = 0; i < n; ++i )
						assert ( V[i] == it[i] );
					#endif
					
					mod = libmaus::math::nextTwoPow(maxsym+1);
					mask = mod-1;
					invmask = ~mask;

					uint64_t const div = libmaus::math::lcm(numthreads,mod);
					nperpack = (n+div-1)/div;
					packs = (n + nperpack - 1)/nperpack;
					
					// std::cerr << "mod=" << mod << " mask=" << std::hex << mask << " invmask=" << invmask << std::dec << std::endl;
					
					libmaus::autoarray::AutoArray<uint64_t> Alhist(mod * packs,false);

					#if defined(_OPENMP)
					#pragma omp parallel for
					#endif
					for ( int64_t p = 0; p < packs; ++p )
					{
						uint64_t * const lhist = Alhist.begin() + p * mod;
						std::fill(lhist,lhist+mod,0ull);

						uint64_t const low = p * nperpack;
						uint64_t const high = std::min(low+nperpack,n);
						
						for ( value_type * i = V + low; i != V + high; ++i )
							lhist[*i]++;
					}
					
					// compute prefix sums
					for ( uint64_t i = 0; i < mod; ++i )
					{
						uint64_t acc = 0;
						
						for ( int64_t j = 0; j < packs; ++j )
						{
							uint64_t const t = Alhist[
								j * mod + i
							];
							Alhist[j * mod + i] = acc;
							acc += t;
						}
					}

					#if 0
					for ( int64_t p = 0; p < packs; ++p )
					{
						uint64_t * const lhist = Alhist.begin() + p * mod;

						uint64_t const low = p * nperpack;
						
						std::map<value_type,uint64_t> M;
						for ( uint64_t i = 0; i < low; ++i )
							M [ it[i] ]++;
						
						for ( uint64_t i = 0; i < mod; ++i )
							assert ( lhist[i] == M[i] );
					}
					#endif
					
					R = libmaus::autoarray::AutoArray<rank_type>(n,false);
					
					#if defined(_OPENMP)
					#pragma omp parallel for
					#endif
					for ( int64_t p = 0; p < packs; ++p )
					{
						uint64_t * const lhist = Alhist.begin() + p * mod;

						uint64_t const low = p * nperpack;
						uint64_t const high = std::min(low+nperpack,n);
						
						for ( uint64_t i = low ; i < high; ++i )
						{
							value_type const sym = V[i];
							lhist[sym]++;
							R[i] = lhist[i & mask];
						}
					}
					
				}	
				else // if ( rmaxsym >= 0 )
				{
					maxsym = rmaxsym;
					mod = libmaus::math::nextTwoPow(maxsym+1);
					mask = mod-1;
					invmask = ~mask;

					uint64_t const div = libmaus::math::lcm(numthreads,mod);
					uint64_t const nperpack = (n+div-1)/div;
					int64_t const packs = (n + nperpack - 1)/nperpack;

					// block/thread local histograms
					libmaus::autoarray::AutoArray<uint64_t> Alhist(mod * packs,false);

					// copy data and compute histograms
					#if defined(_OPENMP)
					#pragma omp parallel for
					#endif
					for ( int64_t p = 0; p < packs; ++p )
					{
						uint64_t * const lhist = Alhist.begin() + p * mod;
						std::fill(lhist,lhist+mod,0ull);
						
						uint64_t const low = p * nperpack;
						uint64_t const high = std::min(low+nperpack,n);
						
						iterator itc = it + low;
						iterator ite = it + high;
						value_type * O = V + low;
						
						for ( ; itc != ite; ++itc )
						{
							value_type const sym = *itc;
							*(O++) = sym;
							lhist[sym]++;
						}							
					}
					
					// compute prefix sums
					for ( uint64_t i = 0; i < mod; ++i )
					{
						uint64_t acc = 0;
						
						for ( int64_t j = 0; j < packs; ++j )
						{
							uint64_t const t = Alhist[
								j * mod + i
							];
							Alhist[j * mod + i] = acc;
							acc += t;
						}
					}
					
					// compute R array
					R = libmaus::autoarray::AutoArray<rank_type>(n,false);
					
					#if defined(_OPENMP)
					#pragma omp parallel for
					#endif
					for ( int64_t p = 0; p < packs; ++p )
					{
						uint64_t * const lhist = Alhist.begin() + p * mod;

						uint64_t const low = p * nperpack;
						uint64_t const high = std::min(low+nperpack,n);
						
						for ( uint64_t i = low ; i < high; ++i )
						{
							value_type const sym = V[i];
							lhist[sym]++;
							R[i] = lhist[i & mask];
						}
					}
				}
				
				#if 0
				std::map<value_type,rank_type> M;
				for ( uint64_t i = 0; i < n; ++i )
				{
					M[it[i]]++;
					assert ( R[i] == M[i & mask] );
				}
				#endif
			}
		};
	}
}