#include "tests.hpp"
#include "lib_modules/modules.hpp"
#include "lib_media/transform/restamp.hpp"

using namespace Tests;
using namespace Modules;

namespace {

unittest("Basic clock") {
	for (int i = 0; i < 10; ++i) {
		auto const now = g_DefaultClock->now();
		std::cout << "Time: " << now << std::endl;

		auto const duration = std::chrono::milliseconds(20);
		std::this_thread::sleep_for(duration);
	}
}

unittest("Restamp: passthru with offsets") {
	const uint64_t time = 10001;
	auto data = std::make_shared<DataRaw>(0);

	data->setTime(time);
	auto restamp = uptr(new Transform::Restamp(Transform::Restamp::Reset));
	restamp->process(data);
	ASSERT_EQUALS(0, data->getTime());

	data->setTime(time);
	restamp = uptr(new Transform::Restamp(Transform::Restamp::Reset, 0));
	restamp->process(data);
	ASSERT_EQUALS(0, data->getTime());

	data->setTime(time);
	restamp = uptr(new Transform::Restamp(Transform::Restamp::Reset, time));
	restamp->process(data);
	ASSERT_EQUALS(time, data->getTime());
}

unittest("Restamp: reset with offsets") {
	uint64_t time = 10001;
	int64_t offset = -100;
	auto data = std::make_shared<DataRaw>(0);

	data->setTime(time);
	auto restamp = uptr(new Transform::Restamp(Transform::Restamp::Passthru));
	restamp->process(data);
	ASSERT_EQUALS(time, data->getTime());

	data->setTime(time);
	restamp = uptr(new Transform::Restamp(Transform::Restamp::Passthru, 0));
	restamp->process(data);
	ASSERT_EQUALS(time, data->getTime());

	data->setTime(time);
	restamp = uptr(new Transform::Restamp(Transform::Restamp::Passthru, offset));
	restamp->process(data);
	ASSERT_EQUALS(time + offset, data->getTime());

	data->setTime(time);
	restamp = uptr(new Transform::Restamp(Transform::Restamp::Passthru, time));
	restamp->process(data);
	ASSERT_EQUALS(time+time, data->getTime());
}

}
