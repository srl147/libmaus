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
#if ! defined(LIBMAUS_QUANTISATION_KMLOCALINTERFACE_HPP)
#define LIBMAUS_QUANTISATION_KMLOCALINTERFACE_HPP

#include <libmaus/types/types.hpp>

extern "C" {
	typedef void * (*libmaus_quantisation_kmeansWrapperByTypeC_type)(unsigned int const, void *, uint64_t const, uint64_t const, uint64_t const, unsigned int const);
	void * libmaus_quantisation_kmeansWrapperByTypeC(unsigned int const type, void * pvita, uint64_t const rn, uint64_t const k, uint64_t const runs, unsigned int const debug);
	// libmaus_quantisation_kmeansWrapperByTypeC_type libmaus_quantisation_kmeansWrapperByTypeC;
}
#endif
