#include "gpac_mux_m2ts.hpp"
#include "../common/libav.hpp"
#include <cassert>

extern "C"  {
#include <gpac/mpegts.h>
#include <libavformat/avformat.h>
}

namespace Modules  {
namespace Mux  {

const Bool single_au_pes = GF_FALSE;
const int pcrOffset = 0;
const int curPid = 100;

GPACMuxMPEG2TS::GPACMuxMPEG2TS(bool real_time, unsigned mux_rate, unsigned pcr_ms, int64_t pcr_init_val) {
	addOutput(new OutputDataDefault<DataAVPacket>(nullptr));

	muxer = gf_m2ts_mux_new(mux_rate, GF_M2TS_PSI_DEFAULT_REFRESH_RATE, real_time == true ? GF_TRUE : GF_FALSE);
	if (muxer != nullptr)
		throw std::runtime_error("[GPACMuxMPEG2TS] Could not create the muxer.");

	gf_m2ts_mux_use_single_au_pes_mode(muxer, single_au_pes);
	if (pcr_init_val >= 0) 
		gf_m2ts_mux_set_initial_pcr(muxer, (u64) pcr_init_val);

	gf_m2ts_mux_set_pcr_max_interval(muxer, pcr_ms);
	program = gf_m2ts_mux_program_add(muxer, 1, curPid, GF_M2TS_PSI_DEFAULT_REFRESH_RATE, pcrOffset, GF_FALSE);
}

GPACMuxMPEG2TS::~GPACMuxMPEG2TS() {
}

GF_Err GPACMuxMPEG2TS::staticFillInput(GF_ESInterface *esi, u32 ctrl_type, void *param) {
	return GF_OK;
}

GF_Err GPACMuxMPEG2TS::fillInput(GF_ESInterface *esi, u32 ctrl_type, void *param) {
	//esi->output_ctrl(esi, , );
	return GF_OK;
}

void GPACMuxMPEG2TS::declareStream(Data data) {
	auto const metadata_ = data->getMetadata();
	if (auto metadata = std::dynamic_pointer_cast<const MetadataPktLibavVideo>(metadata_)) {
		auto input = addInput(new Input<DataAVPacket>(this));
		input->setMetadata(new MetadataPktLibavVideo(metadata->getAVCodecContext()));
	} else if (auto metadata2 = std::dynamic_pointer_cast<const MetadataPktLibavAudio>(metadata_)) {
		auto input = addInput(new Input<DataAVPacket>(this));
		input->setMetadata(new MetadataPktLibavAudio(metadata2->getAVCodecContext()));
	} else 
		throw std::runtime_error("[GPACMuxMPEG2TS] Stream creation failed: unknown type.");

	//TODO: Fill the interface with test content; the current GPAC importer needs to be generalized
	GF_ESInterface ifce;
	ifce.input_ctrl = &GPACMuxMPEG2TS::staticFillInput;
	//auto stream = gf_m2ts_program_stream_add(program, &sources[i].streams[j], cur_pid+j+1, (sources[i].pcr_idx==j) ? 1 : 0, force_pes_mode);
	//if ((sources[i].streams[j].stream_type==GF_STREAM_VISUAL)) stream->start_pes_at_rap = 1;	
}

void GPACMuxMPEG2TS::process() {
	for (size_t i = 0; i < getNumInputs() - 1; ++i) {
		Data data;
		if(!inputs[0]->tryPop(data))
			continue;

		if (inputs[0]->updateMetadata(data))
			declareStream(data);
		auto encoderData = safe_cast<const DataAVPacket>(data);

		/*auto pkt =*/ encoderData->getPacket();
   
		/* write the compressed frame to the output. */
		
	}

	const char *ts_pck;
	u32 status, usec_till_next;
	while ((ts_pck = gf_m2ts_mux_process(muxer, &status, &usec_till_next)) != NULL) {
		getOutput(0)->emit(uptr(new DataRaw((uint8_t*)ts_pck, 188)));
	}
}

}
}
