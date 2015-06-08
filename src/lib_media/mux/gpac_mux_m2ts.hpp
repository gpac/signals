#pragma once

#include "lib_modules/core/module.hpp"

typedef struct __m2ts_mux_program GF_M2TS_Mux_Program;
typedef struct __m2ts_mux GF_M2TS_Mux;

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
