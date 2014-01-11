#include "gpac_mp4_full.hpp"
#include "internal/log.hpp"
#include <string>
#include <sstream>

extern "C" {
#include <gpac/tools.h>
#include <gpac/isomedia.h>
#include <gpac/thread.h>
}


//#define GPAC_MEM_TRACKER


//TODO: set appropriate CCO credits
//see http://sourceforge.net/p/gpac/code/HEAD/tree/trunk/gpac/applications/testapps/fmp4demux/main.c
class ISOProgressiveReader {
public:
	ISOProgressiveReader()
		: data(0), movie(NULL), refresh_boxes(GF_TRUE), samples_processed(0), sample_index(1), sample_count(0), track_number(1) {
#ifdef GPAC_MEM_TRACKER
		gf_sys_init(GF_TRUE);
#endif
	}

	~ISOProgressiveReader() {
		gf_isom_close(movie);
#ifdef GPAC_MEM_TRACKER
		gf_sys_close();
#endif
	}
 
	/* data buffer to be read by the parser */
	std::vector<u8> data;
	/* URL used to pass a buffer to the parser */
	char data_url[256];
	/* The ISO file structure created for the parsing of data */
	GF_ISOFile *movie;
	/* Boolean state to indicate if the needs to be parsed */
	Bool refresh_boxes;
	u32 samples_processed;
	u32 sample_index; /* samples are numbered starting from 1 */
	u32 sample_count;
	int track_number; //TODO: multi-tracks
};

GPAC_MP4_Full* GPAC_MP4_Full::create(const Param &parameters) {
	return new GPAC_MP4_Full();
}

GPAC_MP4_Full::GPAC_MP4_Full()
: reader(new ISOProgressiveReader) {
#ifndef GPAC_MEM_TRACKER
	gf_sys_init(GF_FALSE);
#endif
	//gf_log_set_tool_level(GF_LOG_ALL, GF_LOG_INFO);
	signals.push_back(new Pin);
}

GPAC_MP4_Full::~GPAC_MP4_Full() {
	delete signals[0];
#ifndef GPAC_MEM_TRACKER
	gf_sys_close();
#endif
}

bool GPAC_MP4_Full::openData() {
	/* if the file is not yet opened (no movie), open it in progressive mode (to update its data later on) */
	u64 missing_bytes;
	GF_Err e = gf_isom_open_progressive(reader->data_url, 0, 0, &reader->movie, &missing_bytes);
	if ((e != GF_OK && e != GF_ISOM_INCOMPLETE_FILE) || reader->movie) {
		Log::get(Log::Warning) << "Error opening fragmented mp4 in progressive mode: " << gf_error_to_string(e) << " (missing " << missing_bytes << " bytes)" << std::endl;
		return false;
	}
	gf_isom_set_single_moof_mode(reader->movie, GF_TRUE);
	return true;
}

bool GPAC_MP4_Full::updateData() {
	/* let inform the parser that the buffer has been updated with new data */
	u64 missing_bytes;
	GF_Err e = gf_isom_refresh_fragmented(reader->movie, &missing_bytes, reader->data_url);
	if (e != GF_OK && e != GF_ISOM_INCOMPLETE_FILE) {
		Log::get(Log::Warning) << "Error refreshing fragmented mp4: " << gf_error_to_string(e) << " (missing " << missing_bytes << " bytes)" << std::endl;
		return false;
	}
	return true;
}

