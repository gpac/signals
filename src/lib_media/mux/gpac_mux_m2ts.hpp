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
		GPACMuxMPEG2TS(bool real_time, unsigned mux_rate, unsigned pcr_ms, int64_t pcr_init_val);
		~GPACMuxMPEG2TS();
		void process() override;

	private:

		void declareStream(Data data);
		GF_M2TS_Mux *muxer;
		Bool real_time, single_au_pes = GF_FALSE;
		u32 mux_rate, psi_refresh_rate, pcr_ms = 100;
		s64 pcr_init_val = (s64) -1;
		GF_M2TS_Mux_Program *program;



		//TODO
};

}
}
