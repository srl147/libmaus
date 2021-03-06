/**
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
**/
#if ! defined(LIBMAUS_AIO_CIRCULARWRAPPER_HPP)
#define LIBMAUS_AIO_CIRCULARWRAPPER_HPP

#include <libmaus/aio/CircularBuffer.hpp>
#include <libmaus/aio/CheckedInputStream.hpp>
#include <libmaus/bitio/CompactDecoderBuffer.hpp>
#include <libmaus/bitio/PacDecoderBuffer.hpp>
#include <libmaus/util/unique_ptr.hpp>
#include <libmaus/util/shared_ptr.hpp>

namespace libmaus
{
	namespace aio
	{
		struct CircularWrapper : public CircularBuffer, public ::std::istream
		{
			typedef CircularWrapper this_type;
			typedef ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef ::libmaus::util::shared_ptr<this_type>::type shared_ptr_type;
		
			CircularWrapper(
				std::string const & filename, 
				uint64_t const offset = 0,
				uint64_t const buffersize = 64*1024, 
				uint64_t const pushbackspace = 64
			)
			: CircularBuffer(filename,offset,buffersize,pushbackspace), ::std::istream(this)
			{
				
			}
			CircularWrapper(
				std::istream & rin, 
				uint64_t const offset = 0,
				uint64_t const buffersize = 64*1024, 
				uint64_t const pushbackspace = 64
			)
			: CircularBuffer(rin,offset,buffersize,pushbackspace), ::std::istream(this)
			{
				
			}
			uint64_t tellg() const
			{
				return CircularBuffer::tellg();
			}
		};

		struct Utf8CircularWrapper : public Utf8CircularBuffer, public ::std::wistream
		{
			typedef Utf8CircularWrapper this_type;
			typedef ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef ::libmaus::util::shared_ptr<this_type>::type shared_ptr_type;
		
			Utf8CircularWrapper(
				std::string const & filename, 
				uint64_t const offset = 0,
				uint64_t const buffersize = 64*1024, 
				uint64_t const pushbackspace = 64
			)
			: Utf8CircularBuffer(filename,offset,buffersize,pushbackspace), ::std::wistream(this)
			{
				
			}
			Utf8CircularWrapper(
				std::wistream & rin, 
				uint64_t const offset = 0,
				uint64_t const buffersize = 64*1024, 
				uint64_t const pushbackspace = 64
			)
			: Utf8CircularBuffer(rin,offset,buffersize,pushbackspace), ::std::wistream(this)
			{
				
			}
			uint64_t tellg() const
			{
				return Utf8CircularBuffer::tellg();
			}
		};


		struct CircularReverseWrapper : public CircularReverseBuffer, public ::std::istream
		{
			typedef CircularReverseWrapper this_type;
			typedef ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef ::libmaus::util::shared_ptr<this_type>::type shared_ptr_type;

			CircularReverseWrapper(
				std::string const & filename, 
				uint64_t const offset = 0,
				uint64_t const buffersize = 64*1024, 
				uint64_t const pushbackspace = 64
			)
			: CircularReverseBuffer(filename,offset,buffersize,pushbackspace), ::std::istream(this)
			{
				
			}
			CircularReverseWrapper(
				std::istream & rin, 
				uint64_t const offset = 0,
				uint64_t const buffersize = 64*1024, 
				uint64_t const pushbackspace = 64
			)
			: CircularReverseBuffer(rin,offset,buffersize,pushbackspace), ::std::istream(this)
			{
				
			}
			uint64_t tellg() const
			{
				return CircularReverseBuffer::tellg();
			}
		};

		struct Utf8CircularReverseWrapper : public Utf8CircularReverseBuffer, public ::std::wistream
		{
			typedef Utf8CircularReverseWrapper this_type;
			typedef ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef ::libmaus::util::shared_ptr<this_type>::type shared_ptr_type;

			Utf8CircularReverseWrapper(
				std::string const & filename, 
				uint64_t const offset = 0,
				uint64_t const buffersize = 64*1024, 
				uint64_t const pushbackspace = 64
			)
			: Utf8CircularReverseBuffer(filename,offset,buffersize,pushbackspace), ::std::wistream(this)
			{
				
			}
			Utf8CircularReverseWrapper(
				std::wistream & rin, 
				uint64_t const offset = 0,
				uint64_t const buffersize = 64*1024, 
				uint64_t const pushbackspace = 64
			)
			: Utf8CircularReverseBuffer(rin,offset,buffersize,pushbackspace), ::std::wistream(this)
			{
				
			}
			uint64_t tellg() const
			{
				return Utf8CircularReverseBuffer::tellg();
			}
		};
		
		struct CheckedInputStreamWrapper
		{
			::libmaus::aio::CheckedInputStream stream;
			
			CheckedInputStreamWrapper(std::string const & filename)
			: stream(filename)
			{
			
			}
		};

		struct Utf8DecoderWrapperWrapper
		{
			::libmaus::util::Utf8DecoderWrapper stream;
			
			Utf8DecoderWrapperWrapper(std::string const & filename) : stream(filename)
			{
			
			}
		};
		
		struct CompactDecoderWrapperWrapper
		{
			::libmaus::bitio::CompactDecoderWrapper stream;
			
			CompactDecoderWrapperWrapper(std::string const & filename)
			: stream(filename)
			{
			
			}
		};

