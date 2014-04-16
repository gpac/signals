#pragma once

#include "../../internal/config.hpp"
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

class MODULES_EXPORT GPACMuxMP4 : public Module {
public:
	static GPACMuxMP4* create(const std::string &baseName);
	~GPACMuxMP4();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

	void declareStream(std::shared_ptr<StreamVideo> stream);

private:
	GPACMuxMP4(GF_ISOFile *file);
	GF_ISOFile *file;
	uint32_t m_Dts;
	uint32_t trackId;
};

}
