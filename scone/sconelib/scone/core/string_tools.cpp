#include "string_tools.h"

#include <time.h>

#ifdef WIN32
#include <shlwapi.h>
#pragma comment( lib, "shlwapi.lib" )
#pragma warning( disable: 4996 ) // no push/pop because it's a cpp file
#else
#include <fnmatch.h>
#endif

#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <cstdarg>

using std::cout;
using std::endl;

namespace scone
{
	std::string GetStringF( const char* format, ... )
	{
		va_list args;
		va_start( args, format );

		char buf[8192];
		vsnprintf( buf, sizeof( buf ), format, args );

		va_end( args );

		return std::string( buf );
	}

	std::string GetDateTimeAsString()
	{
		auto now = boost::posix_time::second_clock::local_time();
		auto month = static_cast<int>( now.date().month() );
		auto day = static_cast<int>( now.date().day() );
		auto hours = static_cast<int>( now.time_of_day().hours() );
		auto mins = static_cast<int>( now.time_of_day().minutes() );

		return stringf( "%02d%02d.%02d%02d", month, day, hours, mins );
	}

	std::string GetDateTimeExactAsString()
	{
		auto now = boost::posix_time::microsec_clock::local_time();
		auto month = static_cast<int>( now.date().month() );
		auto day = static_cast<int>( now.date().day() );
		auto hours = static_cast<int>( now.time_of_day().hours() );
		auto mins = static_cast<int>( now.time_of_day().minutes() );
		auto secs = static_cast<int>( now.time_of_day().seconds() );
        auto frac_secs = static_cast<int>( now.time_of_day().fractional_seconds() );

		return stringf( "%02d%02d.%02d%02d%02d.%06d", month, day, hours, mins, secs, frac_secs );
	}

	// TODO: Could use regex to remove platform dependencies
	// Currently assumes one delimiter char. Can extend with boost if needed
	bool SCONE_API MatchesPattern( const String& str, const String& pattern, bool multiple_patterns, char delim )
	{
#ifdef WIN32
		return PathMatchSpecEx( str.c_str(), pattern.c_str(), multiple_patterns ? PMSF_MULTIPLE : PMSF_NORMAL ) == S_OK;
#else
		std::vector<std::string> tokens;
		std::stringstream ss(pattern);
		std::string token;
		while ( std::getline(ss, token, delim) ) {
			tokens.push_back(token);
		}
		for ( auto thisPattern : tokens ) {
			bool isMatch = fnmatch(thisPattern.c_str(), str.c_str(), FNM_NOESCAPE) == 0;
			if ( isMatch ) return isMatch;
		}
		return false;	
#endif
	}
}
