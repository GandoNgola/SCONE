#include "stdafx.h"
#include "opt.h"
#include "../core/PropNodeFactory.h"
#include "CmaOptimizer.h"

namespace scone
{
	namespace opt
	{
		void RegisterFactoryTypes()
		{
			CmaOptimizer::RegisterFactory();
		}

		OptimizerUP CreateOptimizerFromXml( const String& xml_file, const String& key )
		{
			RegisterFactoryTypes();

			PropNode p = ReadXmlFile( xml_file );
			OptimizerUP o = CreateFromPropNode< Optimizer >( p.GetChild( "Optimizer" ) );

			// report unused parameters
			p.ToStream( std::cout, "Unused parameter ", true );

			return o;
		}
	}
}
