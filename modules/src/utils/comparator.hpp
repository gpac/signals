#pragma once

#include "internal/core/module.hpp"

namespace Modules {
namespace Utils {

class IComparator : public Module {
public:
	void process(std::shared_ptr<Data> data) override;
	virtual bool compare(std::shared_ptr<Data> original, std::shared_ptr<Data> other) const = 0;
	virtual void pushOriginal(std::shared_ptr<Data> data);
	virtual void pushOther(std::shared_ptr<Data> data);

private:
	Queue<std::shared_ptr<Data>> original, other;
};


class PcmComparator : public IComparator {
public:
	bool compare(std::shared_ptr<Data> data1, std::shared_ptr<Data> data2) const override;
	virtual void pushOriginal(std::shared_ptr<Data> data) override;
	virtual void pushOther(std::shared_ptr<Data> data) override;

private:
	const float tolerance = 0.0;
};

}
}

