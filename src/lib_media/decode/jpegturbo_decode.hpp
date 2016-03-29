#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/picture.hpp"


namespace Modules {
namespace Decode {

class JPEGTurbo;

class JPEGTurboDecode : public ModuleS {
	public:
		JPEGTurboDecode();
		~JPEGTurboDecode();
		void process(Data data) override;

	private:
		OutputPicture* output;
		void ensureMetadata(int width, int height, int pixelFmt);
		std::unique_ptr<JPEGTurbo> const jtHandle;
};

}
}
