#include "log.hpp"
#include <iostream>


Log::Level Log::globalLevel = Log::Error;


std::ostream& Log::get(Level /*level*/) {
	return std::cerr;
}
