#pragma once

#include "internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <cstdint>
#include <string>

typedef struct __tag_isom GF_ISOFile;
struct GF_ISOSample;


class EXPORT GPAC_MP4_Simple : public Module {
public:
	static GPAC_MP4_Simple* create(const Param &parameters);
	~GPAC_MP4_Simple();
	bool process(Data *data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url); //Romain: useful?

private:
	GPAC_MP4_Simple(GF_ISOFile *movie);

	void deleteLastSample();

	GF_ISOFile *movie;
	uint32_t track_number;

	GF_ISOSample *iso_sample;
	uint32_t sample_index, sample_count;
};
