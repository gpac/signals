#pragma once

#include "data.hpp"
#include "lib_utils/log.hpp"
#include <memory>

namespace Modules {

struct IMetadataCap {
	virtual ~IMetadataCap() noexcept(false) {}
	virtual std::shared_ptr<IMetadata> getMetadata() const = 0;
	virtual void setMetadata(IMetadata *metadata) = 0;
};

class MetadataCap : public IMetadataCap {
public:
	MetadataCap(IMetadata *metadata = nullptr) : m_metadata(metadata) {}
	virtual ~MetadataCap() noexcept(false) {}

	std::shared_ptr<IMetadata> getMetadata() const override {
		return m_metadata;
	}

	//Takes ownership.
	void setMetadata(IMetadata *metadata) override {
		m_metadata = std::shared_ptr<IMetadata>(metadata);
	}

protected:
	bool updateMetadata(std::shared_ptr<const Data> data) {
		if (!data->getMetadata()) {
			const_cast<Data*>(data.get())->setMetadata(m_metadata);
			return true;
		} else if (data->getMetadata() != m_metadata) {
			Log::msg(Log::Info, "Output: metadata transported by data changed. Updating.");
			m_metadata = data->getMetadata();
			return true;
		} else {
			return false;
		}
	}

	std::shared_ptr<IMetadata> m_metadata;
};

}
