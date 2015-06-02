#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Mux {

class GPACMuxMPEG2TS : public ModuleDynI {
public:
	GPACMuxMPEG2TS();
	~GPACMuxMPEG2TS();
	void process() override;

private:
	void declareStream(Data data);
	//TODO
};

}
}
