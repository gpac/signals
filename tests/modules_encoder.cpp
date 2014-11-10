#include "tests.hpp"
#include "modules.hpp"

#include "encode/libav_encode.hpp"
#include "tools.hpp"

using namespace Tests;
using namespace Modules;

unittest("encoder: video simple") {
	std::shared_ptr<Data> picture = uptr(new Picture(Resolution(VIDEO_WIDTH, VIDEO_HEIGHT)));

	int numEncodedFrames = 0;
	auto onFrame = [&](std::shared_ptr<Data> data) {
		numEncodedFrames++;
	};

	auto encode = uptr(new Encode::LibavEncode(Encode::LibavEncode::Video));
	ConnectPin(encode->getPin(0), onFrame);
	for(int i=0;i < 50;++i)
		encode->process(picture);

	ASSERT(numEncodedFrames > 0);
}

