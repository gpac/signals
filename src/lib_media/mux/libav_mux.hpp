#pragma once

#include "../common/mm.hpp"
#include "lib_modules/core/module.hpp"

struct AVFormatContext;

using namespace Modules;

namespace Mux {

class LibavMux : public Module {
public:
	LibavMux(const std::string &baseName);
	~LibavMux();
	void process(std::shared_ptr<const Data> data) override;

private:
	void ensureHeader();

	bool declareStream(std::shared_ptr<const Data> stream);

	struct AVFormatContext *m_formatCtx;
	bool m_headerWritten;
};

}
