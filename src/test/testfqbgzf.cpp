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
#include <libmaus/util/ArgInfo.hpp>
#include <libmaus/fastx/SocketFastQReader.hpp>
#include <libmaus/util/PushBuffer.hpp>
#include <libmaus/lz/BgzfDeflateParallel.hpp>
#include <libmaus/util/TempFileRemovalContainer.hpp>

struct PatternBuffer : public libmaus::util::PushBuffer<libmaus::fastx::SocketFastQReader::pattern_type>
{
	PatternBuffer()
	{
	
	}
	
	libmaus::fastx::SocketFastQReader::pattern_type & getNextPattern()
	{
		return get();
	}	
};

int main(int argc, char * argv[])
{
	try
	{
		libmaus::util::ArgInfo const arginfo(argc,argv);
		
		if ( !arginfo.hasArg("index") )
		{
			libmaus::exception::LibMausException se;
			se.getStream() << "Please set the index key (e.g. index=file.idx)" << std::endl;
			se.finish();
			throw se;
		}

		std::string const indexfilename = arginfo.getUnparsedValue("index","file.idx" /* unused */);
		std::string const deftmp = arginfo.getDefaultTmpFileName();
		std::string const fifilename = deftmp + ".fi";
		std::string const bgzfidxfilename = deftmp + ".bgzfidx";
		libmaus::util::TempFileRemovalContainer::addTempFile(fifilename);
		libmaus::util::TempFileRemovalContainer::addTempFile(bgzfidxfilename);
	
		::libmaus::network::SocketBase fdin(STDIN_FILENO);
		typedef ::libmaus::fastx::SocketFastQReader reader_type;
		reader_type reader(&fdin,0 /* q offset */);
		
		libmaus::aio::CheckedOutputStream::unique_ptr_type bgzfidoutstr(
			new libmaus::aio::CheckedOutputStream(bgzfidxfilename)
		);
		libmaus::aio::CheckedOutputStream::unique_ptr_type fioutstr(
			new libmaus::aio::CheckedOutputStream(fifilename)
		);
		libmaus::lz::BgzfDeflateParallel::unique_ptr_type 
			bgzfenc(
				new libmaus::lz::BgzfDeflateParallel(
					std::cout,32,128,Z_DEFAULT_COMPRESSION,
					bgzfidoutstr.get()
				)
			);
		
		typedef reader_type::pattern_type pattern_type;
		
		PatternBuffer buf;
		uint64_t fqacc = 0;
		uint64_t patacc = 0;
		uint64_t numblocks = 0;
		libmaus::autoarray::AutoArray<char> outbuf(libmaus::lz::BgzfConstants::getBgzfMaxBlockSize(),false);
	
		while ( reader.getNextPatternUnlocked(buf.getNextPattern()) )
		{
			uint64_t const patid = buf.f-1;
			pattern_type & pattern = buf.A[patid];
		
			uint64_t const qlen = pattern.spattern.size();
			uint64_t const nlen = pattern.sid.size();
			uint64_t const fqlen =  
				1 + nlen + 1 + 
				qlen + 1 + 
				1 + 1 + 
				qlen + 1
			;

			if ( fqacc + fqlen <= libmaus::lz::BgzfConstants::getBgzfMaxBlockSize() )
			{
				fqacc += fqlen;
			}
			else
			{
				char * outp = outbuf.begin();
				uint64_t lnumsyms = 0;
				uint64_t minlen = std::numeric_limits<uint64_t>::max();
				uint64_t maxlen = 0;
				
				for ( uint64_t i = 0; i < patid; ++i )
				{
					*(outp)++ = '@';
					std::copy(buf.A[i].sid.begin(), buf.A[i].sid.end(),outp);
					outp += buf.A[i].sid.size();
					*(outp++) = '\n';

					std::copy(buf.A[i].spattern.begin(), buf.A[i].spattern.end(),outp);
					outp += buf.A[i].spattern.size();
					lnumsyms += buf.A[i].spattern.size();
					minlen = std::min(minlen,static_cast<uint64_t>(buf.A[i].spattern.size()));
					maxlen = std::max(maxlen,static_cast<uint64_t>(buf.A[i].spattern.size()));
					*(outp++) = '\n';

					*(outp)++ = '+';
					*(outp++) = '\n';

					std::copy(buf.A[i].quality.begin(), buf.A[i].quality.end(),outp);
					outp += buf.A[i].quality.size();
					*(outp++) = '\n';
				}
				
				//std::cerr << "expect " << fqacc << " got " << outp-outbuf.begin() << std::endl;
				
				assert ( outp - outbuf.begin() == static_cast<ptrdiff_t>(fqacc) );
				
				// std::cout.write(outbuf.begin(),outp-outbuf.begin());
				bgzfenc->writeSynced(outbuf.begin(),outp-outbuf.begin());

				uint64_t const fqlow  = patacc;
				uint64_t const fqhigh = fqlow + patid;
				uint64_t const fqfileoffset = 0;
				uint64_t const fqfileoffsethigh = 0;
				uint64_t const fqnumsyms = lnumsyms;
				uint64_t const fqminlen = minlen;
				uint64_t const fqmaxlen = maxlen;
				
				libmaus::fastx::FastInterval FI(
					fqlow,fqhigh,fqfileoffset,fqfileoffsethigh,
					fqnumsyms,fqminlen,fqmaxlen
				);
				
				// std::cerr << "*" << FI << std::endl;
				
				(*fioutstr) << FI.serialise();
				
				numblocks++;

				fqacc = fqlen;
				buf.A[0] = buf.A[patid];
				buf.f = 1;
				
				patacc += patid;
				
				// std::cerr << "flushed." << std::endl;
			}
		}
		
		buf.f -= 1;

		char * outp = outbuf.begin();
		uint64_t lnumsyms = 0;
		uint64_t minlen = std::numeric_limits<uint64_t>::max();
		uint64_t maxlen = 0;
		
		for ( uint64_t i = 0; i < buf.f; ++i )
		{
			*(outp)++ = '@';
			std::copy(buf.A[i].sid.begin(), buf.A[i].sid.end(),outp);
			outp += buf.A[i].sid.size();
			*(outp++) = '\n';

			std::copy(buf.A[i].spattern.begin(), buf.A[i].spattern.end(),outp);
			outp += buf.A[i].spattern.size();
			lnumsyms += buf.A[i].spattern.size();
			minlen = std::min(minlen,static_cast<uint64_t>(buf.A[i].spattern.size()));
			maxlen = std::max(maxlen,static_cast<uint64_t>(buf.A[i].spattern.size()));
			*(outp++) = '\n';

			*(outp)++ = '+';
			*(outp++) = '\n';

			std::copy(buf.A[i].quality.begin(), buf.A[i].quality.end(),outp);
			outp += buf.A[i].quality.size();
			*(outp++) = '\n';
		}
		
		// std::cerr << "expecting " << fqacc << " got " << outp - outbuf.begin() << std::endl;
		
		// std::cout.write(outbuf.begin(),outp-outbuf.begin());
		bgzfenc->writeSynced(outbuf.begin(),outp-outbuf.begin());

		uint64_t const fqlow  = patacc;
		uint64_t const fqhigh = fqlow + buf.f;
		uint64_t const fqfileoffset = 0;
		uint64_t const fqfileoffsethigh = 0;
		uint64_t const fqnumsyms = lnumsyms;
		uint64_t const fqminlen = minlen;
		uint64_t const fqmaxlen = maxlen;
		
		libmaus::fastx::FastInterval FI(
			fqlow,fqhigh,fqfileoffset,fqfileoffsethigh,
			fqnumsyms,fqminlen,fqmaxlen
		);

		// std::cerr << "*" << FI << std::endl;

		(*fioutstr) << FI.serialise();

		numblocks++;

		assert ( outp - outbuf.begin() == static_cast<ptrdiff_t>(fqacc) );
				
		fqacc = 0;
		buf.f = 0;
		
		bgzfenc->flush();
		bgzfenc.reset();
		bgzfidoutstr->flush();
		bgzfidoutstr.reset();
		fioutstr->flush();
		fioutstr.reset();
		
		libmaus::aio::CheckedInputStream fiin(fifilename);
		libmaus::aio::CheckedInputStream bgzfidxin(bgzfidxfilename);
		libmaus::fastx::FastInterval rFI;
		
		uint64_t filow = 0;
		uint64_t fihigh = 0;
		
		libmaus::aio::CheckedOutputStream indexCOS(indexfilename);
		uint64_t const combrate = 4;
		::libmaus::util::NumberSerialisation::serialiseNumber(indexCOS,(numblocks+combrate-1)/combrate);
		std::vector < libmaus::fastx::FastInterval > FIV;
		
		for ( uint64_t i = 0; i < numblocks; ++i )
		{
			rFI = libmaus::fastx::FastInterval::deserialise(fiin);
			/* uint64_t const uncomp = */ libmaus::util::UTF8::decodeUTF8(bgzfidxin);
			uint64_t const comp = libmaus::util::UTF8::decodeUTF8(bgzfidxin);
			
			fihigh += comp;
			
			rFI.fileoffset = filow;
			rFI.fileoffsethigh = fihigh;
			
			// std::cerr << rFI << std::endl;
			
			FIV.push_back(rFI);
			
			if ( FIV.size() == combrate )
			{
				indexCOS << libmaus::fastx::FastInterval::merge(FIV.begin(),FIV.end()).serialise();
				FIV.clear();
			}
			// indexCOS << rFI.serialise();
			
			filow = fihigh;
		}
		
		if ( FIV.size() )
		{
			indexCOS << libmaus::fastx::FastInterval::merge(FIV.begin(),FIV.end()).serialise();
			FIV.clear();	
		}
		
		indexCOS.flush();
		indexCOS.close();
	}
	catch(std::exception const & ex)
	{
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
}
