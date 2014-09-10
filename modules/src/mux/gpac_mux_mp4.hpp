#pragma once

#include "../common/mm.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

typedef struct __tag_isom GF_ISOFile;

namespace gpacpp {
class IsoSample;
}

using namespace Modules;

namespace Mux {

class GPACMuxMP4 : public Module {
public:
	static GPACMuxMP4* create(const std::string &baseName);
	~GPACMuxMP4();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

	void declareStream(std::shared_ptr<Stream> stream);

private:
	GPACMuxMP4(GF_ISOFile *file);
	void declareStreamVideo(std::shared_ptr<StreamVideo> stream);
	void declareStreamAudio(std::shared_ptr<StreamAudio> stream);
	GF_ISOFile *m_file;
	uint64_t m_Dts;
	uint32_t m_trackId;
};

}
