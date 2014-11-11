#pragma once

#include "internal/core/module.hpp"

namespace Modules {
namespace Utils {

class IComparator : public Module {
public:
	void process(std::shared_ptr<Data> data) override {
		if (data != nullptr)
			throw std::runtime_error("[Comparator] data not expected");

		for (;;) {
			std::shared_ptr<Data> aData, bData;
			auto a = original.tryPop(aData);
			auto b = other.tryPop(bData);
			if (!a || !b) {
				if (a || b)
					throw std::runtime_error("[Comparator] not the same number of samples");
				Log::msg(Log::Info, "[Comparator] end of process");
				break;
			}

			auto res = compare(aData, bData);
			if (!res)
				Log::msg(Log::Info, "[Comparator] comparison failed");
		}
	}
	virtual void pushOriginal(std::shared_ptr<Data> data) {
		original.push(data);
	}
	virtual void pushOther(std::shared_ptr<Data> data) {
		other.push(data);
	}

	virtual bool compare(std::shared_ptr<Data> original, std::shared_ptr<Data> other) const = 0;

private:
	Queue<std::shared_ptr<Data>> original, other;
};


class PcmComparator : public IComparator {
public:
	bool compare(std::shared_ptr<Data> data1, std::shared_ptr<Data> data2) const override {
		auto pcm1 = safe_cast<PcmData>(data1);
		auto pcm2 = safe_cast<PcmData>(data2);
		if (pcm1->getFormat() != pcm2->getFormat())
			throw std::runtime_error("[PcmComparator] Incompatible audio data");

		auto const size1 = pcm1->size();
		auto const size2 = pcm2->size();
		if (size1 != size2)
			Log::msg(Log::Warning, "[PcmComparator] Sample sizes are different, comparing the overlap.");
		PcmData *data;
		if (size1 < size2)
			data = pcm1.get();
		else
			data = pcm2.get();

		for (size_t planeIdx = 0; planeIdx < data->getFormat().numPlanes; ++planeIdx) {
			for (size_t i = 0; i < data->getPlaneSize(planeIdx); ++i) {
				if (fabs(pcm1->getPlane(planeIdx)[i] - pcm2->getPlane(planeIdx)[i]) > tolerance) {
					std::stringstream ss;
					ss << "[PcmComparator] Samples are different at plane " << planeIdx << ", index " << i << "." << std::endl;
					throw std::runtime_error(ss.str());
					return false;
				}
			}
		}

		return true;
	}
	virtual void pushOriginal(std::shared_ptr<Data> data) {
		IComparator::pushOriginal(data);
	}
	virtual void pushOther(std::shared_ptr<Data> data) {
		IComparator::pushOther(data);
	}

private:
	const float tolerance = 0.0;
};

}
}

