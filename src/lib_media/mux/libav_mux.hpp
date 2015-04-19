#pragma once

#include "lib_modules/core/module.hpp"

struct AVFormatContext;

using namespace Modules;

namespace Mux {

class LibavMux : public ModuleDynI {
public:
	LibavMux(const std::string &baseName);
	~LibavMux();
	void process(bool dataTypeUpdated) override;

private:
	void ensureHeader();

	void declareStream(Data stream);

	struct AVFormatContext *m_formatCtx;
	bool m_headerWritten;
};

}
