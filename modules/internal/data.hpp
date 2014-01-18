#pragma once

#include "config.hpp"

#include <cstdint>
#include <vector>
#include <stdlib.h>

namespace Modules {

/**
 * A generic data container.
 */
class MODULES_EXPORT IData {
public:
	IData() {
	}
	virtual ~IData() {
	}
	virtual uint8_t* data() = 0;
	virtual uint64_t size() const = 0;
	virtual void resize(size_t size) = 0;
};

//TODO: we should do get() et use the future
class MODULES_EXPORT Data : public IData {
public:
	Data(size_t size)
		: ptr(size) {
	}

	virtual ~Data() {
	}

	uint8_t* data() {
		return ptr.data();
	}

	uint64_t size() const {
		return ptr.size();
	}

	void resize(size_t size) {
		ptr.resize(size);
	}

private:
	std::vector<uint8_t> ptr;
};

}
