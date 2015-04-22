#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Utils {

class IComparator : public ModuleS {
public:
	void process(Data data) override;
	virtual bool compare(Data original, Data other) const = 0;
	virtual void pushOriginal(Data data);
	virtual void pushOther(Data data);

private:
	Signals::Queue<Data> original, other;
};


class PcmComparator : public IComparator {
public:
	PcmComparator();
	bool compare(Data data1, Data data2) const override;
	void pushOriginal(Data data) override;
	void pushOther(Data data) override;

private:
	const float tolerance = 0.0;
};

}
}

