/*
 * The MIT License
 *
 * Copyright (c) 2009 The Broad Institute
 * Copyright (c) 2013 German Tischler
 * Copyright (c) 2013 Genome Research Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <libmaus/bambam/DuplicationMetrics.hpp>

std::ostream & operator<<(std::ostream & out, libmaus::bambam::DuplicationMetrics const & M)
{
	out << "unmapped\t" << M.unmapped << std::endl;
	out << "unpaired\t" << M.unpaired << std::endl;
	out << "readpairsexamined\t" << M.readpairsexamined << std::endl;
	out << "unpairedreadduplicates\t" << M.unpairedreadduplicates << std::endl;
	out << "readpairduplicates\t" << M.readpairduplicates << std::endl;
	out << "opticalduplicates\t" << M.opticalduplicates << std::endl;
	// out << "unmapped\t" << M.unmapped << std::endl;
	return out;
}
