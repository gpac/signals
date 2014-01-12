#pragma once

#include "internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <cstdint>
#include <string>


class ISOProgressiveReader;


class EXPORT GPAC_MP4_Full : public Module {
public:
	static GPAC_MP4_Full* create();
	~GPAC_MP4_Full();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url); //FIXME: useless for memory-based module...
	static bool canHandle(const std::string &url); //FIXME: useless for memory-based module...

private:
	GPAC_MP4_Full();
	bool openData();
	bool updateData();
	bool processSample();
	bool processData();

	std::unique_ptr<ISOProgressiveReader> reader;
};
