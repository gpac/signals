#pragma once

#include "internal/core/module.hpp"
#include <string>

using namespace Modules;

namespace Out {

class Print : public Module {
public:
	Print(std::ostream &os);
	bool process(std::shared_ptr<Data> data);

private:
	Print& operator= (const Print&) = delete;

	std::ostream &os;
};

}
