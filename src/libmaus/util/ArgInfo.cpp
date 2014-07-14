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
		
bool libmaus::util::ArgInfo::helpRequested() const
{
	return 
		(argmap.size() == 0) &&
		(restargs.size()==1) &&
		(
			restargs[0] == "-h"
			||
			restargs[0] == "--help"
		);
}

std::string libmaus::util::ArgInfo::getProgFileName(std::string const & progname)
{
	int64_t l = -1;
	
	for ( uint64_t i = 0; i < progname.size(); ++i )
		if ( progname[i] == '/' )
			l = i;
	
	if ( l < 0 )
		return progname;
	else
		return progname.substr(l+1);
}

std::string libmaus::util::ArgInfo::getDefaultTmpFileName(std::string const & progname)
{
	std::ostringstream ostr;
	ostr << getProgFileName(progname) 
		<< "_" << ::libmaus::network::GetHostName::getHostName()
		<< "_" << getpid()
		<< "_" << time(0);
	return ostr.str();
}

std::string libmaus::util::ArgInfo::getDefaultTmpFileName() const
{
	return getDefaultTmpFileName(progname);
}

std::string libmaus::util::ArgInfo::getCurDir()
{
	size_t len = PATH_MAX;
	char * p = 0;
	::libmaus::autoarray::AutoArray<char> Acurdir;
	
	do
	{
		Acurdir = ::libmaus::autoarray::AutoArray<char>(len+1);
		#if defined(_WIN32)
		p = _getcwd(Acurdir.get(), Acurdir.size()-1);
		#else
		p = getcwd ( Acurdir.get(), Acurdir.size()-1 );
		#endif
		len *= 2;
	}
	while ( (!p) && errno==ERANGE );
	
	if ( p )
		return std::string(Acurdir.get());
	else
	{
		throw std::runtime_error("Cannot get current directory.");
	}
}

std::string libmaus::util::ArgInfo::getDirName(std::string absprogname)
{
	::libmaus::autoarray::AutoArray<char> Aprogname(absprogname.size()+1);
	std::copy ( absprogname.begin(), absprogname.end(), Aprogname.get() );
	char * pdirname = dirname ( Aprogname.get() );
	return std::string ( pdirname );
}

std::string libmaus::util::ArgInfo::getAbsProgName() const
{
	if ( progname.size() > 0 && progname[0] == '/' )
		return progname;
	else
		return getCurDir() + "/" + progname;
}

std::string libmaus::util::ArgInfo::getProgDirName() const
{
	std::string absprog = getAbsProgName();
	return absprog.substr(0,absprog.find_last_of("/"));
}

void libmaus::util::ArgInfo::init(std::vector<std::string> const args)
{
	uint64_t i = 0;
	progname = args[i++];

	for ( ; (i < args.size()) && std::string(args[i]).find('=') != std::string::npos ; ++i )
	{
		std::pair<std::string,std::string> valpair = ::libmaus::util::stringFunctions::tokenizePair<std::string>(args[i],std::string("="));
		argmap[valpair.first] = valpair.second;
		argmultimap.insert(std::pair<std::string,std::string>(valpair.first,valpair.second));
	}
	
	for ( ; i < args.size(); ++i )
		restargs.push_back(args[i]);
}

std::string libmaus::util::ArgInfo::reconstructCommandLine(int argc, char const * argv[])
{
	// "reconstruct" command line
	std::string cl;
	for ( int i = 0; i < argc; ++i )
	{
		cl += argv[i];
		if ( i+1 < argc )
			cl += " ";
	}			
	
	return cl;
}

libmaus::util::ArgInfo::ArgInfo(int argc, char * argv[])
: commandline(reconstructCommandLine(argc,const_cast<char const **>(argv)))
{
	init(argsToVector(argc,argv));
}
libmaus::util::ArgInfo::ArgInfo(int argc, char const * argv[])
: commandline(reconstructCommandLine(argc,argv))
{
	init(argsToVector(argc,argv));
}
libmaus::util::ArgInfo::ArgInfo(std::vector<std::string> const & args)
{
	init(args);
}

libmaus::util::ArgInfo::ArgInfo(
	std::string const & rprogname,
	keymap_type const & keymap,
	std::vector<std::string> const & rrestargs)
{
	std::vector<std::string> V;
	V.push_back(rprogname);
	for ( std::map<std::string,std::string>::const_iterator ita = keymap.begin(); ita != keymap.end(); ++ita )
		V.push_back ( ita->first + "=" + ita->second );
	for ( uint64_t i = 0; i < rrestargs.size(); ++i )
		V.push_back ( rrestargs[i] );
	init(V);
}

bool libmaus::util::ArgInfo::hasArg(std::string const & key) const
{
	return argmap.find(key) != argmap.end();
}

std::string libmaus::util::ArgInfo::stringRestArg(uint64_t const i) const
{
	if ( i < restargs.size() )
	{
		return restargs[i];
	}
	else
	{
		::libmaus::exception::LibMausException se;
		se.getStream() << "Argument index out of range in stringRestArg()";
		se.finish();
		throw se;
	}

}

std::ostream & operator<<(std::ostream & out, libmaus::util::ArgInfo const & arginfo)
{
	out << "ArgInfo(progname=" << arginfo.progname << ",{";
	for ( 
		std::map<std::string,std::string>::const_iterator ita = arginfo.argmap.begin();
		ita != arginfo.argmap.end(); )
	{
		out << ita->first << "=" << ita->second;
		++ita;
		if ( ita != arginfo.argmap.end() )
			out << ";";
	}
	out << "},[";
	for ( uint64_t i = 0; i < arginfo.restargs.size(); ++i )
	{
		out << arginfo.restargs[i];
		if ( i+1 < arginfo.restargs.size() )
			out << ",";
	}
	out << "])";
	return out;
}

