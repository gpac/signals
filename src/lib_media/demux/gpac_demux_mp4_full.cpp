#include "gpac_demux_mp4_full.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include <string>
#include <sstream>

#include "lib_gpacpp/gpacpp.hpp"

namespace Demux {

//TODO: set appropriate CCO credits
//see http://sourceforge.net/p/gpac/code/HEAD/tree/trunk/gpac/applications/testapps/fmp4demux/main.c
class ISOProgressiveReader {
public:
	ISOProgressiveReader()
		: data(0), refreshBoxes(GF_TRUE), samplesProcessed(0), sampleIndex(1), sampleCount(0), trackNumber(1) {
	}

	~ISOProgressiveReader() {
	}

	/* data buffer to be read by the parser */
	std::vector<u8> data;
	/* URL used to pass a buffer to the parser */
	std::string dataUrl;
	/* The ISO file structure created for the parsing of data */
	std::unique_ptr<gpacpp::IsoFile> movie;
	/* Boolean state to indicate if the needs to be parsed */
	Bool refreshBoxes;
	u32 samplesProcessed;
	u32 sampleIndex; /* samples are numbered starting from 1 */
	u32 sampleCount;
	int trackNumber; //TODO: multi-tracks
};

GPACDemuxMP4Full::GPACDemuxMP4Full()
: reader(new ISOProgressiveReader) {
	output = addOutput(new OutputDefault);
}

GPACDemuxMP4Full::~GPACDemuxMP4Full() {
}

bool GPACDemuxMP4Full::openData() {
	/* if the file is not yet opened (no movie), open it in progressive mode (to update its data later on) */
	u64 missingBytes;
	GF_ISOFile *movie;
	GF_Err e = gf_isom_open_progressive(reader->dataUrl.c_str(), 0, 0, &movie, &missingBytes);
	if ((e != GF_OK && e != GF_ISOM_INCOMPLETE_FILE) || reader->movie) {
		Log::msg(Log::Warning, "Error opening fragmented mp4 in progressive mode: %s (missing %s bytes)",
		         gf_error_to_string(e), missingBytes);

		return false;
	}
	reader->movie.reset(new gpacpp::IsoFile(movie));
	reader->movie->setSingleMoofMode(true);
	return true;
}

bool GPACDemuxMP4Full::updateData() {
	/* let inform the parser that the buffer has been updated with new data */
	uint64_t missingBytes;
	reader->movie->refreshFragmented(missingBytes, reader->dataUrl);
	return true;
}

bool GPACDemuxMP4Full::processSample() {
	try {
		/* only if we have the track number can we try to get the sample data */
		if (reader->trackNumber != 0) {
			u32 newSampleCount;

			/* let's see how many samples we have since the last parsed */
			newSampleCount = reader->movie->getSampleCount(reader->trackNumber);
			if (newSampleCount > reader->sampleCount) {
				/* New samples have been added to the file */
				Log::msg(Log::Info, "Found %s new samples (total: %s)",
				         newSampleCount - reader->sampleCount,
				         newSampleCount);
				if (reader->sampleCount == 0) {
					reader->sampleCount = newSampleCount;
				}
			}
			if (reader->sampleCount == 0) {
				/* no sample yet, let the data input force a reparsing of the data */
				reader->refreshBoxes = GF_TRUE;
			} else {
				/* we have some samples, lets keep things stable in the parser for now and
					 don't let the data input force a reparsing of the data */
				reader->refreshBoxes = GF_FALSE;

				{
					/* let's analyze the samples we have parsed so far one by one */
					int di /*descriptor index*/;
					auto ISOSample = reader->movie->getSample(reader->trackNumber, reader->sampleIndex, di);
					/* if you want the sample description data, you can call:
						 GF_Descriptor *desc = movie->getDecoderConfig(reader->track_handle, di);
						 */

					reader->samplesProcessed++;
					/*here we dump some sample info: samp->data, samp->dataLength, samp->isRAP, samp->DTS, samp->CTS_Offset */
					Log::msg(Log::Info,
					         "Found sample #%s(#%s) of length %s , RAP: %s, DTS : %s, CTS : %s",
					         reader->sampleIndex,
					         reader->samplesProcessed,
					         ISOSample->dataLength,
					         ISOSample->IsRAP,
					         ISOSample->DTS,
					         ISOSample->DTS + ISOSample->CTS_Offset
					        );
					reader->sampleIndex++;

					auto out = output->getBuffer(ISOSample->dataLength);
					memcpy(out->data(), ISOSample->data, ISOSample->dataLength);
					output->emit(out);
				}

				/* once we have read all the samples, we can release some data and force a reparse of the input buffer */
				if (reader->sampleIndex > reader->sampleCount) {
					u64 newBufferStart;
					u64 missingBytes;

					Log::msg(Log::Debug, "Releasing unnecessary buffers");
					/* release internal structures associated with the samples read so far */
					reader->movie->resetTables(true);

					/* release the associated input data as well */
					reader->movie->resetDataOffset(newBufferStart);
					if (newBufferStart) {
						u32 offset = (u32)newBufferStart;
						const size_t newSize = reader->data.size() - offset;
						memmove(reader->data.data(), reader->data.data() + offset, newSize);
						reader->data.resize(newSize);
					}
					std::stringstream ss;
					ss << "gmem://" << reader->data.size() << "@" << (void*)reader->data.data();
					reader->dataUrl = ss.str();
					reader->movie->refreshFragmented(missingBytes, reader->dataUrl);

					/* update the sample count and sample index */
					reader->sampleCount = newSampleCount - reader->sampleCount;
					//FIXME: doesn't happen because the size of the chunck is not aligned on samples: assert(reader->sampleCount == 0);
					reader->sampleIndex = 1;
				}
			}
		}

		return true;
	} catch(gpacpp::Error const& e) {
		Log::msg(Log::Warning, "Could not get sample: %s", gf_error_to_string(e.error_));
		return false;
	}
}

bool GPACDemuxMP4Full::processData() {
	bool res = processSample();
	if (!res) {
		return false;
	}
	while (processSample()) {
	}
	return true;
}

void GPACDemuxMP4Full::process(std::shared_ptr<const Data> data_) {
#if 0 //TODO: zero copy mode, or at least improve the current system
	reader->validDataSize = reader->dataSize = data->size();
	reader->data.data() = data->data();
#else
	auto data = safe_cast<const RawData>(data_);
	const size_t currSize = reader->data.size();
	reader->data.resize(reader->data.size() + (size_t)data->size());
	memcpy(reader->data.data() + currSize, data->data(), (size_t)data->size());
#endif
	std::stringstream ss;
	ss << "gmem://" << reader->data.size() << "@" << (void*)reader->data.data();
	reader->dataUrl = ss.str();

	if (!reader->movie) {
		if (!openData()) {
			return;
		}
	} else {
		if (!updateData()) {
			return;
		}
	}

	processData();
}

}
