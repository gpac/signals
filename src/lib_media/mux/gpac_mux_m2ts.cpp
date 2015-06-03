#include "gpac_mux_m2ts.hpp"
#include "../common/libav.hpp"
#include <cassert>

extern "C" {
#include <libavformat/avformat.h>
}

namespace Modules {

namespace Mux {

GPACMuxMPEG2TS::GPACMuxMPEG2TS() {
	addOutput(new OutputDataDefault<DataAVPacket>(nullptr));
}

GPACMuxMPEG2TS::~GPACMuxMPEG2TS() {
}

void GPACMuxMPEG2TS::declareStream(Data data) {
	auto const metadata_ = data->getMetadata();
	if (auto metadata = std::dynamic_pointer_cast<const MetadataPktLibavVideo>(metadata_)) {
		auto input = addInput(new Input<DataAVPacket>(this));
		input->setMetadata(new MetadataPktLibavVideo(metadata->getAVCodecContext()));
	} else if (auto metadata2 = std::dynamic_pointer_cast<const MetadataPktLibavAudio>(metadata_)) {
		auto input = addInput(new Input<DataAVPacket>(this));
		input->setMetadata(new MetadataPktLibavAudio(metadata2->getAVCodecContext()));
	} else {
		throw std::runtime_error("[GPACMuxMPEG2TS] Stream creation failed: unknown type.");
	}
}

void GPACMuxMPEG2TS::process() {
	for (size_t i = 0; i < getNumInputs() - 1; ++i) {
		Data data = inputs[0]->pop();
		if (inputs[0]->updateMetadata(data))
			declareStream(data);
		auto encoderData = safe_cast<const DataAVPacket>(data);
		/*auto pkt =*/ encoderData->getPacket();

		/* write the compressed frame to the output */
		//TODO: getOutput(0)->emit(data);
	}
}

}
}
