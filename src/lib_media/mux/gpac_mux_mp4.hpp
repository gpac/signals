#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/libav.hpp"
#include <string>

typedef struct __tag_isom GF_ISOFile;
namespace gpacpp {
class IsoSample;
}

namespace Modules {
namespace Mux {

class GPACMuxMP4 : public ModuleDynI {
	public:
		GPACMuxMP4(const std::string &baseName, uint64_t chunkDurationInMs = 0, bool useSegments = false);
		~GPACMuxMP4();
		void process() override;
		void flush() override;

	private:
		void declareStream(Data stream);
		void declareStreamVideo(std::shared_ptr<const MetadataPktLibavVideo> stream);
		void declareStreamAudio(std::shared_ptr<const MetadataPktLibavAudio> stream);
		void sendOutput();
		void addSample(gpacpp::IsoSample &sample, const uint64_t dataDuration);

		GF_ISOFile *m_iso;
		uint32_t m_trackId;
		uint64_t m_DTS;

		//fragments
		void setupFragments();
		bool m_useFragments;
		uint64_t m_curFragDur;

		//segments
		void closeSegment(bool isLastSeg);
		bool m_useSegments;
		uint64_t m_chunkDuration, m_chunkNum;
		std::string m_chunkName;

		OutputDataDefault<DataAVPacket>* output;
		union {
			unsigned int resolution[2];
			unsigned int sampleRate;
		};
};

}
}
