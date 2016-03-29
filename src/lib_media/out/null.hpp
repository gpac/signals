#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Out {

//Open bar output. Thread-safe by design ©
class Null : public ModuleS {
	public:
		Null();
		void process(Data data) override;
};

}
}
