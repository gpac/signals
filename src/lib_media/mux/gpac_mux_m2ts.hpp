#pragma once

extern "C" 
{
#include <gpac/mpegts.h>
}

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Mux {

class GPACMuxMPEG2TS : public ModuleDynI 
{
	public:
		GPACMuxMPEG2TS(bool real_time, unsigned mux_rate, unsigned pcr_ms = 100, int64_t pcr_init_val = -1);
		~GPACMuxMPEG2TS();
		void process() override;

	private:

		void declareStream(Data data);
		GF_M2TS_Mux *muxer;
		GF_M2TS_Mux_Program *program;



		//TODO
};

}
}
