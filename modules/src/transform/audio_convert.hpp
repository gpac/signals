#pragma once

#include "internal/module.hpp"
#include <string>

namespace Modules {
namespace Transform {

class AudioConvert : public Module {
public:
	static AudioConvert* create();
	~AudioConvert();
	bool process(std::shared_ptr<Data> data);

private:
	AudioConvert();
};

}
}
