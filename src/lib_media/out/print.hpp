#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Out {

class Print : public ModuleS {
	public:
		Print(std::ostream &os);
		void process(Data data) override;

	private:
		Print& operator= (const Print&) = delete;

		std::ostream &os;
};

}
}
