#pragma once

#include "../../internal/data.hpp"


namespace Modules {

class MODULES_EXPORT PcmData : public Data {
public:
	PcmData(size_t size) : Data(size) {
	}
};

}