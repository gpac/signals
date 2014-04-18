#pragma once

#include "../../internal/data.hpp"
#include "../../internal/pin.hpp"


namespace Modules {

class MODULES_EXPORT PcmData : public Data {
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