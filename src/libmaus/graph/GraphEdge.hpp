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

#include <libmaus/graph/FilteredGraphTypes.hpp>
#include <libmaus/lcs/OverlapOrientation.hpp>

#if ! defined(LIBMAUS_GRAPH_GRAPHEDGE_HPP)
#define LIBMAUS_GRAPH_GRAPHEDGE_HPP

namespace libmaus
{
	namespace graph
	{
		struct GraphEdge
		{
			::libmaus::graph::FilteredGraphTypes::filtered_graph_target_type s;
			::libmaus::graph::FilteredGraphTypes::filtered_graph_target_type t;
			::libmaus::graph::FilteredGraphTypes::filtered_graph_weight_type w;
			::libmaus::graph::FilteredGraphTypes::filtered_graph_orientation_type o;
			
			GraphEdge() : s(0), t(0), w(0), o(0) {}
			GraphEdge(
				::libmaus::graph::FilteredGraphTypes::filtered_graph_target_type const rs,
				::libmaus::graph::FilteredGraphTypes::filtered_graph_target_type const rt,
				::libmaus::graph::FilteredGraphTypes::filtered_graph_weight_type const rw,
				::libmaus::graph::FilteredGraphTypes::filtered_graph_orientation_type const ro
			)
			: s(rs), t(rt), w(rw), o(ro)
			{
			
			}
			
			bool operator<(GraphEdge const & o) const
			{
				if ( s != o.s )
					return s < o.s;
				else
					return t < o.t;
			}
			
			bool operator>(GraphEdge const & o) const
			{
				if ( s != o.s )
					return s > o.s;
				else
					return t > o.t;
			}
			
			bool operator==(GraphEdge const & o) const
			{
				return
					s == o.s && t == o.t;
			}
			
			bool operator!=(GraphEdge const & o) const
			{
				return !(*this == o);
			}
			
			static bool isCover(::libmaus::graph::FilteredGraphTypes::filtered_graph_orientation_type o)
			{
				switch ( o )
				{
					case libmaus::lcs::OverlapOrientation::overlap_a_covers_b:
					case libmaus::lcs::OverlapOrientation::overlap_ar_covers_b:
					case libmaus::lcs::OverlapOrientation::overlap_b_covers_a:
					case libmaus::lcs::OverlapOrientation::overlap_b_covers_ar:
					case libmaus::lcs::OverlapOrientation::overlap_cover_complete:
					case libmaus::lcs::OverlapOrientation::overlap_a_complete_b:
					case libmaus::lcs::OverlapOrientation::overlap_ar_complete_b:
						return true;
					default:
						return false;
				}
			}
			
			void fixCoverWeight()
			{
				if ( isCover(o) )
					w = 0;
			}
			
			GraphEdge inverse(uint16_t const * patlen) const
			{
				uint64_t const slen = patlen[s];
				uint64_t const tlen = patlen[t];
				uint64_t const overlap = tlen-w;
				uint64_t const revoverhang = isCover(o) ? 0 : slen-overlap;
				
				return 
					GraphEdge(
						t,s,
						revoverhang,
						libmaus::lcs::OverlapOrientation::getInverse(libmaus::lcs::OverlapOrientation::overlap_orientation(o))
					);
			}
			GraphEdge inverse(uint64_t const slen, uint64_t const tlen) const
			{
				uint64_t const overlap = tlen-w;
				uint64_t const revoverhang = isCover(o) ? 0 : slen-overlap;
				
				return 
					GraphEdge(
						t,s,
						revoverhang,
						libmaus::lcs::OverlapOrientation::getInverse(libmaus::lcs::OverlapOrientation::overlap_orientation(o))
					);
			}
		};
		
		struct GraphEdgeTransitiveDecoverOrder
		{
			GraphEdgeTransitiveDecoverOrder() {}
		
			bool operator()(libmaus::graph::GraphEdge const & A, libmaus::graph::GraphEdge const & B) const
			{
				int const da = 
					libmaus::lcs::OverlapOrientation::isDovetail(static_cast<libmaus::lcs::OverlapOrientation::overlap_orientation>(A.o)) ? 1 : 0;
				int const db = 
					libmaus::lcs::OverlapOrientation::isDovetail(static_cast<libmaus::lcs::OverlapOrientation::overlap_orientation>(B.o)) ? 1 : 0;
				
				if ( da ^ db )
				{
					return da != 0;
				}
				else
				{
					return A.w < B.w;
				}
			}
		};

		std::ostream & operator<<(std::ostream & out, GraphEdge const & GE);
	}
}
#endif
