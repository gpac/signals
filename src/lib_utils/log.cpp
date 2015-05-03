#include "log.hpp"
#include <iostream>


Log::Level Log::globalLogLevel = Log::Warning;


std::ostream& Log::get(Level /*level*/) {
	return std::cerr;
}

void Log::setLevel(Level level) {
	globalLogLevel = level;
}

Log::Level Log::getLevel() {
	return globalLogLevel;
}
