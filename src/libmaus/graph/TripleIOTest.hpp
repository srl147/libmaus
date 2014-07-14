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

#if ! defined(TRIPLEIOTEST_HPP)
#define TRIPLEIOTEST_HPP

#include <libmaus/autoarray/AutoArray.hpp>
#include <libmaus/graph/TripleEdge.hpp>
#include <libmaus/graph/TripleEdgeInput.hpp>
#include <libmaus/graph/TripleEdgeOutput.hpp>
#include <libmaus/graph/TripleEdgeOperations.hpp>
#include <iostream>
#include <cassert>

namespace libmaus
{
	namespace graph
	{
		struct TripleIOTest
		{
			static ::libmaus::autoarray::AutoArray< ::libmaus::graph::TripleEdge > testTripleIO(
				uint64_t const n, std::string const filename, bool deleteFile = true)
			{
				std::cerr << "n=" << n << std::endl;
			
				::libmaus::autoarray::AutoArray < ::libmaus::graph::TripleEdge > edges = 
					::libmaus::graph::TripleEdge::randomValidArray(n);
					
				assert ( edges.getN() == n );
				
				typedef ::libmaus::graph::TripleEdgeOutput output_type;
				output_type::unique_ptr_type ateo(new output_type(filename, 128));
				
				for ( uint64_t i = 0; i < edges.getN(); ++i )
					ateo->write ( edges[i] );
			
				ateo->flush();
				ateo.reset(0);
			
				uint64_t const fl = ::libmaus::graph::TripleEdgeOperations::getFileLength ( filename );
				assert ( fl == n * sizeof(::libmaus::graph::TripleEdge) );
				
				::libmaus::autoarray::AutoArray < ::libmaus::graph::TripleEdge > redges(n);
				typedef ::libmaus::graph::TripleEdgeInput input_type;
				input_type::unique_ptr_type atei(new input_type(filename, 128));
			
				uint64_t j = 0;
				::libmaus::graph::TripleEdge te;
				while ( j < n )
				{
					bool const ok = atei->getNextTriple(te);
					if ( ! ok )
						std::cerr << "Failure for j=" << j << std::endl;
					assert ( ok );
					redges[j] = te;
					++j;
					
					if ( j == n )
						assert ( ! atei->getNextTriple(te) );
				}
				
				for ( uint64_t i = 0; i < n; ++i )
					assert ( edges[i] == redges[i] );
					
				if ( deleteFile )
					remove ( filename.c_str() );
				
				return edges;
			}
			
			static void testTripleMerge(uint64_t const n0, uint64_t const n1, std::string const filenameprefix)
			{
				std::string const filea = filenameprefix+"_a";
				std::string const fileb = filenameprefix+"_b";
				std::string const filem = filenameprefix+"_m";
			
				::libmaus::autoarray::AutoArray< ::libmaus::graph::TripleEdge > triplesa = testTripleIO(n0, filea, false);
				::libmaus::autoarray::AutoArray< ::libmaus::graph::TripleEdge > triplesb = testTripleIO(n1, fileb, false);
				
				std::map < std::pair<uint64_t,uint64_t>, uint64_t > M;
				for ( uint64_t i = 0; i < triplesa.getN(); ++i )
					M [ std::pair < uint64_t, uint64_t > ( triplesa[i].a, triplesa[i].b ) ] += triplesa[i].c;
				for ( uint64_t i = 0; i < triplesb.getN(); ++i )
					M [ std::pair < uint64_t, uint64_t > ( triplesb[i].a, triplesb[i].b ) ] += triplesb[i].c;
			
				::libmaus::autoarray::AutoArray< ::libmaus::graph::TripleEdge > triplesm(M.size(),false);
				uint64_t j = 0;
				for ( std::map < std::pair<uint64_t,uint64_t>, uint64_t >::const_iterator ita = M.begin(); 
					ita != M.end(); ++ita )
				{
					triplesm[j++] = ::libmaus::graph::TripleEdge (
						ita->first.first,
						ita->first.second,
						ita->second );
				}
			
				::libmaus::graph::TripleEdgeOperations::sortFile(filea);
				::libmaus::graph::TripleEdgeOperations::sortFile(fileb);
				::libmaus::graph::TripleEdgeOperations::mergeFiles(filea,fileb,filem);
				
				remove ( filea.c_str() );
				remove ( fileb.c_str() );
			
				if ( triplesa.getN() + triplesb.getN() != M.size() )
					std::cerr << "***" << std::endl;
				
				#if 0
				std::cerr << "Merging input:" << std::endl;
				for ( uint64_t i = 0; i < triplesa.getN(); ++i )
					std::cerr << "(" << triplesa[i].a << "," << triplesa[i].b << "," << triplesa[i].c << ")";
				std::cerr << std::endl;
				for ( uint64_t i = 0; i < triplesb.getN(); ++i )
					std::cerr << "(" << triplesb[i].a << "," << triplesb[i].b << "," << triplesb[i].c << ")";
				std::cerr << std::endl;
				for ( uint64_t i = 0; i < triplesm.getN(); ++i )
					std::cerr << "(" << triplesm[i].a << "," << triplesm[i].b << "," << triplesm[i].c << ")";
				std::cerr << std::endl;
				#endif
			
				::libmaus::autoarray::AutoArray < ::libmaus::graph::TripleEdge > redges( triplesm.getN() );
				typedef ::libmaus::graph::TripleEdgeInput input_type;
				input_type::unique_ptr_type atei(new input_type(filem, 128));
			
				j = 0;
				::libmaus::graph::TripleEdge te;
				while ( j < triplesm.getN() )
				{
					bool const ok = atei->getNextTriple(te);
					if ( ! ok )
						std::cerr << "Failure for j=" << j << std::endl;
					assert ( ok );
					redges[j] = te;
					
					if ( ! (redges[j] == triplesm[j] ) )
					{
						std::cerr << "Reference "
							<< "(" << triplesm[j].a << "," << triplesm[j].b << "," << triplesm[j].c << ")"
							<< " file merged "
							<< "(" << redges[j].a << "," << redges[j].b << "," << redges[j].c << ")" 
							<< std::endl;
					}
					
					assert ( redges[j] == triplesm[j] );
					
					++j;
					
					if ( j == triplesm.getN() )
						assert ( ! atei->getNextTriple(te) );
				}
			
				atei.reset(0);
			
				for ( uint64_t i = 0; i < triplesm.getN(); ++i )
					assert ( triplesm[i] == redges[i] );
				
				remove ( filem.c_str() );
			}
		};
	}
}
#endif
