#include "libav.hpp"
#include "../utils/log.hpp"
#include <cassert>
#include <cstring>

extern "C" {
	#include <libavcodec/avcodec.h>
}

namespace Modules {

DataAVPacket::DataAVPacket()
	: Data(0) {
	pkt = new AVPacket;
	av_init_packet(pkt);
	av_free_packet(pkt);
}

DataAVPacket::~DataAVPacket() {
	av_free_packet(pkt);
	delete pkt;
}

uint8_t* DataAVPacket::data() {
	return pkt->data;
}

uint64_t DataAVPacket::size() const {
	return pkt->size;
}

AVPacket* DataAVPacket::getPacket() const {
	return pkt;
}

void DataAVPacket::resize(size_t size) {
	assert(0);
}

void buildAVDictionary(const std::string &moduleName, AVDictionary **dict, const char *options, const char *type) {
	char* opt = strdup(options);
	char *tok = strtok(opt, "- ");
	char *tokval = NULL;
	while (tok && (tokval = strtok(NULL, "- "))) {
		if (av_dict_set(dict, tok, tokval, 0) < 0) { //Romain: replace ffmpeg by module name
			Log::msg(Log::Warning, "[%s] unknown %s option \"%s\" with value \"%s\"", moduleName.c_str(), type, tok, tokval);
		}
		tok = strtok(NULL, "- ");
	}
	free(opt);
}

}
