#pragma once

#include "lib_utils/log.hpp"


namespace Modules {

struct ILog {
	virtual ~ILog() noexcept(false) {}

	template<typename... Arguments>
	void log(Level level, const std::string& fmt, Arguments... args) {
		Log::msg(level, format("[%s] %s", typeid(*this).name(), format(fmt, args...)));
	}
};

}
