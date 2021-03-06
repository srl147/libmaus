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

#include <libmaus/util/MemUsage.hpp>

libmaus::util::MemUsage::MemUsage()
{
	std::map<std::string,std::string> const M = getProcSelfStatusMap();
	VmPeak = getMemParam(M,"VmPeak");
	VmSize = getMemParam(M,"VmSize");
	VmLck = getMemParam(M,"VmLck");
	VmHWM = getMemParam(M,"VmHWM");
	VmRSS = getMemParam(M,"VmRSS");
	VmData = getMemParam(M,"VmData");
	VmStk = getMemParam(M,"VmStk");
	VmExe = getMemParam(M,"VmExe");
	VmLib = getMemParam(M,"VmLib");
	VmPTE = getMemParam(M,"VmPTE");
}

libmaus::util::MemUsage::MemUsage(MemUsage const & o)
: VmPeak(o.VmPeak),
  VmSize(o.VmSize),
  VmLck(o.VmLck),
  VmHWM(o.VmHWM),
  VmRSS(o.VmRSS),
  VmData(o.VmData),
  VmStk(o.VmStk),
  VmExe(o.VmExe),
  VmLib(o.VmLib),
  VmPTE(o.VmPTE)
{	
}

libmaus::util::MemUsage & libmaus::util::MemUsage::operator=(libmaus::util::MemUsage const & o)
{
	VmPeak = o.VmPeak;
	VmSize = o.VmSize;
	VmLck = o.VmLck;
	VmHWM = o.VmHWM;
	VmRSS = o.VmRSS;
	VmData = o.VmData;
	VmStk = o.VmStk;
	VmExe = o.VmExe;
	VmLib = o.VmLib;
	VmPTE = o.VmPTE;
	return *this;
}

bool libmaus::util::MemUsage::operator==(libmaus::util::MemUsage const & o) const
{
	return
		VmPeak == o.VmPeak &&
		VmSize == o.VmSize &&
		VmLck  == o.VmLck &&
		VmHWM  == o.VmHWM &&
		VmRSS  == o.VmRSS &&
		VmData == o.VmData &&
		VmStk  == o.VmStk &&
		VmExe  == o.VmExe &&
		VmLib  == o.VmLib &&
		VmPTE  == o.VmPTE;
}

bool libmaus::util::MemUsage::operator!=(libmaus::util::MemUsage const & o) const
{
	return !((*this)==o);
}

uint64_t libmaus::util::MemUsage::getMemParam(std::map<std::string,std::string> const & M, std::string const key)
{
	if ( M.find(key) != M.end() )
	{
		std::string const & V = M.find(key)->second;
		return parseMemPair(V);
	}
	
	return 0;
}

uint64_t libmaus::util::MemUsage::parseMemPair(std::string const & V)
{
	std::pair<std::string,std::string> P;
	
	if ( tokenise(V,P) )
	{
		if ( P.second == "kB" )
		{
			std::istringstream istr(P.first);
			uint64_t v;
			istr >> v;
			if ( istr )
				return v * 1024;
		}
	}
	
	return 0;
}

bool libmaus::util::MemUsage::tokenise(std::string line, std::pair<std::string,std::string> & P)
{
	bool containsspace = false;
	uint64_t spacepos = 0;
	for ( uint64_t i = 0; (!containsspace) && i < line.size(); ++i )
		if ( isspace(line[i]) )
		{
			containsspace = true;
			spacepos = i;
		}

	if ( containsspace )
	{
		std::string key = line.substr(0,spacepos);
		
		if ( key.size() && key[key.size()-1] == ':' )
			key = key.substr(0,key.size()-1);
		
		uint64_t nextnonspace = spacepos;
		
		while ( nextnonspace < line.size() && isspace(line[nextnonspace]) )
			nextnonspace++;
			
		if ( nextnonspace < line.size() )
		{
			std::string const value = line.substr(nextnonspace);
			P = std::pair<std::string,std::string>(key,value);
			return true;
		}
	}
	
	return false;	
}

std::map<std::string,std::string> libmaus::util::MemUsage::getProcSelfStatusMap()
{
	std::map<std::string,std::string> M;
	
	std::ifstream istr("/proc/self/status");
	if ( ! istr.is_open() )
		return M;
	
	while ( istr )
	{
		std::string line;
		std::getline(istr,line);
		
		if ( istr && line.size() )
		{
			std::pair<std::string,std::string> P;
			if ( tokenise(line,P) )
			{
				M [ P.first ] = P.second;
				// std::cerr << "(" << P.first << "->" << P.second << ")" << std::endl;
			}
		}
	}
	
	return M;
}

std::ostream & libmaus::util::operator<<(std::ostream & out, libmaus::util::MemUsage const & M)
{
	out << "MemUsage("
		<< "size=" << static_cast<double>(M.VmSize)/(1024.0*1024.0) << ","
		<< "rss=" << static_cast<double>(M.VmRSS)/(1024.0*1024.0) << ","
		<< "peak=" << static_cast<double>(M.VmPeak)/(1024.0*1024.0) << ")";
	return out;
}
