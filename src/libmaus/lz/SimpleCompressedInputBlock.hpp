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
#if ! defined(LIBMAUS_LZ_SIMPLECOMPRESSEDINPUTBLOCK_HPP)
#define LIBMAUS_LZ_SIMPLECOMPRESSEDINPUTBLOCK_HPP

#include <libmaus/autoarray/AutoArray.hpp>
#include <libmaus/lz/DecompressorObjectFactory.hpp>
#include <libmaus/util/CountPutObject.hpp>
#include <libmaus/util/NumberSerialisation.hpp>
#include <libmaus/util/utf8.hpp>

namespace libmaus
{
	namespace lz
	{
		struct SimpleCompressedInputBlock
		{
			typedef SimpleCompressedInputBlock this_type;
			typedef libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef libmaus::util::shared_ptr<this_type>::type shared_ptr_type;
			
			libmaus::lz::DecompressorObject::unique_ptr_type Pdecompressor;
			libmaus::lz::DecompressorObject & decompressor;
			
			libmaus::autoarray::AutoArray<uint8_t> I;
			libmaus::autoarray::AutoArray<uint8_t> O;
			uint64_t metasize;
			uint64_t compsize;
			uint64_t uncompsize;
			bool eof;
			
			SimpleCompressedInputBlock(libmaus::lz::DecompressorObjectFactory & factory)
			: Pdecompressor(factory()), decompressor(*Pdecompressor), I(), O(), compsize(0), uncompsize(0), eof(false)
			{
			
			}
			
			template<typename stream_type>
			bool readBlock(stream_type & stream)
			{
				if ( stream.peek() == stream_type::traits_type::eof() )
				{
					eof = true;
					return true;
				}

				libmaus::util::CountPutObject CPO;
				uncompsize = libmaus::util::UTF8::decodeUTF8(stream);
				::libmaus::util::UTF8::encodeUTF8(uncompsize,CPO);
				
				compsize = ::libmaus::util::NumberSerialisation::deserialiseNumber(stream);
				::libmaus::util::NumberSerialisation::serialiseNumber(CPO,compsize);
				
				metasize = CPO.c;
				
				if ( compsize > I.size() )
					I = libmaus::autoarray::AutoArray<uint8_t>(compsize,false);
				
				stream.read(reinterpret_cast<char *>(I.begin()),compsize);
					
				if ( stream.gcount() == static_cast<int64_t>(compsize) )
					return true;
				else
					return false;
			}
			
			bool uncompressBlock()
			{
				if ( uncompsize > O.size() )
					O = libmaus::autoarray::AutoArray<uint8_t>(uncompsize,false);			

				bool const ok = decompressor.rawuncompress(
					reinterpret_cast<char const *>(I.begin()),compsize,
					reinterpret_cast<char *>(O.begin()),uncompsize
				);
				
				return ok;
			}
		};
	}
}
#endif
