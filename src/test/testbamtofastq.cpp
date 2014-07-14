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
#include <libmaus/bambam/CircularHashCollatingBamDecoder.hpp>	
#include <libmaus/util/TempFileRemovalContainer.hpp>
#include <libmaus/util/ArgInfo.hpp>

#include <libmaus/bambam/BamMultiAlignmentDecoderFactory.hpp>

#include <libmaus/lz/SimpleCompressedStreamInterval.hpp>

#include <libmaus/bambam/BamEntryContainer.hpp>
#include <libmaus/bambam/BamAlignmentReadGroupFilter.hpp>

using namespace libmaus::bambam;
using namespace libmaus::util;
using namespace std;

int main(int argc, char * argv[])
{
	try
	{
		ArgInfo const arginfo(argc,argv);
		
		libmaus::bambam::BamAlignmentDecoderWrapper::unique_ptr_type decwrapper(
			libmaus::bambam::BamMultiAlignmentDecoderFactory::construct(
				arginfo,false // put rank
			)
		);

		/* remove temporary file at program exit */
		string const tmpfilename = "tmpfile";
		TempFileRemovalContainer::addTempFile(tmpfilename);

		// collator type
		typedef libmaus::bambam::CircularHashCollatingBamDecoder collator_type;
		// typedef BamParallelCircularHashCollatingBamDecoder collator_type;
		typedef collator_type::alignment_ptr_type alignment_ptr_type;
		// set up collator
		collator_type C(
			decwrapper->getDecoder(),
			tmpfilename,
			0, // exclude
			20, // hlog
			128ull*1024ull*1024ull // sortbufsize
		);
		// collator_type C(cin,8,tmpfilename);
		pair<alignment_ptr_type,alignment_ptr_type> P;

		/* read alignments */	
		while ( C.tryPair(P) )
			/* if we have a pair, then print both ends as FastQ */
			if ( P.first && P.second )
			{
				cout << P.first->formatFastq();
				cout << P.second->formatFastq();
			}
			
		return EXIT_SUCCESS;
	}
	catch(std::exception const & ex)
	{
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
}
