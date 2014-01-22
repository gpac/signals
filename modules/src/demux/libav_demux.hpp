#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

struct AVFormatContext;

using namespace Modules;

namespace Demux {

class MODULES_EXPORT LibavDemux : public Module {
public:
	static LibavDemux* create(const std::string &url);
	~LibavDemux();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	LibavDemux(struct AVFormatContext *formatCtx, std::vector<Pin*> signals);

	struct AVFormatContext *formatCtx;
};

}