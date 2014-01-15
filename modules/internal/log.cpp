#include "log.hpp"
#include <iostream>

namespace Modules {


Log::Level Log::globalLevel = Log::Error;


std::ostream& Log::get(Level level) {
	return std::cerr;
}

}
