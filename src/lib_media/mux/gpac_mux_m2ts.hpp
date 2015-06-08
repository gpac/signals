#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Mux {

class GPACMuxMPEG2TS : public ModuleDynI 
{
	public:
		GPACMuxMPEG2TS(u32, u32, Bool);
		~GPACMuxMPEG2TS();
		void process() override;

	private:
		void declareStream(Data data);
		u32 mux_rate, psi_refresh_rate;
		Bool real_time;

		//TODO
};

}
}
