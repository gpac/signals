#pragma once

#include "../common/mm.hpp"
#include "internal/module.hpp"
#include <string>

struct AVFormatContext;

using namespace Modules;

namespace Mux {

class LibavMux : public Module {
public:
	static LibavMux* create(const std::string &baseName);
	~LibavMux();
	bool process(std::shared_ptr<Data> data);

	void declareStream(std::shared_ptr<Stream> stream);

private:
	LibavMux(struct AVFormatContext *formatCtx);
	void ensureHeader();

	struct AVFormatContext *m_formatCtx;
	bool m_headerWritten;
};

}
