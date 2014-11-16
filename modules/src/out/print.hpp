#pragma once

#include "internal/core/module.hpp"

namespace Modules {
namespace Out {

class Print : public Module {
public:
	Print(std::ostream &os);
	void process(std::shared_ptr<const Data> data) override;

private:
	Print& operator= (const Print&) = delete;

	std::ostream &os;
};

}
}
