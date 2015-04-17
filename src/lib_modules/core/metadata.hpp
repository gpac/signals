#pragma once

#include "data.hpp"
#include "lib_utils/log.hpp"
#include <memory>

namespace Modules {

//A generic property container.
struct IProps {
	virtual ~IProps() {}
};

class IPropsHandler {
public:
	virtual ~IPropsHandler() {}
	virtual IProps* getProps() const = 0;
	virtual void setProps(IProps *props) = 0;
};

class PropsHandler : public IPropsHandler {
public:
	PropsHandler(IProps *props) : props(props) {} //FIXME: takes shared ptr in
	virtual ~PropsHandler() {}

	IProps* getProps() const override { //FIXME: return shared ptr
		return props.get();
	}

	//Takes ownership.
	void setProps(IProps *props)  override { //FIXME: takes shared ptr in
		this->props = std::shared_ptr<IProps>(props);
	}

protected:
	bool updateMetadata(std::shared_ptr<const Data> data) {
		if (!data->getMetadata()) {
			const_cast<Data*>(data.get())->setMetadata(props);
			return true;
		} else if (data->getMetadata() != props) {
			Log::msg(Log::Info, "Output: metadata transported by data changed. Updating.");
			props = data->getMetadata();
			return true;
		} else {
			return false;
		}
	}

	std::shared_ptr<IProps> props;
};

}
