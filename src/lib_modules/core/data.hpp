#pragma once

#include "clock.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace Modules {

struct IMetadata;

//A generic timed data container.
class DataBase {
public:
	DataBase() = default;
	virtual ~DataBase() {}
	virtual uint8_t* data() = 0;
	virtual const uint8_t* data() const = 0;
	virtual uint64_t size() const = 0;
	virtual void resize(size_t size) = 0;

	std::shared_ptr<const IMetadata> getMetadata() const {
		return m_metadata;
	}
	void setMetadata(std::shared_ptr<const IMetadata> metadata) {
		m_metadata = metadata;
	}

	void setTime(uint64_t timeIn180k) {
		m_TimeIn180k = timeIn180k;
	}
	void setTime(uint64_t timeIn180k, uint64_t timescale) {
		m_TimeIn180k = timescaleToClock(timeIn180k, timescale);
	}
	uint64_t getTime() const {
		return m_TimeIn180k;
	}
	void setDuration(uint64_t DurationIn180k) {
		m_DurationIn180k = DurationIn180k;
	}
	void setDuration(uint64_t DurationInTimescale, uint64_t timescale) {
		m_DurationIn180k = timescaleToClock(DurationInTimescale, timescale);
	}
	uint64_t getDuration() const {
		return m_DurationIn180k;
	}

private:
	uint64_t m_TimeIn180k;
	uint64_t m_DurationIn180k;
	std::shared_ptr<const IMetadata> m_metadata;
};

typedef std::shared_ptr<const DataBase> Data;

/* automatic inputs have a loose datatype */
struct DataLoose : public DataBase {};

class DataRaw : public DataBase {
public:
	DataRaw(size_t size) : buffer(size) {}
	DataRaw(uint8_t *buffer, size_t size) : buffer(size) {
		this->buffer.assign(buffer, buffer + size);
	}
	virtual ~DataRaw() {}
	uint8_t* data() override {
		return buffer.data();
	}
	const uint8_t* data() const override {
		return buffer.data();
	}
	uint64_t size() const override {
		return buffer.size();
	}
	void resize(size_t size) override {
		buffer.resize(size);
	}

private:
	std::vector<uint8_t> buffer;
};

}
