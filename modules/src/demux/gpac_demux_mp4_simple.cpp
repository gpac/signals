#include "gpac_demux_mp4_simple.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"

#include "gpacpp.hpp"

namespace Demux {

class ISOFileReader {
	public:
		void init(GF_ISOFile* m) {
			movie.reset(new gpacpp::IsoFile(m));
			u32 trackId = movie->getTrackId(1); //FIXME should be a parameter? hence not processed in create() but in a stateful process? or a control module?
			trackNumber = movie->getTrackById(trackId);
			sampleCount = movie->getSampleCount(trackNumber);
			sampleIndex = 1;
		}

		std::unique_ptr<gpacpp::IsoFile> movie;
		uint32_t trackNumber;
		uint32_t sampleIndex, sampleCount;
};


GPACDemuxMP4Simple* GPACDemuxMP4Simple::create(std::string const& fn) {
	/* The ISO progressive reader */
	GF_ISOFile *movie;
	/* Number of bytes required to finish the current ISO Box reading */
	u64 missingBytes;

	GF_Err e = gf_isom_open_progressive(fn.c_str(), 0, 0, &movie, &missingBytes);
	if ((e != GF_OK && e != GF_ISOM_INCOMPLETE_FILE) || movie == NULL) {
		Log::msg(Log::Error, "Could not open file %s for reading (%s).", fn, gf_error_to_string(e));
		throw std::runtime_error("File not found");
	}

	return new GPACDemuxMP4Simple(movie);
}

GPACDemuxMP4Simple::GPACDemuxMP4Simple(GF_ISOFile *movie)
	: reader(new ISOFileReader) {
	reader->init(movie);
	output = addPin(new PinDefault);
}

GPACDemuxMP4Simple::~GPACDemuxMP4Simple() {
}

void GPACDemuxMP4Simple::process(std::shared_ptr<const Data> /*data*/) {
	for (;;) {
		try {
			int sampleDescriptionIndex;
			std::unique_ptr<gpacpp::IsoSample> ISOSample;
			ISOSample = reader->movie->getSample(reader->trackNumber, reader->sampleIndex, sampleDescriptionIndex);

			Log::msg(Log::Debug, "Found sample #%s/%s of length %s, RAP %s, DTS: %s, CTS: %s",
			         reader->sampleIndex,
			         reader->sampleCount,
			         ISOSample->dataLength,
			         ISOSample->IsRAP,
			         ISOSample->DTS,
			         ISOSample->DTS + ISOSample->CTS_Offset);
			reader->sampleIndex++;

			auto out = output->getBuffer(ISOSample->dataLength);
			memcpy(out->data(), ISOSample->data, ISOSample->dataLength);
			pins[0]->emit(out);
		} catch (gpacpp::Error const& err) {
			if (err.error_ == GF_ISOM_INCOMPLETE_FILE) {
				u64 missingBytes = reader->movie->getMissingBytes(reader->trackNumber);
				Log::msg(Log::Error, "Missing %s bytes on input file", missingBytes);
			} else {
				return;
			}
		}
	}
}

}
