#pragma once

#include "internal/core/module.hpp"
#include <string>

struct AVFormatContext;

using namespace Modules;

namespace Demux {

class LibavDemux : public Module {
public:
	static LibavDemux* create(const std::string &url);
	~LibavDemux();
	void process(std::shared_ptr<Data> data);

private:
	LibavDemux(struct AVFormatContext *formatCtx);

	struct AVFormatContext *m_formatCtx;
};

}
