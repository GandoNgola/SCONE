#include "system_tools.h"

#include <fstream>

#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "flut/system_tools.hpp"

#ifdef _MSC_VER
#include <shlobj.h>
#endif

namespace scone
{
	boost::mutex g_SystemMutex;
	PropNode g_GlobalSettings;
	String g_Version;

	SCONE_API String GetApplicationFolder()
	{
#ifdef _MSC_VER
		char buf[ 1024 ];
		GetModuleFileName( 0, buf, sizeof( buf ) );

		boost::filesystem::path folder( buf );
		return folder.parent_path().string();
#else
        return "";
#endif
	}

	const PropNode& GetSconeSettings()
	{
		boost::lock_guard< boost::mutex > lock( g_SystemMutex );

		// lazy initialization
		if ( g_GlobalSettings.IsEmpty() )
        	g_GlobalSettings.FromIniFile( flut::get_config_folder() + "/Scone/settings.ini" );

		return g_GlobalSettings;
	}

	SCONE_API String GetSconeFolder( const String& folder )
	{
		return GetSconeSettings().GetStr( "folders." + folder ) + "/";
	}

	SCONE_API String GetApplicationVersion()
	{
		if ( g_Version.empty() )
		{
			boost::filesystem::path versionpath( GetApplicationFolder() );

			// look for .version file, up to three levels from application folder
			for ( int level = 0; level <= 3; ++level )
			{
				if ( boost::filesystem::exists( versionpath / ".version" ) )
				{
					// .version file found, read its contents
					std::ifstream ifstr( ( versionpath / ".version" ).string() );
					ifstr >> g_Version;
					break;

				}
				else versionpath = versionpath / "..";
			}

			if ( g_Version.empty() ) 
				g_Version = "UNKNOWN_VERSION"; // reading has failed, version unknown
		}

		return g_Version;
	}
}
