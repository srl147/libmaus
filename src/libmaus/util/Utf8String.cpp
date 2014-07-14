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
#include <libmaus/util/Utf8String.hpp>

uint64_t libmaus::util::Utf8String::computeOctetLength(std::wistream & stream, uint64_t const len)
{
	::libmaus::util::CountPutObject CPO;
	for ( uint64_t i = 0; i < len; ++i )
	{
		std::wistream::int_type const w = stream.get();
		assert ( w != std::wistream::traits_type::eof() );
		::libmaus::util::UTF8::encodeUTF8(w,CPO);
	}
	
	return CPO.c;
}

libmaus::util::Utf8String::shared_ptr_type libmaus::util::Utf8String::constructRaw(
	std::string const & filename, 
	uint64_t const offset, 
	uint64_t const blength
)
{
	return shared_ptr_type(new this_type(filename, offset, blength));
}

void libmaus::util::Utf8String::setup()
{
	uint64_t const bitalign = 6*64;
	
	::libmaus::rank::ImpCacheLineRank::unique_ptr_type tI(new ::libmaus::rank::ImpCacheLineRank(((A.size()+(bitalign-1))/bitalign)*bitalign));
	I = UNIQUE_PTR_MOVE(tI);

	::libmaus::rank::ImpCacheLineRank::WriteContext WC = I->getWriteContext();
	for ( uint64_t i = 0; i < A.size(); ++i )
		if ( (!(A[i] & 0x80)) || ((A[i] & 0xc0) != 0x80) )
			WC.writeBit(1);
		else
			WC.writeBit(0);
			
	for ( uint64_t i = A.size(); i % bitalign; ++i )
		WC.writeBit(0);
			
	WC.flush();
	
	::libmaus::select::ImpCacheLineSelectSupport::unique_ptr_type tS(new ::libmaus::select::ImpCacheLineSelectSupport(*I,8));
	S = UNIQUE_PTR_MOVE(tS);
	
	#if 0
	std::cerr << "A.size()=" << A.size() << std::endl;
	std::cerr << "I.byteSize()=" << I->byteSize() << std::endl;
	#endif
}

libmaus::util::Utf8String::Utf8String(
	std::string const & filename, 
	uint64_t offset, 
	uint64_t blength)
{	
	::libmaus::aio::CheckedInputStream CIS(filename);
	uint64_t const fs = ::libmaus::util::GetFileSize::getFileSize(CIS);
	offset = std::min(offset,fs);
	blength = std::min(blength,fs-offset);

	CIS.seekg(offset);
	A = ::libmaus::autoarray::AutoArray<uint8_t>(blength,false);
	CIS.read(reinterpret_cast<char *>(A.begin()),blength);
	
	setup();
}

libmaus::util::Utf8String::Utf8String(std::istream & CIS, uint64_t blength)
: A(blength,false)
{
	CIS.read(reinterpret_cast<char *>(A.begin()),blength);				
	setup();
}

libmaus::util::Utf8String::Utf8String(std::wistream & CIS, uint64_t const octetlength, uint64_t const symlength)
: A(octetlength,false)
{	
	::libmaus::util::PutObject<uint8_t *> P(A.begin());
	for ( uint64_t i = 0; i < symlength; ++i )
	{
		std::wistream::int_type const w = CIS.get();
		assert ( w != std::wistream::traits_type::eof() );
		::libmaus::util::UTF8::encodeUTF8(w,P);
	}

	setup();
}

::libmaus::autoarray::AutoArray<uint64_t> libmaus::util::Utf8String::computePartStarts(
	::libmaus::autoarray::AutoArray<uint8_t> const & A, uint64_t const tnumparts
)
{
	uint64_t const fs = A.size();
	uint64_t const tpartsize = (fs + tnumparts-1)/tnumparts;
	uint64_t const numparts = (fs + tpartsize-1)/tpartsize;
	::libmaus::autoarray::AutoArray<uint64_t> partstarts(numparts+1,false);

	for ( int64_t i = 0; i < static_cast<int64_t>(numparts); ++i )
	{
		uint64_t j = std::min(i*tpartsize,fs);
		::libmaus::util::GetObject<uint8_t const *> G(A.begin()+j);
		
		while ( j != fs && ((G.get() & 0xc0) == 0x80) )
			++j;
			
		partstarts[i] = j;					
	}
	
	partstarts[numparts] = fs;

	return partstarts;
}

