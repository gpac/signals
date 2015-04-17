#pragma once

#include "data.hpp"
#include "lib_utils/log.hpp"
#include <memory>

namespace Modules {

struct IMetadata {
	virtual ~IMetadata() noexcept(false) {}
	virtual std::shared_ptr<IIMetadata> getMetadata() const = 0;
	virtual void setMetadata(IIMetadata *metadata) = 0;
};

class Metadata : public IMetadata {
public:
	Metadata(IIMetadata *metadata = nullptr) : metadata(metadata) {}
	virtual ~Metadata() noexcept(false) {}

	std::shared_ptr<IIMetadata> getMetadata() const override {
		return metadata;
	}

	//Takes ownership.
	void setMetadata(IIMetadata *metadata) override {
		this->metadata = std::shared_ptr<IIMetadata>(metadata);
	}

protected:
	bool updateMetadata(std::shared_ptr<const Data> data) {
		if (!data->getMetadata()) {
			const_cast<Data*>(data.get())->setMetadata(metadata);
			return true;
		} else if (data->getMetadata() != metadata) {
			Log::msg(Log::Info, "Output: metadata transported by data changed. Updating.");
			metadata = data->getMetadata();
			return true;
		} else {
			return false;
		}
	}

	std::shared_ptr<IIMetadata> metadata;
};

}
