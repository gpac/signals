#pragma once

#include "lib_modules/core/module.hpp"

struct AVFormatContext;

using namespace Modules;

namespace Mux {

class LibavMux : public Module {
public:
	LibavMux(const std::string &baseName);
	~LibavMux();
	void process(bool dataTypeUpdated) override;
	void flush() override {}
	void process(Data data) {
		if (inputs.size() == 0)
			addInput(new Input<DataBase>(this));
		assert(inputs.size() == 1);
		inputs[0]->process(data);
	}

private:
	void ensureHeader();

	void declareStream(Data stream);

	struct AVFormatContext *m_formatCtx;
	bool m_headerWritten;
};

}
