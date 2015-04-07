#include "tests.hpp"
#include "lib_modules/modules.hpp"

#include "lib_media/encode/libav_encode.hpp"
#include "lib_utils/tools.hpp"

using namespace Tests;
using namespace Modules;

unittest("encoder: video simple") {
	std::shared_ptr<Data> picture = uptr(new PictureYUV420P(VIDEO_RESOLUTION));

	int numEncodedFrames = 0;
	auto onFrame = [&](std::shared_ptr<const Data> data) {
		numEncodedFrames++;
	};

	auto encode = uptr(new Encode::LibavEncode(Encode::LibavEncode::Video));
	ConnectPin(encode->getPin(0), onFrame);
	for (int i = 0; i < 50; ++i)
		encode->process(picture);

	ASSERT(numEncodedFrames > 0);
}