::libmaus::autoarray::AutoArray<uint64_t> libmaus::util::Utf8String::computePartStarts(
	std::string const & fn, uint64_t const tnumparts
)
{
	uint64_t const fs = ::libmaus::util::GetFileSize::getFileSize(fn);
	uint64_t const tpartsize = (fs + tnumparts-1)/tnumparts;
	uint64_t const numparts = (fs + tpartsize-1)/tpartsize;
	::libmaus::autoarray::AutoArray<uint64_t> partstarts(numparts+1,false);

	for ( int64_t i = 0; i < static_cast<int64_t>(numparts); ++i )
	{
		uint64_t j = std::min(i*tpartsize,fs);
		::libmaus::aio::CheckedInputStream G(fn);
		G.seekg(j);
		
		while ( j != fs && ((G.get() & 0xc0) == 0x80) )
			++j;
			
		partstarts[i] = j;					
	}
	
	partstarts[numparts] = fs;

	return partstarts;
}

template<typename _get_object_type>
struct HistogramThread : public ::libmaus::parallel::PosixThread
{
	typedef _get_object_type get_object_type;
	typedef HistogramThread<get_object_type> this_type;
	typedef typename ::libmaus::util::unique_ptr<this_type>::type unique_ptr_type;

	get_object_type & G;
	uint64_t const tcodelen;
	::libmaus::parallel::PosixMutex & mutex;
	::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t> & ESCH;
	uint64_t const t;

	HistogramThread(
		get_object_type & rG,
		uint64_t const rtcodelen,
		::libmaus::parallel::PosixMutex & rmutex,
		::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t> & rESCH,
		uint64_t const rt
	)
	: G(rG), tcodelen(rtcodelen), mutex(rmutex), ESCH(rESCH), t(rt)
	{
		startStack(32*1024);
	}
	
	virtual void * run()
	{
		try
		{
			::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t> LESCH(8u);
			::libmaus::autoarray::AutoArray<uint64_t> low(256);
			uint64_t codelen = 0;
			
			while ( codelen != tcodelen )
			{
				uint32_t const code = ::libmaus::util::UTF8::decodeUTF8(G,codelen);
				
				if ( code < low.size() )
					low[code]++;
				else
				{
					if ( LESCH.loadFactor() > 0.8 )
						LESCH.extendInternal();
					LESCH.insert(code,1);
				}
			}
		
			// add low key counts to hash map		
			for ( uint64_t i = 0; i < low.size(); ++i )
				if ( low[i] )
				{
					if ( LESCH.loadFactor() > 0.8 )
						LESCH.extendInternal();
					LESCH.insert(i,low[i]);
				}
			
			// add to global hash map
			mutex.lock();	
			for ( ::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t>::key_type const * ita =
				LESCH.begin(); ita != LESCH.end(); ++ita )
				if ( *ita != ::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t>::unused() )
				{
					if ( ESCH.loadFactor() > 0.8 )
						ESCH.extendInternal();
					ESCH.insert(*ita,LESCH.getCount(*ita));
				}
			mutex.unlock();
		
		}
		catch(std::exception const & ex)
		{
			std::cerr << ex.what() << std::endl;
		}
		return 0;
	}	
};

