#include "gpac_mp4_full.hpp"
#include "../utils/log.hpp"
#include <string>
#include <sstream>

extern "C" {
#include <gpac/tools.h>
#include <gpac/isomedia.h>
#include <gpac/thread.h>
}

#include "gpacpp.hpp"

namespace Demux {

//TODO: set appropriate CCO credits
//see http://sourceforge.net/p/gpac/code/HEAD/tree/trunk/gpac/applications/testapps/fmp4demux/main.c
class ISOProgressiveReader {
public:
	ISOProgressiveReader()
		: data(0), refresh_boxes(GF_TRUE), samples_processed(0), sample_index(1), sample_count(0), track_number(1) {
	}

	~ISOProgressiveReader() {
	}

	/* data buffer to be read by the parser */
	std::vector<u8> data;
	/* URL used to pass a buffer to the parser */
	std::string data_url;
	/* The ISO file structure created for the parsing of data */
	std::unique_ptr<gpacpp::IsoFile> movie;
	/* Boolean state to indicate if the needs to be parsed */
	Bool refresh_boxes;
	u32 samples_processed;
	u32 sample_index; /* samples are numbered starting from 1 */
	u32 sample_count;
	int track_number; //TODO: multi-tracks
};

GPAC_MP4_Full* GPAC_MP4_Full::create() {
	return new GPAC_MP4_Full();
}

GPAC_MP4_Full::GPAC_MP4_Full()
	: reader(new ISOProgressiveReader) {
	signals.push_back(new Pin<>);
}

GPAC_MP4_Full::~GPAC_MP4_Full() {
	delete signals[0];
}

bool GPAC_MP4_Full::openData() {
	/* if the file is not yet opened (no movie), open it in progressive mode (to update its data later on) */
	u64 missing_bytes;
	GF_ISOFile *movie;
	GF_Err e = gf_isom_open_progressive(reader->data_url.c_str(), 0, 0, &movie, &missing_bytes);
	if ((e != GF_OK && e != GF_ISOM_INCOMPLETE_FILE) || reader->movie) {
		Log::msg(Log::Warning, "Error opening fragmented mp4 in progressive mode: %s (missing %s bytes)",
		         gf_error_to_string(e), missing_bytes);

		return false;
	}
	reader->movie.reset(new gpacpp::IsoFile(movie));
	reader->movie->setSingleMoofMode(true);
	return true;
}

bool GPAC_MP4_Full::updateData() {
	/* let inform the parser that the buffer has been updated with new data */
	uint64_t missing_bytes;
	reader->movie->refreshFragmented(missing_bytes, reader->data_url);
	return true;
}

bool GPAC_MP4_Full::processSample() {
	try {
		/* only if we have the track number can we try to get the sample data */
		if (reader->track_number != 0) {
			u32 new_sample_count;

			/* let's see how many samples we have since the last parsed */
			new_sample_count = reader->movie->getSampleCount(reader->track_number);
			if (new_sample_count > reader->sample_count) {
				/* New samples have been added to the file */
				Log::msg(Log::Info, "Found %s new samples (total: %s)",
				         new_sample_count - reader->sample_count,
				         new_sample_count);
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

				{
					/* let's analyze the samples we have parsed so far one by one */
					int di /*descriptor index*/;
					auto iso_sample = reader->movie->getSample(reader->track_number, reader->sample_index, di);
					/* if you want the sample description data, you can call:
						 GF_Descriptor *desc = movie->getDecoderConfig(reader->track_handle, di);
						 */

					reader->samples_processed++;
					/*here we dump some sample info: samp->data, samp->dataLength, samp->isRAP, samp->DTS, samp->CTS_Offset */
					Log::msg(Log::Info,
					         "Found sample #%s(#%s) of length %s , RAP: %s, DTS : %s, CTS : %s",
					         reader->sample_index,
					         reader->samples_processed,
					         iso_sample->dataLength,
					         iso_sample->IsRAP,
					         iso_sample->DTS,
					         iso_sample->DTS + iso_sample->CTS_Offset
					        );
					reader->sample_index++;

					std::shared_ptr<Data> out(signals[0]->getBuffer(iso_sample->dataLength));
					memcpy(out->data(), iso_sample->data, iso_sample->dataLength);
					signals[0]->emit(out);
				}

				/* once we have read all the samples, we can release some data and force a reparse of the input buffer */
				if (reader->sample_index > reader->sample_count) {
					u64 new_buffer_start;
					u64 missing_bytes;

					Log::msg(Log::Debug, "Releasing unnecessary buffers");
					/* release internal structures associated with the samples read so far */
					reader->movie->resetTables(true);

					/* release the associated input data as well */
					reader->movie->resetDataOffset(new_buffer_start);
					if (new_buffer_start) {
						u32 offset = (u32)new_buffer_start;
						const size_t newSize = reader->data.size() - offset;
						memmove(reader->data.data(), reader->data.data() + offset, newSize);
						reader->data.resize(newSize);
					}
					std::stringstream ss;
					ss << "gmem://" << reader->data.size() << "@" << (void*)reader->data.data();
					reader->data_url = ss.str();
					reader->movie->refreshFragmented(missing_bytes, reader->data_url);

					/* update the sample count and sample index */
					reader->sample_count = new_sample_count - reader->sample_count;
					//FIXME: doesn't happen because the size of the chunck is not aligned on samples: assert(reader->sample_count == 0);
					reader->sample_index = 1;
				}
			}
		}

		return true;
	} catch(gpacpp::Error const& e) {
		Log::msg(Log::Warning, "Could not get sample: %s", gf_error_to_string(e.error_));
		return false;
	}
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
	const size_t currSize = reader->data.size();
	reader->data.resize(reader->data.size() + data->size());
	memcpy(reader->data.data() + currSize, data->data(), data->size());
#endif
	std::stringstream ss;
	ss << "gmem://" << reader->data.size() << "@" << (void*)reader->data.data();
	reader->data_url = ss.str();

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

}
