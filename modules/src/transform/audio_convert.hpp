#pragma once

#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>

namespace Modules {
namespace Transform {

class AudioConvert : public Module {
public:
	static AudioConvert* create();
	~AudioConvert();
	bool handles(const std::string &url);
	bool process(std::shared_ptr<Data> data);
	static bool canHandle(const std::string &url);

private:
	AudioConvert();
};

}
}
