#pragma once

#include "lib_utils/format.hpp"
#include <stdexcept>
#include <string>


namespace Modules {

class Exception : public std::exception {
public:
	Exception(std::string const &msg) throw() : msg(msg) {}
	~Exception() throw() {}

	char const* what() const throw() {
		return msg.c_str();
	}

private:
	Exception& operator= (const Exception&) = delete;
	std::string msg;
};

struct IError {
	virtual std::exception error(std::string const &msg) {
		throw Exception(format("[%s] %s", typeid(*this).name(), msg));
	}
};

}
