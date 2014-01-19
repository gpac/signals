#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

struct AVFormatContext;

using namespace Modules;

namespace Mux {

class MODULES_EXPORT Libavformat_55 : public Module {
public:
	static Libavformat_55* create(const std::string &baseName);
	~Libavformat_55();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	Libavformat_55(struct AVFormatContext *formatCtx);
	void ensureHeader();

	struct AVFormatContext *formatCtx;
	bool headerWritten;
};

}
