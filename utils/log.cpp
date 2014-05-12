#include "log.hpp"
#include <iostream>


std::ostream& Log::get(Level /*level*/) {
	return std::cerr;
}
