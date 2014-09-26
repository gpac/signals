#pragma once

#include "../common/mm.hpp"
#include "internal/core/module.hpp"
#include <string>

typedef struct __tag_isom GF_ISOFile;

namespace gpacpp {
class IsoSample;
}

using namespace Modules;

namespace Mux {

class GPACMuxMP4 : public Module {
public:
	GPACMuxMP4(const std::string &baseName);
	~GPACMuxMP4();
	bool process(std::shared_ptr<Data> data);

	void declareStream(std::shared_ptr<Stream> stream);

private:
	void declareStreamVideo(std::shared_ptr<StreamVideo> stream);
	void declareStreamAudio(std::shared_ptr<StreamAudio> stream);
	GF_ISOFile *m_file;
	uint64_t m_DTS, m_curFragDur;
	uint32_t m_trackId;
};

}
