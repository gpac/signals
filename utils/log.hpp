#pragma once

#include "config.hpp"
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <vector>

template<typename T>
std::string ToString(T const& val) {
	std::stringstream ss;
	ss << val;
	return ss.str();
}

template<typename T>
std::string ToString(std::vector<T> const& val) {
	std::stringstream ss;
	ss << "[";
	for(size_t i=0; i < val.size(); ++i) {
		if(i > 0)
			ss << ", ";
		ss << ToString(val[i]);
	}
	ss << "]";
	return ss.str();
}

inline std::string format(const std::string& format) {
	return format;
}

template<typename T, typename... Arguments>
std::string format(const std::string& fmt, const T& firstArg, Arguments... args) {
	std::string r;
	size_t i=0;
	while(i < fmt.size()) {
		if(fmt[i] == '%') {
			++i;
			if(i >= fmt.size())
				throw std::runtime_error("Invalid fmt specifier");

			if(fmt[i] == '%')
				r += '%';
			else if(fmt[i] == 's') {
				r += ToString(firstArg);
				return r + format(fmt.substr(i+1), args...);
			}
		} else {
			r += fmt[i];
		}
		++i;
	}
	return r;
}

class UTILS_EXPORT Log {
public:
	enum Level {
	    Quiet = -1,
	    Error = 0,
	    Warning,
	    Info,
	    Debug
	};

	template<typename... Arguments>
	static void msg(Level level, const std::string& fmt, Arguments... args) {
		if (level != Quiet) {
			get(level) << format(fmt, args...) << std::endl;
			get(level).flush();
		}
	}

private:
	Log();
	~Log();
	static Level globalLevel;
	static std::ostream& get(Level level);
};
