#pragma once

#include "../../internal/core/data.hpp"
#include "../../internal/core/pin.hpp"


namespace Modules {

class PcmData : public Data {
public:
	PcmData(size_t size) : Data(size) {
	}
};

typedef PinDataDefault<PcmData> PinPcm;

class PinPcmFactory : public PinFactory {
public:
	PinPcmFactory() {
	}
	Pin* createPin(IProps *props = nullptr) {
		return new PinPcm(props);
	}
};

}
