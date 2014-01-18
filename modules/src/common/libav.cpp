#include "libav.hpp"
#include <cassert>

extern "C" {
	#include <libavcodec/avcodec.h>
}

namespace Modules {

DataDecoder::DataDecoder()
	: Data(0) {
	pkt = new AVPacket;
	av_init_packet(pkt);
	av_free_packet(pkt);
}

DataDecoder::~DataDecoder() {
	av_free_packet(pkt);
	delete pkt;
}

uint8_t* DataDecoder::data() {
	return pkt->data;
}

uint64_t DataDecoder::size() const {
	return pkt->size;
}

AVPacket* DataDecoder::getPacket() const {
	return pkt;
}

void DataDecoder::resize(size_t size) {
	assert(0);
}

}
