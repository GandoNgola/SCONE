#pragma once

#include "ChannelSensor.h"
#include "../core/Delayer.h"
#include "../core/Storage.h"

namespace scone
{
	namespace sim
	{
		class Model;

		class SCONE_SIM_API ChannelSensorDelayAdapter : public ChannelSensor
		{
		public:
			ChannelSensorDelayAdapter( Model& model, Storage< Real >& storage, ChannelSensor& source, TimeInSeconds default_delay );
			virtual ~ChannelSensorDelayAdapter();

			virtual Real GetSensorValue( size_t idx ) const override;
			Real GetSensorValue( size_t idx, TimeInSeconds delay ) const;
			virtual const String& GetName() const override;
			virtual const StringIndexMap& GetSensorNames() const override;
		
		private:
			friend Model;
			void UpdateStorage();

			Index m_ChannelOfs;
			TimeInSeconds m_Delay;
			Model& m_Model;
			Storage< Real >& m_Storage;
			ChannelSensor& m_Source;
			int m_PreviousUpdateStep;
		};
	}
}