bool GPAC_MP4_Full::processSample() {
	/* only if we have the track number can we try to get the sample data */
	if (reader->track_number != 0) {
		u32 new_sample_count;
		u32 di; /*descriptor index*/

		/* let's see how many samples we have since the last parsed */
		new_sample_count = gf_isom_get_sample_count(reader->movie, reader->track_number);
		if (new_sample_count > reader->sample_count) {
			/* New samples have been added to the file */
			Log::get(Log::Info) << "Found " << new_sample_count - reader->sample_count << " new samples (total: " << new_sample_count << ")" << std::endl;
			if (reader->sample_count == 0) {
				reader->sample_count = new_sample_count;
			}
		}
		if (reader->sample_count == 0) {
			/* no sample yet, let the data input force a reparsing of the data */
			reader->refresh_boxes = GF_TRUE;
		} else {
			/* we have some samples, lets keep things stable in the parser for now and
			don't let the data input force a reparsing of the data */
			reader->refresh_boxes = GF_FALSE;

			/* let's analyze the samples we have parsed so far one by one */
			GF_ISOSample *iso_sample = gf_isom_get_sample(reader->movie, reader->track_number, reader->sample_index, &di);
			if (iso_sample) {
				/* if you want the sample description data, you can call:
				GF_Descriptor *desc = gf_isom_get_decoder_config(reader->movie, reader->track_handle, di);
				*/

				reader->samples_processed++;
				/*here we dump some sample info: samp->data, samp->dataLength, samp->isRAP, samp->DTS, samp->CTS_Offset */
				Log::get(Log::Info) << "Found sample #" << reader->sample_index << "(#" << reader->samples_processed << ") of length " << iso_sample->dataLength << ", RAP: " << iso_sample->IsRAP << ", DTS : " << iso_sample->DTS << ", CTS : " << iso_sample->DTS + iso_sample->CTS_Offset << std::endl;
				reader->sample_index++;

				std::shared_ptr<Data> out(new Data(iso_sample->dataLength));
				memcpy(out->data(), iso_sample->data, iso_sample->dataLength);
				signals[0]->emit(out);

				/*release the sample data, once you're done with it*/
				gf_isom_sample_del(&iso_sample);

				/* once we have read all the samples, we can release some data and force a reparse of the input buffer */
				if (reader->sample_index > reader->sample_count) {
					u64 new_buffer_start;
					u64 missing_bytes;

					Log::get(Log::Debug) << std::endl << "Releasing unnecessary buffers" << std::endl;
					/* release internal structures associated with the samples read so far */
					gf_isom_reset_tables(reader->movie, GF_TRUE);

					/* release the associated input data as well */
					gf_isom_reset_data_offset(reader->movie, &new_buffer_start);
					if (new_buffer_start) {
						u32 offset = (u32)new_buffer_start;
						const size_t newSize = reader->data.size() - offset;
						memmove(reader->data.data(), reader->data.data() + offset, newSize);
						reader->data.resize(newSize);
					}
					std::stringstream ss;
					ss << "gmem://" << reader->data.size() << "@" << (void*)reader->data.data();
					strcpy(reader->data_url, ss.str().c_str());
					gf_isom_refresh_fragmented(reader->movie, &missing_bytes, reader->data_url);

					/* update the sample count and sample index */
					reader->sample_count = new_sample_count - reader->sample_count;
					//FIXME: doesn't happen because the size of the chunck is not aligned on samples: assert(reader->sample_count == 0);
					reader->sample_index = 1;
				}
			} else {
				GF_Err e = gf_isom_last_error(reader->movie);
				Log::get(Log::Warning) << "Could not get sample " << gf_error_to_string(e) << std::endl;
				return false;
			}
		}
	}

	return true;
}

bool GPAC_MP4_Full::processData() {
	bool res = processSample();
	if (!res) {
		return false;
	}
	while (processSample()) {
	}
	return true;
}

bool GPAC_MP4_Full::process(std::shared_ptr<Data> data) {
#if 0 //TODO: zero copy mode, or at least improve the current system
	reader->valid_data_size = reader->dataSize = data->size();
	reader->data.data() = data->data();
#else
	const int currSize = reader->data.size();
	reader->data.resize(reader->data.size() + data->size());
	memcpy(reader->data.data() + currSize, data->data(), data->size());
#endif
	std::stringstream ss;
	ss << "gmem://" << reader->data.size() << "@" << (void*)reader->data.data();
	strcpy(reader->data_url, ss.str().c_str());

	if (!reader->movie) {
		if (!openData()) {
			return false;
		}
	} else {
		if (!updateData()) {
			return false;
		}
	}

	return processData();
}

bool GPAC_MP4_Full::handles(const std::string &url) {
	return GPAC_MP4_Full::canHandle(url);
}

bool GPAC_MP4_Full::canHandle(const std::string &url) {
	if (url.find_last_of("mp4") + 1 == url.size()) {
		return true;
	} else {
		return false;
	}
}
