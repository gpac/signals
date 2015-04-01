#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>


namespace Modules {

class Module {
public:
	Module() = default;

	virtual ~Module() noexcept(false) {
	}

	virtual void process(std::shared_ptr<const Data> data) = 0;
	virtual void flush() { };

	size_t getNumPin() const {
		return pins.size();
	}

	IPin* getPin(size_t i) {
		return pins[i].get();
	}

	void setLowLatency(bool isLowLatency) {
		m_isLowLatency = isLowLatency;
	}

protected:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;

	template<typename T>
	T* addPin(T* p) {
		if (m_isLowLatency)
			p->setAllocator(new typename T::AllocatorType(ALLOC_NUM_BLOCKS_LOW_LATENCY));
		pins.push_back(uptr(p));
		return p;
	}

private:
	std::vector<std::unique_ptr<IPin>> pins;
	bool m_isLowLatency = false;
};

}
