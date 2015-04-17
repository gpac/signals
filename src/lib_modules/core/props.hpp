#pragma once

#include <memory>

namespace Modules {
/**
 * A generic property container.
 */
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
	PropsHandler(IProps *props) : props(props) {} //TODO: takes shared ptr in

	IProps* getProps() const override { //TODO: return shared ptr
		return props.get();
	}

	/**
	 * Takes ownership.
	 */
	void setProps(IProps *props)  override { //TODO: takes shared ptr in
		this->props = std::shared_ptr<IProps>(props);
	}

protected:
	std::shared_ptr<IProps> props;
};

}
