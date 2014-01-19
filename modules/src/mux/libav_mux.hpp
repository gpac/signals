#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

struct AVFormatContext;

using namespace Modules;

namespace Mux {

class MODULES_EXPORT LibavMux : public Module {
public:
	static LibavMux* create(const std::string &baseName);
	~LibavMux();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	LibavMux(struct AVFormatContext *formatCtx);
	void ensureHeader();

	struct AVFormatContext *formatCtx;
	bool headerWritten;
};

}
