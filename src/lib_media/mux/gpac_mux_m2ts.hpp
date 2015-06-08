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
		GPACMuxMPEG2TS(Bool real_time, Bool single_au_pes, u32 mux_rate, u32 psi_refresh_rate, u32 pcr_ms, s64 pcr_init_val);
		~GPACMuxMPEG2TS();
		void process() override;

	private:
		void declareStream(Data data);
		GF_M2TS_Mux *muxer;
		Bool real_time, single_au_pes = GF_FALSE;
		u32 mux_rate, psi_refresh_rate, pcr_ms = 100;
		s64 pcr_init_val = (s64) -1;


		//TODO
};

}
}
