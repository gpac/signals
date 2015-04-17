#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Utils {

class IComparator : public ModuleS {
public:
	void process(std::shared_ptr<const Data> data) override;
	virtual bool compare(std::shared_ptr<const Data> original, std::shared_ptr<const Data> other) const = 0;
	virtual void pushOriginal(std::shared_ptr<const Data> data);
	virtual void pushOther(std::shared_ptr<const Data> data);

private:
	Queue<std::shared_ptr<const Data>> original, other;
};


class PcmComparator : public IComparator {
public:
	bool compare(std::shared_ptr<const Data> data1, std::shared_ptr<const Data> data2) const override;
	void pushOriginal(std::shared_ptr<const Data> data) override;
	void pushOther(std::shared_ptr<const Data> data) override;

private:
	const float tolerance = 0.0;
};

}
}

