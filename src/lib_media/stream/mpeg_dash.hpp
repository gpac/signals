#pragma once

#include "lib_modules/core/module.hpp"
#include "lib_gpacpp/gpacpp.hpp"

namespace Modules {
namespace Stream {

class MPEG_DASH : public ModuleDynI, public gpacpp::Init {
	public:
		enum Type {
			Live,
			Static
		};

		MPEG_DASH(const std::string &mpdPath, Type type, uint64_t segDurationInMs);
		~MPEG_DASH();
		void process() override;
		void flush() override;

	private:
		void DASHThread();
		void endOfStream();
		int numDataQueueNotify = 0;
		std::thread workingThread;

		void generateMPD();
		void ensureMPD();
		std::string mpdPath;
		Type type;
		uint64_t segDurationInMs, totalDurationInMs;

		struct Quality {
			Quality() : meta(nullptr), bitrate_in_bps(0), rep(nullptr) {}
			std::shared_ptr<const MetadataFile> meta;
			double bitrate_in_bps;
			GF_MPD_Representation *rep;
		};
		std::vector<Quality> qualities;

		std::unique_ptr<gpacpp::MPD> mpd;
};

}
}
