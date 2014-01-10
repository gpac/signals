#include "gpac_mp4_simple.hpp"
#include "internal/log.hpp"
#include <string>

extern "C" {
#include <gpac/tools.h>
#include <gpac/isomedia.h>
}


GPAC_MP4_Simple* GPAC_MP4_Simple::create(const Param &parameters) {
	auto filename = parameters.find("filename");
	if (filename == parameters.end()) {
		return NULL;
	}
	const std::string &fn = (*filename).second;

	/* The ISO progressive reader */
	GF_ISOFile *movie;
	/* Number of bytes required to finish the current ISO Box reading */
	u64 missing_bytes;

	GF_Err e = gf_isom_open_progressive(fn.c_str(), 0, 0, &movie, &missing_bytes);
	if ((e != GF_OK && e != GF_ISOM_INCOMPLETE_FILE) || movie == NULL) {
		Log::get(Log::Error) << "Could not open file " << fn << " for reading (" << gf_error_to_string(e) << ")." << std::endl;
		return NULL;
	}

	return new GPAC_MP4_Simple(movie);
}

void GPAC_MP4_Simple::deleteLastSample() {
	if (iso_sample) {
		//invalidate the previous data and delete the sample.
		iso_sample->data = NULL;
		iso_sample->dataLength = 0;
		gf_isom_sample_del(&iso_sample);
	}
}

GPAC_MP4_Simple::GPAC_MP4_Simple(GF_ISOFile *movie)
: movie(movie), iso_sample(NULL) {
	u32 track_id = gf_isom_get_track_id(movie, 1); //FIXME should be a parameter? hence not processed in create() but in a stateful process? or a control module?
	track_number = gf_isom_get_track_by_id(movie, track_id);
	if (track_number == 0) {
		Log::get(Log::Error) << "Could not find track ID=" << track_id << std::endl;
	}
	sample_count = gf_isom_get_sample_count(movie, track_number);
	sample_index = 1;
	signals.push_back(new Pin);
}

GPAC_MP4_Simple::~GPAC_MP4_Simple() {
	deleteLastSample();
	gf_isom_close(movie);
	delete signals[0];
}

bool GPAC_MP4_Simple::process(Data *data) {
	delete data; //Romain: actually reads from file
	deleteLastSample();

	u32 sample_description_index;
	iso_sample = gf_isom_get_sample(movie, track_number, sample_index, &sample_description_index);
	if (iso_sample) {
		Log::get(Log::Error) << "Found sample #" << sample_index << "/" << sample_count << " of length " << iso_sample->dataLength << ", RAP: " << iso_sample->IsRAP << ", DTS: " << iso_sample->DTS << ", CTS: " << iso_sample->DTS + iso_sample->CTS_Offset << std::endl;
		sample_index++;

		Data *out = new Data(iso_sample->dataLength);
		memcpy(out->data(), iso_sample->data, iso_sample->dataLength);
		signals[0]->emit(out);

		/*release the sample data, once you're done with it*/
		gf_isom_sample_del(&iso_sample); //FIXME: embed it in the allocator:
	} else {
		GF_Err e = gf_isom_last_error(movie);
		if (e == GF_ISOM_INCOMPLETE_FILE) {
			u64 missing_bytes = gf_isom_get_missing_bytes(movie, track_number);
			Log::get(Log::Error) << "Missing " << missing_bytes << " bytes on input file" << std::endl;
		} else {
			return false;
		}
	}

	return true;
}

bool GPAC_MP4_Simple::handles(const std::string &url) {
	return GPAC_MP4_Simple::canHandle(url);
}

bool GPAC_MP4_Simple::canHandle(const std::string &url) {
	if (url.find_last_of("mp4") + 1 == url.size()) {
		return true;
	} else {
		return false;
	}
}
