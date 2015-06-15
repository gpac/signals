#pragma once

#include "lib_modules/core/module.hpp"
#include "lib_gpacpp/gpacpp.hpp"
#include <vector>

typedef struct __m2ts_mux_program GF_M2TS_Mux_Program;
typedef struct __m2ts_mux GF_M2TS_Mux;
typedef struct __elementary_stream_ifce GF_ESInterface;

namespace Modules {

class DataAVPacket;

namespace Mux {

typedef Signals::Queue<std::shared_ptr<const DataAVPacket>> DataInput;

class GPACMuxMPEG2TS : public ModuleDynI, public gpacpp::Init {
public:
	GPACMuxMPEG2TS(bool real_time, unsigned mux_rate, unsigned pcr_ms = 100, int64_t pcr_init_val = -1);
	~GPACMuxMPEG2TS();
	void process() override;

private:
	void declareStream(Data data);
	GF_Err fillInput(GF_ESInterface *esi, u32 ctrl_type, size_t inputIdx);
	static GF_Err staticFillInput(GF_ESInterface *esi, u32 ctrl_type, void *param);

	GF_M2TS_Mux *muxer;
	GF_M2TS_Mux_Program *program;
	std::vector<std::unique_ptr<DataInput>> inputData;
};

}
}
