#include "stdafx.h"

#include "..\..\core\Exception.h"

#include "Model_Simbody.h"
#include "Body_Simbody.h"
#include "Muscle_Simbody.h"
#include "Simulation_Simbody.h"
#include "Joint_Simbody.h"
#include "tools.h"
#include "../../core/Log.h"

#include <OpenSim/OpenSim.h>

namespace scone
{
	namespace sim
	{
		/// Simbody controller that calls scone controllers
		class Model_Simbody::ControllerDispatcher : public OpenSim::Controller
		{
		public:
			ControllerDispatcher( Model_Simbody& model ) : m_Model( model ) { };
			virtual void computeControls( const SimTK::State& s, SimTK::Vector &controls ) const override
			{
				// reset actuator values
				std::vector< MuscleUP >& musvec = m_Model.GetMuscles();
				for ( auto iter = musvec.begin(); iter != musvec.end(); ++iter )
					(*iter)->ResetControlValue();

				// update all controllers
				for ( auto iter = m_Model.GetControllers().begin(); iter != m_Model.GetControllers().end(); ++iter )
					(*iter)->Update( s.getTime() );

				// inject actuator values into controls
				SimTK::Vector controlValue( 1 );
				for ( auto iter = musvec.begin(); iter != musvec.end(); ++iter )
				{
					controlValue[ 0 ] = (*iter)->GetControlValue();
					dynamic_cast< Muscle_Simbody* >( iter->get() )->GetOSMuscle().addInControls( controlValue, controls );
				}
			}

			virtual ControllerDispatcher* clone() const override {
				return new ControllerDispatcher( *this );
			}

			virtual const std::string& getConcreteClassName() const override {
				throw std::logic_error("The method or operation is not implemented.");
			}

		private:
			Model_Simbody& m_Model;
		};

		Model_Simbody::Model_Simbody( const String& filename ) :
		m_osModel( nullptr )
		{
			OpenSim::Object::setSerializeAllDefaults(true);
			m_osModel = std::unique_ptr< OpenSim::Model >( new OpenSim::Model( filename ) );

			// Create wrappers for actuators
			for ( int idx = 0; idx < m_osModel->getActuators().getSize(); ++idx )
			{
				OpenSim::Actuator& osAct = m_osModel->getActuators().get( idx );

				try // see if it's a muscle
				{
					OpenSim::Muscle& osMus = dynamic_cast< OpenSim::Muscle& >( osAct );
					m_Muscles.push_back( MuscleUP( new Muscle_Simbody( osMus ) ) );
				}
				catch ( std::bad_cast& )
				{
					SCONE_THROW( "Unsupported actuator type" );
				}
			}
			SCONE_LOG( "Muscles created: " << m_Muscles.size() );

			// Create wrappers for bodies
			for ( int idx = 0; idx < m_osModel->getBodySet().getSize(); ++idx )
				m_Bodies.push_back( BodyUP( new Body_Simbody( m_osModel->getBodySet().get( idx ) ) ) );
			SCONE_LOG( "Bodies created: " << m_Bodies.size() );

			// Create wrappers for joints
			for ( int idx = 0; idx < m_osModel->getJointSet().getSize(); ++idx )
				m_Joints.push_back( JointUP( new Joint_Simbody( m_osModel->getJointSet().get( idx ) ) ) );
			SCONE_LOG( "Joints created: " << m_Joints.size() );

			// setup hierarchy and create wrappers
			m_RootLink = CreateLinkHierarchy( m_osModel->getGroundBody() );

			// debug print
			SCONE_LOG( m_RootLink->ToString() );
		}

		Model_Simbody::~Model_Simbody()
		{
		}

		Vec3 Model_Simbody::GetComPos()
		{
			SimTK::Vec3 osVec = m_osModel->getMultibodySystem().getMatterSubsystem().calcSystemMassCenterLocationInGround( m_osModel->getWorkingState() );

			return ToVec3( osVec );
		}
		
		Vec3 Model_Simbody::GetComVel()
		{
			SimTK::Vec3 osVec = m_osModel->getMultibodySystem().getMatterSubsystem().calcSystemMassCenterVelocityInGround( m_osModel->getWorkingState() );
			
			return ToVec3( osVec );
		}

		Real Model_Simbody::GetMass()
		{
			return m_osModel->getMultibodySystem().getMatterSubsystem().calcSystemMass( m_osModel->getWorkingState() );
		}

		bool is_body_equal( BodyUP& body, OpenSim::Body& osBody )
		{
			return dynamic_cast< Body_Simbody& >( *body ).m_osBody == osBody;
		}

		LinkUP Model_Simbody::CreateLinkHierarchy( OpenSim::Body& osBody )
		{
			LinkUP link;

			// find the sim::Body
			auto itBody = std::find_if( m_Bodies.begin(), m_Bodies.end(), [&]( BodyUP& body ){ return dynamic_cast< Body_Simbody& >( *body ).m_osBody == osBody; } );
			SCONE_ASSERT( itBody != m_Bodies.end() );

			// find the sim::Joint (if any)
			if ( osBody.hasJoint() )
			{
				auto itJoint = std::find_if( m_Joints.begin(), m_Joints.end(), [&]( JointUP& body ){ return dynamic_cast< Joint_Simbody& >( *body ).m_osJoint == osBody.getJoint(); } );
				SCONE_ASSERT( itJoint != m_Joints.end() );
				link = LinkUP( new Link( **itBody, **itJoint ) );
			}
			else
			{
				link = LinkUP( new Link( **itBody ) );
			}

			// add children
			for ( auto iter = m_Bodies.begin(); iter != m_Bodies.end(); ++iter )
			{
				Body_Simbody& childBody = dynamic_cast< Body_Simbody& >( **iter );
				if ( childBody.m_osBody.hasJoint() && childBody.m_osBody.getJoint().getParentBody() == osBody )
				{
					// create child link
					link->children().push_back( CreateLinkHierarchy( childBody.m_osBody ) );
				}
			}

			return link;
		}
	}
}
