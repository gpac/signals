#pragma once

#include "data.hpp"
#include "lib_utils/log.hpp"
#include <memory>

namespace Modules {

//A generic property container.
struct IProperty {
	virtual ~IProperty() {}
};

class IMetadata {
public:
	virtual ~IMetadata() noexcept(false) {}
	virtual IProperty* getMetadata() const = 0;
	virtual void setMetadata(IProperty *metadata) = 0;
};

class Metadata : public IMetadata {
public:
	Metadata(IProperty *metadata = nullptr) : metadata(metadata) {} //FIXME: takes shared ptr in
	virtual ~Metadata() noexcept(false) {}

	IProperty* getMetadata() const override { //FIXME: return shared ptr
		return metadata.get();
	}

	//Takes ownership.
	void setMetadata(IProperty *metadata)  override { //FIXME: takes shared ptr in
		this->metadata = std::shared_ptr<IProperty>(metadata);
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

	std::shared_ptr<IProperty> metadata;
};

}
