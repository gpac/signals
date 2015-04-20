#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/libav.hpp"
#include <string>

typedef struct __tag_isom GF_ISOFile;

namespace gpacpp {
class IsoSample;
}

using namespace Modules;

namespace Mux {

class GPACMuxMP4 : public ModuleDynI {
public:
	GPACMuxMP4(const std::string &baseName, bool useSegments = false, uint64_t segDurationInMs = 2000);
	~GPACMuxMP4();
	void process() override;
	void flush() override;

private:
	void declareStream(Data stream);
	void declareStreamVideo(std::shared_ptr<const MetadataPktLibavVideo> stream);
	void declareStreamAudio(std::shared_ptr<const MetadataPktLibavAudio> stream);
	void setupFragments();
	void closeSegment();
	GF_ISOFile *m_iso;
	uint64_t m_DTS, m_curFragDur, m_segNum;
	uint32_t m_trackId;
	bool m_useSegments, m_useFragments;
	uint64_t m_segDuration;
	OutputDataDefault<DataAVPacket>* output;
};

}
