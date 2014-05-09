#include "log.hpp"
#include <iostream>


std::ostream& UTILS_EXPORT Log::get(Level /*level*/) {
	return std::cerr;
}
