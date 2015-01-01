#pragma once

#include "../core/core.h"
#include "sim.h"
#include "../core/Vec3.h"
#include "Controller.h"
#include "Link.h"
#include "../opt/ParamSet.h"

namespace scone
{
	namespace sim
	{
		class SCONE_SIM_API Model : public Propertyable, public opt::Parameterizable
		{
		public:
			Model();
			virtual ~Model();

			virtual Vec3 GetComPos() = 0;
			virtual Vec3 GetComVel() = 0;
			virtual Real GetMass() = 0;
			virtual Vec3 GetGravity() = 0;
			virtual bool HasGroundContact() = 0;

			/// muscle access
			size_t GetMuscleCount() { return m_Muscles.size(); }
			Muscle& GetMuscle( size_t idx ) { return *m_Muscles[ idx ]; }
			std::vector< MuscleUP >& GetMuscles() { return m_Muscles; }

			/// body access
			size_t GetBodyCount() { return m_Bodies.size(); }
			Body& GetBody( size_t idx ) { return *m_Bodies[ idx ]; }
			std::vector< BodyUP >& GetBodies() { return m_Bodies; }

			/// joint access
			std::vector< JointUP >& GetJoints() { return m_Joints; }

			/// link access
			Link& GetRootLink() { return *m_RootLink; }

			/// controller access
			std::vector< ControllerUP >& GetControllers() { return m_Controllers; }

			// initialization
			virtual void ProcessProperties( const PropNode& props ) override;
			virtual void ProcessParameters( opt::ParamSet& par ) override;

			/// Simulate model
			virtual void AdvanceSimulationTo( double time ) = 0;
			virtual void RequestTermination() { m_ShouldTerminate = true; }
			virtual bool ShouldTerminate() { return m_ShouldTerminate; }
			virtual void WriteStateHistory( const String& file ) = 0;

		protected:
			std::unique_ptr< Link > m_RootLink;
			std::vector< MuscleUP > m_Muscles;
			std::vector< BodyUP > m_Bodies;
			std::vector< JointUP > m_Joints;
			std::vector< ControllerUP > m_Controllers;
			bool m_ShouldTerminate;
		};
	}
}
