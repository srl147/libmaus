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
#if ! defined(LIBMAUS_BAMBAM_BAMMULTIALIGNMENTDECODERFACTORY_HPP)
#define LIBMAUS_BAMBAM_BAMMULTIALIGNMENTDECODERFACTORY_HPP

#include <libmaus/bambam/BamAlignmentDecoderFactory.hpp>
#include <libmaus/bambam/BamMergeCoordinate.hpp>

namespace libmaus
{
	namespace bambam
	{
		struct BamMultiAlignmentDecoderFactory
		{
			typedef BamAlignmentDecoderFactory this_type;
			typedef libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef libmaus::util::shared_ptr<this_type>::type shared_ptr_type;
			
			std::vector<libmaus::bambam::BamAlignmentDecoderInfo> const BADI;
			bool const putrank;
			
			BamMultiAlignmentDecoderFactory(std::vector<libmaus::bambam::BamAlignmentDecoderInfo> const & rBADI, bool const rputrank = false) : BADI(rBADI), putrank(rputrank) {}
			virtual ~BamMultiAlignmentDecoderFactory() {}

			static std::set<std::string> getValidInputFormatsSet()
			{
				return libmaus::bambam::BamAlignmentDecoderFactory::getValidInputFormatsSet();
			}
			
			static std::string getValidInputFormats()
			{
				return libmaus::bambam::BamAlignmentDecoderFactory::getValidInputFormats();
			}
			
			libmaus::bambam::BamAlignmentDecoderWrapper::unique_ptr_type operator()() const
			{
				libmaus::bambam::BamAlignmentDecoderWrapper::unique_ptr_type tptr(construct(BADI,putrank));
				return UNIQUE_PTR_MOVE(tptr);
			}
			
			static libmaus::bambam::BamAlignmentDecoderWrapper::unique_ptr_type construct(
				std::vector<libmaus::bambam::BamAlignmentDecoderInfo> const & BADI,
				bool const putrank = false,
				std::istream & stdin = std::cin
			)
			{
				if ( ! BADI.size() || BADI.size() > 1 )
				{
					libmaus::bambam::BamAlignmentDecoderWrapper::unique_ptr_type tptr(new libmaus::bambam::BamMergeCoordinate(BADI,putrank));
					return UNIQUE_PTR_MOVE(tptr);
				}
				else
				{
					libmaus::bambam::BamAlignmentDecoderWrapper::unique_ptr_type tptr(
						libmaus::bambam::BamAlignmentDecoderFactory::construct(
							stdin,
							BADI[0].inputfilename,
							BADI[0].inputformat,
							BADI[0].inputthreads,
							BADI[0].reference,
							putrank,
							BADI[0].copystr,
							BADI[0].range)
					);

					return UNIQUE_PTR_MOVE(tptr);
				}			
			}
			
			static libmaus::bambam::BamAlignmentDecoderWrapper::unique_ptr_type construct(
				libmaus::util::ArgInfo const & arginfo,
				bool const putrank = false, 
				std::ostream * copystr = 0,
				std::istream & stdin = std::cin)
			{
				std::vector<std::string> const I = arginfo.getPairValues("I");
				std::string const inputformat = arginfo.getValue<std::string>("inputformat",libmaus::bambam::BamAlignmentDecoderInfo::getDefaultInputFormat());
				uint64_t const inputthreads = arginfo.getValue<uint64_t>("inputthreads",libmaus::bambam::BamAlignmentDecoderInfo::getDefaultThreads());
				std::string const reference = arginfo.getUnparsedValue("reference",libmaus::bambam::BamAlignmentDecoderInfo::getDefaultReference());
				std::string const prange = arginfo.getUnparsedValue("range",libmaus::bambam::BamAlignmentDecoderInfo::getDefaultRange());
				std::string const pranges = arginfo.getUnparsedValue("ranges",std::string(""));
				std::string const range = pranges.size() ? pranges : prange;

				std::vector<libmaus::bambam::BamAlignmentDecoderInfo> V;
				for ( uint64_t i = 0; i < I.size(); ++i )
					V.push_back(
						libmaus::bambam::BamAlignmentDecoderInfo(
							I[i],
							inputformat,
							inputthreads,
							reference,
							false,
							copystr,
							range
						)
					);
					
				if ( ! I.size() )
					V.push_back(
						libmaus::bambam::BamAlignmentDecoderInfo(
							std::string("-"),
							inputformat,
							inputthreads,
							reference,
							false,
							copystr,
							range
						)
					);
					
				libmaus::bambam::BamAlignmentDecoderWrapper::unique_ptr_type tptr(construct(V,putrank,stdin));
				return UNIQUE_PTR_MOVE(tptr);
			}
		};
	}
}
#endif
