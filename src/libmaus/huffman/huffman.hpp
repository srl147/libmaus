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

#if ! defined(HUFFMAN_HPP)
#define HUFFMAN_HPP

#include <libmaus/huffman/HuffmanTreeLeaf.hpp>
#include <libmaus/huffman/HuffmanTreeInnerNode.hpp>
#include <libmaus/util/stringFunctions.hpp>
#include <libmaus/bitio/getBit.hpp>

namespace libmaus
{
	namespace huffman
	{
		inline ::libmaus::huffman::HuffmanTreeNode * ::libmaus::huffman::HuffmanTreeNode::deserialize(
			uint64_t const * const struc,
			uint64_t & istruc,
			int64_t const * const symb,
			uint64_t & isymb
			)
		{
			assert ( ! bitio::getBit(struc, istruc) );
			istruc++;
			
			HuffmanTreeNode * N = 0;
			HuffmanTreeNode * left = 0;
			HuffmanTreeNode * right = 0;
			
			try
			{
				// leaf (closing bracket)
				if ( bitio::getBit(struc, istruc) )
				{
					N = new HuffmanTreeLeaf(symb[isymb++],0);
				}
				// inner (opening bracket)
				else
				{
					left = deserialize(struc,istruc,symb,isymb);
					right = deserialize(struc,istruc,symb,isymb);
					N = new HuffmanTreeInnerNode(left,right,0);
					left = 0;
					right = 0;
				}
			}
			catch(...)
			{
				delete left;
				delete right;
				delete N;
				throw;
			}	
			
			assert ( bitio::getBit(struc, istruc) );
			istruc++;
			
			return N;
		}

		inline ::libmaus::util::shared_ptr < libmaus::huffman::HuffmanTreeNode >::type libmaus::huffman::HuffmanTreeNode::simpleDeserialise(::std::istream & in)
		{
			::std::string line;
			::std::getline(in,line);

			::std::deque< ::std::string> tokens = ::libmaus::util::stringFunctions::tokenize< ::std::string>(line,"\t");

			if ( tokens.size() != 2 || (tokens[0] != "HuffmanTreeNode") )
				throw ::std::runtime_error("Malformed input in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");
			
			uint64_t const numnodes = atol( tokens[1].c_str() );
			::std::vector < ::libmaus::huffman::HuffmanTreeNode * > nodes(numnodes);

			try
			{
				::libmaus::util::shared_ptr < ::libmaus::huffman::HuffmanTreeNode >::type sroot;
				
				for ( uint64_t i = 0; i < numnodes; ++i )
				{
					::std::getline(in,line);
					
					if ( ! in )
						throw ::std::runtime_error("Stream invalid before read complete in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");
					
					tokens = ::libmaus::util::stringFunctions::tokenize< ::std::string>(line,"\t");	

					if ( tokens.size() < 2 )
						throw ::std::runtime_error("Invalid input in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");
					
					if ( tokens[1] == "inner" )
					{
						if ( tokens.size() != 4 )
							throw ::std::runtime_error("Invalid input in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");

						uint64_t const nodeid = atol( tokens[0].c_str() );
						
						if ( nodeid >= numnodes )
							throw ::std::runtime_error("Invalid node id in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");
					
						if ( nodes[nodeid] )
							throw ::std::runtime_error("Repeated node in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");
						
						uint64_t const left = atol(tokens[2].c_str());
						uint64_t const right = atol(tokens[3].c_str());

						if ( left >= numnodes || (!nodes[left]) )
							throw ::std::runtime_error("Invalid node id in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");
						if ( right >= numnodes || (!nodes[right]) )
							throw ::std::runtime_error("Invalid node id in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");
							
						nodes [ nodeid ] = new  ::libmaus::huffman::HuffmanTreeInnerNode(0,0,nodes[left]->getFrequency() + nodes[right]->getFrequency() );
						
						if ( left )
							dynamic_cast < ::libmaus::huffman::HuffmanTreeInnerNode * > (nodes [ nodeid ])->left = nodes[left]; nodes[left] = 0;
						if ( right )
							dynamic_cast < ::libmaus::huffman::HuffmanTreeInnerNode * > (nodes [ nodeid ])->right = nodes[right]; nodes[right] = 0;
					}
					else if ( tokens[1] == "leaf" )
					{
						if ( tokens.size() != 4 )
							throw ::std::runtime_error("Invalid input in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");

						uint64_t const nodeid = atol( tokens[0].c_str() );
						
						if ( nodeid >= numnodes )
							throw ::std::runtime_error("Invalid node id in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");

						int const symbol = atoi(tokens[2].c_str());
						uint64_t const frequency = atol(tokens[3].c_str());
						
						nodes[nodeid] = new  ::libmaus::huffman::HuffmanTreeLeaf(symbol,frequency);
					}
					else
					{
						throw ::std::runtime_error("Invalid node type in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");
					}                                                                                                                
				}
				
				if ( ! nodes[0] )
					throw ::std::runtime_error("No root node produced in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");
					
				for ( uint64_t i = 1; i < numnodes; ++i )
					if ( nodes[i] )
						throw ::std::runtime_error("Unlinked node produced in ::libmaus::huffman::HuffmanTreeNode::simpleDeserialise()");

				sroot = ::libmaus::util::shared_ptr< ::libmaus::huffman::HuffmanTreeNode>::type(nodes[0]);
				nodes[0] = 0;
			
				return sroot;
			}
			catch(...)
			{
				for ( uint64_t i = 0; i < nodes.size(); ++i )
					delete  nodes[i];
				throw;
			}
		}
	}
}

#include <libmaus/huffman/HuffmanBase.hpp>

#include <libmaus/huffman/EncodeTable.hpp>
#include <libmaus/huffman/CompressTable.hpp>
#include <libmaus/huffman/DecodeTable.hpp>

#include <libmaus/huffman/hufEncodeString.hpp>
#include <libmaus/huffman/HuffmanSorting.hpp>

#endif
