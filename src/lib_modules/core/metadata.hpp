#pragma once

#include "data.hpp"
#include "lib_utils/log.hpp"
#include <memory>

namespace Modules {

struct IMetadataCap {
	virtual ~IMetadataCap() noexcept(false) {}
	virtual std::shared_ptr<const IMetadata> getMetadata() const = 0;
	virtual void setMetadata(IMetadata *metadata) = 0;
	virtual void setMetadata(std::shared_ptr<const IMetadata> metadata) = 0;
};

enum StreamType {
	UNKNOWN_ST = -1,
	AUDIO_RAW, //UNCOMPRESSED_AUDIO
	VIDEO_RAW, //UNCOMPRESSED_VIDEO
	AUDIO_PKT, //COMPRESSED_AUDIO
	VIDEO_PKT  //COMPRESSED_VIDEO
};

struct IMetadata {
	virtual ~IMetadata() {}
	virtual StreamType getStreamType() const = 0;
};

class MetadataFile : public IMetadata {
public:
	MetadataFile(const std::string &filename) : filename(filename) {}
	std::string getFilename() const {
		return filename;
	}
	virtual StreamType getStreamType() const override {
		return UNKNOWN_ST;
	}

private:
	std::string filename;
};

//TODO: should be picture and Pcm and return the same fields as MetadataPkt
struct MetadataRawVideo : public IMetadata {
	virtual StreamType getStreamType() const override {
		return VIDEO_RAW;
	}
};

struct MetadataRawAudio : public IMetadata {
	virtual StreamType getStreamType() const override {
		return AUDIO_RAW;
	}
};

struct MetadataPkt : public IMetadata {
};

struct MetadataPktVideo : public MetadataPkt {
	virtual StreamType getStreamType() const override {
		return VIDEO_PKT;
	}
};

struct MetadataPktAudio : public MetadataPkt {
	virtual StreamType getStreamType() const override {
		return AUDIO_PKT;
	}
};

class MetadataCap : public IMetadataCap {
public:
	MetadataCap(IMetadata *metadata = nullptr) : m_metadata(metadata) {}
	virtual ~MetadataCap() noexcept(false) {}

	std::shared_ptr<const IMetadata> getMetadata() const override {
		return m_metadata;
	}

	//Takes ownership.
	void setMetadata(IMetadata *metadata) override {
		m_metadata = std::shared_ptr<const IMetadata>(metadata);
	}
	void setMetadata(std::shared_ptr<const IMetadata> metadata) override {
		m_metadata = metadata;
	}

	bool updateMetadata(Data data) {
		if (!data) {
			return false;
		} else if (!data->getMetadata()) {
			const_cast<DataBase*>(data.get())->setMetadata(m_metadata);
			return true;
		} else if (data->getMetadata() != m_metadata) {
			if (m_metadata) {
				Log::msg(Log::Info, "Metadata update from data not supported yet: output pin and data won't carry the same metadata.");
				return true;
			}
			Log::msg(Log::Info, "Output: metadata transported by data changed. Updating.");
			if (m_metadata && (data->getMetadata()->getStreamType() != m_metadata->getStreamType()))
				throw std::runtime_error(format("Metadata update: incompatible types %s for data and %s for attached", data->getMetadata()->getStreamType(), m_metadata->getStreamType()));
			m_metadata = data->getMetadata();
			return true;
		} else {
			return false;
		}
	}

private:
	std::shared_ptr<const IMetadata> m_metadata;
};

}