::libmaus::autoarray::AutoArray< std::pair<int64_t,uint64_t> > libmaus::util::Utf8String::getHistogramAsArray(::libmaus::autoarray::AutoArray<uint8_t> const & A)
{			
	#if defined(_OPENMP)
	uint64_t const numthreads = omp_get_max_threads();
	#else
	uint64_t const numthreads = 1;
	#endif
	
	::libmaus::autoarray::AutoArray<uint64_t> const partstarts = computePartStarts(A,numthreads);
	uint64_t const numparts = partstarts.size()-1;

	::libmaus::parallel::OMPLock lock;
	::libmaus::parallel::PosixMutex mutex;
	::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t> ESCH(8u);
	
	typedef HistogramThread< ::libmaus::util::GetObject<uint8_t const *> > thread_type;
	typedef thread_type::unique_ptr_type thread_ptr_type;
	::libmaus::autoarray::AutoArray< ::libmaus::util::GetObject<uint8_t const *>::unique_ptr_type > getters(numparts);
	::libmaus::autoarray::AutoArray<thread_ptr_type> threads(numparts);

	for ( uint64_t i = 0; i < numparts; ++i )
	{
		::libmaus::util::GetObject<uint8_t const *>::unique_ptr_type tgettersi(
                                new ::libmaus::util::GetObject<uint8_t const *>(A.begin()+partstarts[i])
                        );
		getters[i] = UNIQUE_PTR_MOVE(tgettersi);
		thread_ptr_type tthreadsi(new thread_type(*getters[i],
                                partstarts[i+1]-partstarts[i],mutex,ESCH,i));
		threads[i] = UNIQUE_PTR_MOVE(tthreadsi);
	}
	for ( uint64_t i = 0; i < numparts; ++i )
	{
		threads[i]->join();
		threads[i].reset();
	}	

	::libmaus::autoarray::AutoArray< std::pair<int64_t,uint64_t> > R(ESCH.size(),false);
	uint64_t p = 0;
	for ( ::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t>::key_type const * ita =
		ESCH.begin(); ita != ESCH.end(); ++ita )
		if ( *ita != ::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t>::unused() )
			R [ p++ ] = std::pair<int64_t,uint64_t>(*ita,ESCH.getCount(*ita));
			
	std::sort(R.begin(),R.end());
	
	return R;
}

::libmaus::autoarray::AutoArray< std::pair<int64_t,uint64_t> > libmaus::util::Utf8String::getHistogramAsArray(std::string const & fn)
{			
	#if defined(_OPENMP)
	uint64_t const numthreads = omp_get_max_threads();
	#else
	uint64_t const numthreads = 1;
	#endif
	
	::libmaus::autoarray::AutoArray<uint64_t> const partstarts = computePartStarts(fn,numthreads);
	uint64_t const numparts = partstarts.size()-1;

	::libmaus::parallel::OMPLock lock;
	::libmaus::parallel::PosixMutex mutex;
	::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t> ESCH(8u);
	
	typedef HistogramThread< ::libmaus::aio::CheckedInputStream > thread_type;
	typedef thread_type::unique_ptr_type thread_ptr_type;
	::libmaus::autoarray::AutoArray< ::libmaus::aio::CheckedInputStream::unique_ptr_type > getters(numparts);
	::libmaus::autoarray::AutoArray<thread_ptr_type> threads(numparts);

	for ( uint64_t i = 0; i < numparts; ++i )
	{
		::libmaus::aio::CheckedInputStream::unique_ptr_type tgettersi(
                                new ::libmaus::aio::CheckedInputStream(fn)
                        );
		getters[i] = UNIQUE_PTR_MOVE(tgettersi);
		getters[i]->setBufferSize(16*1024);
		getters[i]->seekg(partstarts[i]);
		thread_ptr_type tthreadsi(new thread_type(*getters[i],
                                partstarts[i+1]-partstarts[i],mutex,ESCH,i));
		threads[i] = UNIQUE_PTR_MOVE(tthreadsi);
	}
	for ( uint64_t i = 0; i < numparts; ++i )
	{
		threads[i]->join();
		threads[i].reset();
	}	

	::libmaus::autoarray::AutoArray< std::pair<int64_t,uint64_t> > R(ESCH.size(),false);
	uint64_t p = 0;
	for ( ::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t>::key_type const * ita =
		ESCH.begin(); ita != ESCH.end(); ++ita )
		if ( *ita != ::libmaus::util::ExtendingSimpleCountingHash<uint64_t,uint64_t>::unused() )
			R [ p++ ] = std::pair<int64_t,uint64_t>(*ita,ESCH.getCount(*ita));
			
	std::sort(R.begin(),R.end());
	
	return R;
}

