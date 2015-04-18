#pragma once

#include "clock.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace Modules {

enum StreamType {
	UNKNOWN_ST = -1,
	AUDIO_RAW, //UNCOMPRESSED_AUDIO
	VIDEO_RAW, //UNCOMPRESSED_VIDEO
	AUDIO_PKT, //COMPRESSED_AUDIO
	VIDEO_PKT  //COMPRESSED_VIDEO
};

struct IMetadataPkt {
	virtual ~IMetadataPkt() {}
	virtual StreamType getStreamType() const = 0;
};
typedef IMetadataPkt IMetadata;

//A generic timed data container.
class DataBase {
public:
	DataBase() = default;
	virtual ~DataBase() {}

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

class RawData : public DataBase {
public:
	RawData(size_t size) : buffer(size) {}
	uint8_t* data() {
		return buffer.data();
	}
	const uint8_t* data() const {
		return buffer.data();
	}
	uint64_t size() const {
		return buffer.size();
	}
	void resize(size_t size) {
		buffer.resize(size);
	}

private:
	std::vector<uint8_t> buffer;
};

}
