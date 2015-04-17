#pragma once

#include "lib_modules/core/module.hpp"

struct AVFormatContext;

using namespace Modules;

namespace Mux {

class LibavMux : public ModuleS {
public:
	LibavMux(const std::string &baseName);
	~LibavMux();
	void process(Data data) override;

private:
	void ensureHeader();

	bool declareStream(Data stream);

	struct AVFormatContext *m_formatCtx;
	bool m_headerWritten;
};

}