::libmaus::util::Histogram::unique_ptr_type libmaus::util::Utf8String::getHistogram(::libmaus::autoarray::AutoArray<uint8_t> const & A)
{
	#if defined(_OPENMP)
	uint64_t const numthreads = omp_get_max_threads();
	#else
	uint64_t const numthreads = 1;
	#endif
	
	::libmaus::autoarray::AutoArray<uint64_t> const partstarts = computePartStarts(A,numthreads);
	uint64_t const numparts = partstarts.size()-1;
	
	::libmaus::util::Histogram::unique_ptr_type hist(new ::libmaus::util::Histogram);
	::libmaus::parallel::OMPLock lock;
	
	#if defined(_OPENMP)
	#pragma omp parallel for
	#endif
	for ( int64_t t = 0; t < static_cast<int64_t>(numparts); ++t )
	{
		::libmaus::util::Histogram::unique_ptr_type lhist(new ::libmaus::util::Histogram);
	
		uint64_t codelen = 0;
		uint64_t const tcodelen = partstarts[t+1]-partstarts[t];
		::libmaus::util::GetObject<uint8_t const *> G(A.begin()+partstarts[t]);
		
		while ( codelen != tcodelen )
			(*lhist)(::libmaus::util::UTF8::decodeUTF8(G,codelen));
			
		lock.lock();
		hist->merge(*lhist);
		lock.unlock();
	}
	
	return UNIQUE_PTR_MOVE(hist);
}

::libmaus::util::Histogram::unique_ptr_type libmaus::util::Utf8String::getHistogram() const
{
	::libmaus::util::Histogram::unique_ptr_type hist(new ::libmaus::util::Histogram);
	
	for ( uint64_t i = 0; i < A.size(); ++i )
		if ( (A[i] & 0xc0) != 0x80 )
		{
			::libmaus::util::GetObject<uint8_t const *> G(A.begin()+i);
			wchar_t const v = ::libmaus::util::UTF8::decodeUTF8(G);
			(*hist)(v);
		}
		
	return UNIQUE_PTR_MOVE(hist);
}

std::map<int64_t,uint64_t> libmaus::util::Utf8String::getHistogramAsMap() const
{
	::libmaus::util::Histogram::unique_ptr_type hist(getHistogram());
	return hist->getByType<int64_t>();
}			

std::map<int64_t,uint64_t> libmaus::util::Utf8String::getHistogramAsMap(::libmaus::autoarray::AutoArray<uint8_t> const & A)
{
	::libmaus::util::Histogram::unique_ptr_type hist(getHistogram(A));
	return hist->getByType<int64_t>();
}			

::libmaus::autoarray::AutoArray<libmaus::util::Utf8String::saidx_t,::libmaus::autoarray::alloc_type_c> 
	libmaus::util::Utf8String::computeSuffixArray32(bool const parallel) const
{
	if ( A.size() > static_cast<uint64_t>(::std::numeric_limits<saidx_t>::max()) )
	{
		::libmaus::exception::LibMausException se;
		se.getStream() << "computeSuffixArray32: input is too large for data type." << std::endl;
		se.finish();
		throw se;
	}
	
	::libmaus::autoarray::AutoArray<saidx_t,::libmaus::autoarray::alloc_type_c> SA(A.size());
	if ( parallel )
		sort_type_parallel::divsufsort ( A.begin() , SA.begin() , A.size() );
	else
		sort_type_serial::divsufsort ( A.begin() , SA.begin() , A.size() );
	
	uint64_t p = 0;
	for ( uint64_t i = 0; i < SA.size(); ++i )
		if ( (A[SA[i]] & 0xc0) != 0x80 )
			SA[p++] = SA[i];
	SA.resize(p);
	
	for ( uint64_t i = 0; i < SA.size(); ++i )
	{
		assert ( (*I)[SA[i]] );
		SA[i] = I->rank1(SA[i])-1;
	}
	
	return SA;
}