		struct PacDecoderWrapperWrapper
		{
			::libmaus::bitio::PacDecoderWrapper stream;
			
			PacDecoderWrapperWrapper(std::string const & filename)
			: stream(filename)
			{
			
			}
		};

		struct PacDecoderTermWrapperWrapper
		{
			::libmaus::bitio::PacDecoderTermWrapper stream;
			
			PacDecoderTermWrapperWrapper(std::string const & filename)
			: stream(filename)
			{
			
			}
		};
		
		struct CheckedCircularWrapper : public CheckedInputStreamWrapper, public CircularWrapper
		{
			CheckedCircularWrapper(std::string const & filename, uint64_t const offset, uint64_t const buffersize = 64*1024, uint64_t const pushbackspace = 64)
			: CheckedInputStreamWrapper(filename), CircularWrapper(CheckedInputStreamWrapper::stream,offset,buffersize,pushbackspace)
			{
			
			}
		};

		struct CompactCircularWrapper : public CompactDecoderWrapperWrapper, public CircularWrapper
		{
			CompactCircularWrapper(std::string const & filename, uint64_t const offset, uint64_t const buffersize = 64*1024, uint64_t const pushbackspace = 64)
			: CompactDecoderWrapperWrapper(filename), CircularWrapper(CompactDecoderWrapperWrapper::stream,offset,buffersize,pushbackspace)
			{
			
			}
		};

		struct PacCircularWrapper : public PacDecoderWrapperWrapper, public CircularWrapper
		{
			PacCircularWrapper(std::string const & filename, uint64_t const offset, uint64_t const buffersize = 64*1024, uint64_t const pushbackspace = 64)
			: PacDecoderWrapperWrapper(filename), CircularWrapper(PacDecoderWrapperWrapper::stream,offset,buffersize,pushbackspace)
			{
			
			}
		};

		struct PacTermCircularWrapper : public PacDecoderTermWrapperWrapper, public CircularWrapper
		{
			PacTermCircularWrapper(std::string const & filename, uint64_t const offset, uint64_t const buffersize = 64*1024, uint64_t const pushbackspace = 64)
			: PacDecoderTermWrapperWrapper(filename), CircularWrapper(PacDecoderTermWrapperWrapper::stream,offset,buffersize,pushbackspace)
			{
			
			}
		};

		struct Utf8CircularWrapperWrapper : public Utf8DecoderWrapperWrapper, public Utf8CircularWrapper
		{
			Utf8CircularWrapperWrapper(std::string const & filename, uint64_t const offset, uint64_t const buffersize = 64*1024, uint64_t const pushbackspace = 64)
			: Utf8DecoderWrapperWrapper(filename), Utf8CircularWrapper(Utf8DecoderWrapperWrapper::stream,offset,buffersize,pushbackspace)
			{
			
			}
		};

		struct CheckedCircularReverseWrapper : public CheckedInputStreamWrapper, public CircularReverseWrapper
		{
			CheckedCircularReverseWrapper(std::string const & filename, uint64_t const offset, uint64_t const buffersize = 64*1024, uint64_t const pushbackspace = 64)
			: CheckedInputStreamWrapper(filename), CircularReverseWrapper(CheckedInputStreamWrapper::stream,offset,buffersize,pushbackspace)
			{
			
			}
		};

		struct CompactCircularReverseWrapper : public CompactDecoderWrapperWrapper, public CircularReverseWrapper
		{
			CompactCircularReverseWrapper(std::string const & filename, uint64_t const offset, uint64_t const buffersize = 64*1024, uint64_t const pushbackspace = 64)
			: CompactDecoderWrapperWrapper(filename), CircularReverseWrapper(CompactDecoderWrapperWrapper::stream,offset,buffersize,pushbackspace)
			{
			
			}
		};

		struct PacCircularReverseWrapper : public PacDecoderWrapperWrapper, public CircularReverseWrapper
		{
			PacCircularReverseWrapper(std::string const & filename, uint64_t const offset, uint64_t const buffersize = 64*1024, uint64_t const pushbackspace = 64)
			: PacDecoderWrapperWrapper(filename), CircularReverseWrapper(PacDecoderWrapperWrapper::stream,offset,buffersize,pushbackspace)
			{
			
			}
		};

		struct PacTermCircularReverseWrapper : public PacDecoderTermWrapperWrapper, public CircularReverseWrapper
		{
			PacTermCircularReverseWrapper(std::string const & filename, uint64_t const offset, uint64_t const buffersize = 64*1024, uint64_t const pushbackspace = 64)
			: PacDecoderTermWrapperWrapper(filename), CircularReverseWrapper(PacDecoderTermWrapperWrapper::stream,offset,buffersize,pushbackspace)
			{
			
			}
		};

		struct Utf8CircularReverseWrapperWrapper : public Utf8DecoderWrapperWrapper, public Utf8CircularReverseWrapper
		{
			Utf8CircularReverseWrapperWrapper(std::string const & filename, uint64_t const offset, uint64_t const buffersize = 64*1024, uint64_t const pushbackspace = 64)
			: Utf8DecoderWrapperWrapper(filename), Utf8CircularReverseWrapper(Utf8DecoderWrapperWrapper::stream,offset,buffersize,pushbackspace)
			{
			
			}
		};
	}
}
#endif
