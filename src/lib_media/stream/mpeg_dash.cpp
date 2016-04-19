#include "mpeg_dash.hpp"
#include "lib_modules/core/clock.hpp"
#include "lib_utils/tools.hpp"
#include "../out/file.hpp"
#include "../common/libav.hpp"
#include <fstream>


#define MIN_BUFFER_TIME_IN_MS_VOD  3000
#define MIN_BUFFER_TIME_IN_MS_LIVE 1000

#define AVAILABILITY_TIMEOFFSET_IN_S 0.0

namespace Modules {

namespace {
GF_MPD_AdaptationSet *createAS(uint64_t segDurationInMs, GF_MPD_Period *period, gpacpp::MPD *mpd) {
	auto as = mpd->addAdaptationSet(period);
	GF_SAFEALLOC(as->segment_template, GF_MPD_SegmentTemplate);
	as->segment_template->duration = segDurationInMs;
	as->segment_template->timescale = 1000;
	as->segment_template->start_number = 1;
	as->segment_template->availability_time_offset = AVAILABILITY_TIMEOFFSET_IN_S;

	//FIXME: arbitrary: should be set by the app, or computed
	as->segment_alignment = GF_TRUE;
	as->bitstream_switching = GF_TRUE;

	return as;
}
}

namespace Stream {

MPEG_DASH::MPEG_DASH(const std::string &mpdPath, Type type, uint64_t segDurationInMs)
	: mpdPath(mpdPath), type(type), segDurationInMs(segDurationInMs), totalDurationInMs(0),
	  mpd(type == Live ? new gpacpp::MPD(GF_MPD_TYPE_DYNAMIC, MIN_BUFFER_TIME_IN_MS_LIVE)
	  : new gpacpp::MPD(GF_MPD_TYPE_STATIC, MIN_BUFFER_TIME_IN_MS_VOD)) {
	addInput(new Input<DataAVPacket>(this));
}

void MPEG_DASH::endOfStream() {
	if (workingThread.joinable()) {
		for (size_t i = 0; i < inputs.size(); ++i)
			inputs[i]->push(nullptr);
		workingThread.join();
	}
}

MPEG_DASH::~MPEG_DASH() {
	endOfStream();
}

//needed because of the use of system time for live - otherwise awake on data as for any multi-input module
//TODO: add clock to the scheduler, see #14
void MPEG_DASH::DASHThread() {
	log(Info, "start processing at UTC: %s.", gf_net_get_utc());

	Data data;
	for (;;) {
		auto const numInputs = getNumInputs() - 1;
		qualities.resize(numInputs);
		for (size_t i = 0; i < numInputs; ++i) {
			data = inputs[i]->pop();
			if (!data) {
				break;
			} else {
				qualities[i].meta = safe_cast<const MetadataFile>(data->getMetadata());
				if (!qualities[i].meta)
					throw error(format("Unknown data received on input %s", i).c_str());
				auto const numSeg = totalDurationInMs / segDurationInMs;
				qualities[i].bitrate_in_bps = (qualities[i].meta->getSize() * 8 + qualities[i].bitrate_in_bps * numSeg) / (numSeg + 1);
			}
		}
		if (!data)
			break;

		generateMPD();
		if (type == Live) {
			if (!mpd->write(mpdPath))
				log(Warning, "Can't write MPD at %s (1). Check you have sufficient rights.", mpdPath);
		}
		log(Info, "Processes segment (total processed: %ss, UTC: %s (deltaAST=%s).", (double)totalDurationInMs / 1000, gf_net_get_utc(), gf_net_get_utc() - mpd->mpd->availabilityStartTime);

		if (type == Live) {
			auto dur = std::chrono::milliseconds(mpd->mpd->availabilityStartTime + totalDurationInMs - gf_net_get_utc());
			log(Info, "Going to sleep for %s ms.", std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
			std::this_thread::sleep_for(dur);
		}
	}

	/*final rewrite of MPD in static mode*/
	mpd->mpd->type = GF_MPD_TYPE_STATIC;
	mpd->mpd->minimum_update_period = 0;
	mpd->mpd->media_presentation_duration = totalDurationInMs;
	generateMPD();
	if (!mpd->write(mpdPath))
		log(Warning, "Can't write MPD at %s (2). Check you have sufficient rights.", mpdPath);
}

void MPEG_DASH::process() {
	if (!workingThread.joinable()) {
		numDataQueueNotify = (int)getNumInputs() - 1; //FIXME: connection/disconnection cannot occur dynamically. Lock inputs?
		workingThread = std::thread(&MPEG_DASH::DASHThread, this);
	}
}

void  MPEG_DASH::ensureMPD() {
	if (!gf_list_count(mpd->mpd->periods)) {
		mpd->mpd->publishTime = mpd->mpd->availabilityStartTime;

		auto period = mpd->addPeriod();
		period->ID = gf_strdup("p0");
		GF_MPD_AdaptationSet *audioAS = nullptr, *videoAS = nullptr;
		for (size_t i = 0; i < getNumInputs() - 1; ++i) {
			GF_MPD_AdaptationSet *as = nullptr;
			switch (qualities[i].meta->getStreamType()) {
			case AUDIO_PKT: audioAS ? as = audioAS : as = audioAS = createAS(segDurationInMs, period, mpd.get()); break;
			case VIDEO_PKT: videoAS ? as = videoAS : as = videoAS = createAS(segDurationInMs, period, mpd.get()); break;
			default: assert(0);
			}

			auto rep = mpd->addRepresentation(as, format("%s", i).c_str(), (u32)qualities[i].bitrate_in_bps);
			qualities[i].rep = rep;
			GF_SAFEALLOC(rep->segment_template, GF_MPD_SegmentTemplate);
			rep->segment_template->media = gf_strdup(format("%s.mp4_$Number$", i).c_str());
			rep->segment_template->initialization = gf_strdup(format("$RepresentationID$.mp4", i).c_str());
			rep->segment_template->start_number = 1;
			rep->mime_type = gf_strdup(qualities[i].meta->getMimeType().c_str());
			rep->codecs = gf_strdup(qualities[i].meta->getCodecName().c_str());
			rep->starts_with_sap = GF_TRUE;
			switch (qualities[i].meta->getStreamType()) {
			case AUDIO_PKT: rep->samplerate = qualities[i].meta->sampleRate; break;
			case VIDEO_PKT: rep->width = qualities[i].meta->resolution[0]; rep->height = qualities[i].meta->resolution[1]; break;
			default: assert(0);
			}
		}
	}
}

void MPEG_DASH::generateMPD() {
	ensureMPD();
	for (size_t i = 0; i < getNumInputs() - 1; ++i) {
		if (qualities[i].rep->width) { /*video only*/
			qualities[i].rep->starts_with_sap = (qualities[i].rep->starts_with_sap == GF_TRUE && qualities[i].meta->getStartsWithRAP()) ? GF_TRUE : GF_FALSE;
		}
	}
	if (!mpd->mpd->availabilityStartTime) {
		mpd->mpd->availabilityStartTime = gf_net_get_utc() - segDurationInMs;
	}
	totalDurationInMs += segDurationInMs;
}

void MPEG_DASH::flush() {
	numDataQueueNotify--;
	if ((type == Live) && (numDataQueueNotify == 0))
		endOfStream();
}

}
}
