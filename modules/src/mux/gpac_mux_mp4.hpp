#pragma once

#include "../common/mm.hpp"
#include "internal/core/module.hpp"
#include "src/common/libav.hpp"
#include <string>

typedef struct __tag_isom GF_ISOFile;

namespace gpacpp {
class IsoSample;
}

using namespace Modules;

namespace Mux {

class GPACMuxMP4 : public Module {
public:
	GPACMuxMP4(const std::string &baseName, bool useSegments = false);
	~GPACMuxMP4();
	void process(std::shared_ptr<const Data> data) override;
	void flush() override;

	void declareStream(std::shared_ptr<Stream> stream);

private:
	void declareStreamVideo(std::shared_ptr<StreamVideo> stream);
	void declareStreamAudio(std::shared_ptr<StreamAudio> stream);
	void closeSegment();
	GF_ISOFile *m_iso;
	uint64_t m_DTS, m_curFragDur, m_segNum;
	uint32_t m_trackId;
	bool m_useSegments, m_useFragments;
	PinDataDefault<DataAVPacket>* output;
};

}
