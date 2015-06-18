#include "gpac_mux_m2ts.hpp"
#include "../common/libav.hpp"
#include <cassert>

extern "C" {
#include <gpac/constants.h>
#include <gpac/mpegts.h>
#include <libavformat/avformat.h>
}

namespace Modules  {
namespace Mux  {

const Bool single_au_pes = GF_FALSE;
const int pcrOffset = 0;
const int curPid = 100;

struct UserData {
	UserData(GPACMuxMPEG2TS *muxer, size_t inputIdx) : muxer(muxer), inputIdx(inputIdx) {}
	GPACMuxMPEG2TS *muxer;
	size_t inputIdx;
};

GPACMuxMPEG2TS::GPACMuxMPEG2TS(bool real_time, unsigned mux_rate, unsigned pcr_ms, int64_t pcr_init_val) {
	addOutput(new OutputDataDefault<DataAVPacket>(nullptr));

	muxer = gf_m2ts_mux_new(mux_rate, GF_M2TS_PSI_DEFAULT_REFRESH_RATE, real_time == true ? GF_TRUE : GF_FALSE);
	if (muxer == nullptr)
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
	auto userData = (UserData*)esi->input_udta;
	return userData->muxer->fillInput(esi, ctrl_type, userData->inputIdx);
}

GF_Err GPACMuxMPEG2TS::fillInput(GF_ESInterface *esi, u32 ctrl_type, size_t inputIdx) {
	switch (ctrl_type) {
	case GF_ESI_INPUT_DATA_FLUSH: {
		std::shared_ptr<const DataAVPacket> data;
		while (inputData[inputIdx]->tryPop(data)) {
			//TODO: this is the libav strcuture, copy the fiels to match the GF_ESIPacket
			/*auto pkt = data->getPacket();

			pkt.dts      = 0;
			pkt.pts      = 512;
			pkt.size     = 1088; //pck.data_len
			pkt.flags    = 31;
			pkt.duration = 512;

			//esi->output_ctrl(esi, , );*/
		}
		break;
	}
	case GF_ESI_INPUT_DESTROY:
		delete (UserData*)esi->input_udta;
		return GF_OK;
	default:
		return GF_BAD_PARAM;
	}

	return GF_OK;
}

void GPACMuxMPEG2TS::declareStream(Data data) {
	const size_t inputIdx = inputs.size() - 1;
	inputData.resize(inputIdx + 1);
	inputData[inputIdx] = uptr(new DataInput);

	GF_ESInterface ifce;
	memset(&ifce, 0, sizeof(ifce));

	auto const metadata_ = data->getMetadata();
	if (auto metadata = std::dynamic_pointer_cast<const MetadataPktLibavVideo>(metadata_)) {
		auto input = addInput(new Input<DataAVPacket>(this));

		ifce.caps = GF_ESI_SIGNAL_DTS;
		ifce.stream_id = 1;
		ifce.program_number = 0;
		ifce.stream_type = GF_STREAM_VISUAL;
		ifce.object_type_indication = GPAC_OTI_VIDEO_AVC;
		ifce.fourcc = 0;
		ifce.lang = 0;
		ifce.timescale = 12288;
		ifce.duration = 608.125;
		ifce.bit_rate = 1696968;
		ifce.repeat_rate = 0;
		ifce.info_video.par = 0;
		ifce.info_video.width = 0;
		ifce.info_video.height = 0;
#if 0
		GF_SAFEALLOC(ifce.sl_config, GF_SLConfig);
		ifce.sl_config->tag = GF_ODF_SLC_TAG;
		ifce.sl_config->predefined = 3;
		ifce.sl_config->useAccessUnitStartFlag = 1;
		ifce.sl_config->useAccessUnitEndFlag = 1;
		ifce.sl_config->useRandomAccessPointFlag = 1;
		ifce.sl_config->useTimestampsFlag = 1;
		ifce.sl_config->timestampLength = 33;
		ifce.sl_config->timestampResolution = ifce.timescale;
#endif

		input->setMetadata(new MetadataPktLibavVideo(metadata->getAVCodecContext()));
	} else if (auto metadata2 = std::dynamic_pointer_cast<const MetadataPktLibavAudio>(metadata_)) {
#if 0
		auto input = addInput(new Input<DataAVPacket>(this));

		ifce.info_audio.sample_rate = 0;
		ifce.info_audio.nb_channels = 0;

		input->setMetadata(new MetadataPktLibavAudio(metadata2->getAVCodecContext()));
#else
		throw std::runtime_error("[GPACMuxMPEG2TS] Stream (audio) creation failed: unknown type.");
#endif
	} else 
		throw std::runtime_error("[GPACMuxMPEG2TS] Stream creation failed: unknown type.");

	//TODO: Fill the interface with test content; the current GPAC importer needs to be generalized
	ifce.input_ctrl = &GPACMuxMPEG2TS::staticFillInput;
	ifce.input_udta = (void*)new UserData(this, inputIdx);
	ifce.output_udta = nullptr;

	Bool isPCR = (ifce.stream_type == GF_STREAM_AUDIO) ? GF_TRUE : GF_FALSE;
	auto stream = gf_m2ts_program_stream_add(program, &ifce, (u32)(curPid + inputIdx), isPCR, GF_FALSE);
	if ((ifce.stream_type==GF_STREAM_VISUAL)) stream->start_pes_at_rap = GF_TRUE;
}

void GPACMuxMPEG2TS::process() {
	for (size_t i = 0; i < getNumInputs() - 1; ++i) {
		Data data;
		while (inputs[i]->tryPop(data)) {
			if (inputs[i]->updateMetadata(data))
				declareStream(data);
			auto encoderData = safe_cast<const DataAVPacket>(data);

			inputData[i]->push(encoderData);
		}
	}

	const char *ts_pck;
	u32 status, usec_till_next;
	while ((ts_pck = gf_m2ts_mux_process(muxer, &status, &usec_till_next)) != nullptr) {
		getOutput(0)->emit(uptr(new DataRaw((uint8_t*)ts_pck, 188)));
	}
}

}
}
