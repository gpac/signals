#pragma once

#include "internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <cstdint>
#include <string>


typedef struct __tag_isom GF_ISOFile;
class ISOFileReader;


class EXPORT GPAC_MP4_Simple : public Module {
public:
	static GPAC_MP4_Simple* create(std::string const& path);
	~GPAC_MP4_Simple();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	GPAC_MP4_Simple(GF_ISOFile *movie);

	std::unique_ptr<ISOFileReader> reader;
};
