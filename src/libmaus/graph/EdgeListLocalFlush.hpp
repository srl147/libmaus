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

#if ! defined(EDGELISTLOCALFLUSH_HPP)
#define EDGELISTLOCALFLUSH_HPP

#include <libmaus/graph/EdgeList.hpp>

namespace libmaus
{
	namespace graph
	{
		struct EdgeListLocalFlush
		{
			typedef ::libmaus::graph::TripleEdge edge_type;
			
			EdgeList & EL;
			
			EdgeListLocalFlush(EdgeList & rEL)
			: EL(rEL)
			{
			}
			
			void operator()(edge_type const * pa, uint64_t const n)
			{
				EL(pa,n);
			}
		};
	}
}
#endif
