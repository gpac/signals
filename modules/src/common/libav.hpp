#pragma once

#include "../../internal/data.hpp"
#include "../../internal/props.hpp"

struct AVCodecContext;
struct AVPacket;

namespace Modules {
	//FIXME: move in libmm, or add a new namespace
	class MODULES_EXPORT PropsDecoder : public Props {
	public:
		/**
		 * Doesn't take the ownership
		 */
		PropsDecoder(AVCodecContext *codecCtx)
			: codecCtx(codecCtx) {
		}

		AVCodecContext* getAVCodecContext() const {
			return codecCtx;
		}

	private:
		AVCodecContext *codecCtx;
	};

	class MODULES_EXPORT DataDecoder : public Data {
	public:
		DataDecoder();		
		~DataDecoder();
		uint8_t* data();
		uint64_t size() const;
		AVPacket* getPacket() const;
		void resize(size_t size);

	private:
		AVPacket *pkt;
	};
}
