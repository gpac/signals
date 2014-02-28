#include "gpac_demux_mp4_simple.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include <string>

#include "gpacpp.hpp"

namespace Demux {

class ISOFileReader {
public:
	void init(GF_ISOFile* m) {
		movie.reset(new gpacpp::IsoFile(m));
		u32 track_id = movie->getTrackId(1); //FIXME should be a parameter? hence not processed in create() but in a stateful process? or a control module?
		track_number = movie->getTrackById(track_id);
		sample_count = movie->getSampleCount(track_number);
		sample_index = 1;
	}

	std::unique_ptr<gpacpp::IsoFile> movie;
	uint32_t track_number;
	uint32_t sample_index, sample_count;
};


GPACDemuxMP4Simple* GPACDemuxMP4Simple::create(std::string const& fn) {
	/* The ISO progressive reader */
	GF_ISOFile *movie;
	/* Number of bytes required to finish the current ISO Box reading */
	u64 missing_bytes;

	GF_Err e = gf_isom_open_progressive(fn.c_str(), 0, 0, &movie, &missing_bytes);
	if ((e != GF_OK && e != GF_ISOM_INCOMPLETE_FILE) || movie == NULL) {
		Log::msg(Log::Error, "Could not open file %s for reading (%s).", fn, gf_error_to_string(e));
		throw std::runtime_error("File not found");
	}

	return new GPACDemuxMP4Simple(movie);
}

GPACDemuxMP4Simple::GPACDemuxMP4Simple(GF_ISOFile *movie)
	: reader(new ISOFileReader) {
	reader->init(movie);
	signals.push_back(uptr(pinFactory->createPin()));
}

GPACDemuxMP4Simple::~GPACDemuxMP4Simple() {
}

bool GPACDemuxMP4Simple::process(std::shared_ptr<Data> /*data*/) {
	try {
		int sample_description_index;
		std::unique_ptr<gpacpp::IsoSample> iso_sample;
		iso_sample = reader->movie->getSample(reader->track_number, reader->sample_index, sample_description_index);

		Log::msg(Log::Debug, "Found sample #%s/%s of length %s, RAP %s, DTS: %s, CTS: %s",
		         reader->sample_index,
		         reader->sample_count,
		         iso_sample->dataLength,
		         iso_sample->IsRAP,
		         iso_sample->DTS,
		         iso_sample->DTS + iso_sample->CTS_Offset);
		reader->sample_index++;

		std::shared_ptr<Data> out(signals[0]->getBuffer(iso_sample->dataLength));
		memcpy(out->data(), iso_sample->data, iso_sample->dataLength);
		signals[0]->emit(out);
	} catch(gpacpp::Error const& err) {
		if (err.error_ == GF_ISOM_INCOMPLETE_FILE) {
			u64 missing_bytes = reader->movie->getMissingBytes(reader->track_number);
			Log::msg(Log::Error, "Missing %s bytes on input file", missing_bytes);
		} else {
			return false;
		}
	}

	return true;
}

bool GPACDemuxMP4Simple::handles(const std::string &url) {
	return GPACDemuxMP4Simple::canHandle(url);
}

bool GPACDemuxMP4Simple::canHandle(const std::string &url) {
	if (url.find_last_of("mp4") + 1 == url.size()) {
		return true;
	} else {
		return false;
	}
}

}
