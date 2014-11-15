#include "../../tests/tests.hpp"
#include "modules.hpp"

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

#include "decode/libav_decode.hpp"
#include "demux/libav_demux.hpp"
#include "encode/libav_encode.hpp"
#include "mux/gpac_mux_mp4.hpp"
#include "stream/mpeg_dash.hpp"
#include "transform/audio_convert.hpp"
#include "transform/video_convert.hpp"
#include "out/null.hpp"

#include "../../utils/tools.hpp"

#include <atomic>
#include <sstream>

using namespace Tests;
using namespace Modules;


namespace {

class EOSData : public Data {
public:
	virtual uint8_t* data() override {
		return nullptr;
	}
	virtual uint64_t size() const override {
		return 0;
	}
	virtual void resize(size_t size) override {
		assert(0);
	}
};


#define EXECUTOR_SYNC ExecutorSync<void(std::shared_ptr<Data>)>
#define EXECUTOR_ASYNC StrandedPoolModuleExecutor
#define EXECUTOR EXECUTOR_ASYNC

struct ICompletionNotifier {
	virtual void finished() = 0;
};

class PipelinedModule {
public:
	/* take ownership of module */
	PipelinedModule(Module *module, ICompletionNotifier *notify) : type(None), delegate(module), localExecutor(new EXECUTOR), executor(*localExecutor), m_notify(notify) {
	}

	void connect(IPin* pin) {
		ConnectToModule(pin->getSignal(), this, executor);
	}

	/* Receiving nullptr stops the execution */
	void process(std::shared_ptr<Data> data) {
		if (data) {
			delegate->process(data);
		} else {
			endOfStream();
		}
	}

	void dispatch(std::shared_ptr<Data> data) {
		executor(MEMBER_FUNCTOR(this, &PipelinedModule::process), data);
	}

	/* source modules are stopped manually - then the message propagates to other connected modules */
	void setSource(bool isSource) {
		type = isSource ? Source : None;
	}

	bool isSource() const {
		return type == Source;
	}

	bool isSink() const {
		return delegate->getNumPin() == 0;
	}

	size_t getNumPin() const {
		return delegate->getNumPin();
	}

	IPin* getPin(size_t i) {
		return delegate->getPin(i);
	}

private:
	void endOfStream() {
		delegate->flush();
		if (isSink()) {
			m_notify->finished();
		} else {
			for (size_t i = 0; i < delegate->getNumPin(); ++i) {
				delegate->getPin(i)->emit(std::shared_ptr<Data>(nullptr));
			}
		}
	}

	enum Type {
		None,
		Source
	};
	Type type;

	std::unique_ptr<Module> delegate;
	std::unique_ptr<IProcessExecutor> const localExecutor;
	IProcessExecutor &executor;
	ICompletionNotifier* const m_notify;
};

class Pipeline : public ICompletionNotifier {
public:
	Pipeline() : numRemainingNotifications(0) {
	}

	void addModule(std::unique_ptr<PipelinedModule> module, bool isSource = false) {
		if (module->isSink())
			numRemainingNotifications++;
		module->setSource(isSource);
		modules.push_back(std::move(module));
	}

	void start() {
		for (auto &m : modules) {
			if (m->isSource())
				m->dispatch(nullptr);
		}
	}

	void waitForCompletion() {
		std::unique_lock<std::mutex> lock(mutex);
		while (numRemainingNotifications > 0) {
			condition.wait(lock);
		}
	}

	virtual void finished() override {
		std::unique_lock<std::mutex> lock(mutex);
		assert(numRemainingNotifications > 0);
		--numRemainingNotifications;
		condition.notify_one();
	}

private:
	std::vector<std::unique_ptr<PipelinedModule>> modules;

	std::mutex mutex;
	std::condition_variable condition;
	std::atomic<int> numRemainingNotifications;
};

Encode::LibavEncode* createEncoder(PropsDecoder *decoderProps) {
	auto const codecType = decoderProps ? decoderProps->getAVCodecContext()->codec_type : AVMEDIA_TYPE_UNKNOWN;
	if (codecType == AVMEDIA_TYPE_VIDEO) {
		Log::msg(Log::Info, "[Encoder] Found video stream");
		return new Encode::LibavEncode(Encode::LibavEncode::Video);
	} else if (codecType == AVMEDIA_TYPE_AUDIO) {
		Log::msg(Log::Info, "[Encoder] Found audio stream");
		return new Encode::LibavEncode(Encode::LibavEncode::Audio);
	} else {
		Log::msg(Log::Info, "[Encoder] Found unknown stream");
		return nullptr;
	}
}

Module* createConverter(PropsDecoder *decoderProps) {
	auto const codecType = decoderProps ? decoderProps->getAVCodecContext()->codec_type : AVMEDIA_TYPE_UNKNOWN;
	if (codecType == AVMEDIA_TYPE_VIDEO) {
		Log::msg(Log::Info, "[Converter] Found video stream");
		auto srcCtx = decoderProps->getAVCodecContext();
		auto srcRes = Resolution(srcCtx->width, srcCtx->height);
		auto dstRes = Resolution(320, 180);
		return new Transform::VideoConvert(srcRes, srcCtx->pix_fmt, dstRes, srcCtx->pix_fmt);
	} else if (codecType == AVMEDIA_TYPE_AUDIO) {
		Log::msg(Log::Info, "[Converter] Found audio stream");
		auto baseFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::F32, AudioStruct::Planar);
		auto otherFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved);
		return new Transform::AudioConvert(baseFormat, otherFormat);
	} else {
		Log::msg(Log::Info, "[Converter] Found unknown stream");
		return nullptr;
	}
}
}


int safeMain(int argc, char const* argv[]) {

	if (argc != 2)
		throw std::runtime_error("usage: dashcastx <URL>");

	auto const inputURL = argv[1];

	Pipeline pipeline;

	auto demux_ = Demux::LibavDemux::create(inputURL);
	auto demux = uptr(new PipelinedModule(demux_, &pipeline));
	pipeline.addModule(std::move(demux), true);

	auto dasher_ = new Modules::Stream::MPEG_DASH(Modules::Stream::MPEG_DASH::Static);
	auto dasher = uptr(new PipelinedModule(dasher_, &pipeline));

	for (size_t i = 0; i < demux_->getNumPin(); ++i) {
		auto props = demux_->getPin(i)->getProps();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		ASSERT(decoderProps);

		auto decoder_ = new Decode::LibavDecode(*decoderProps);
		auto decoder = uptr(new PipelinedModule(decoder_, &pipeline));
		decoder->connect(demux_->getPin(i));

		//FIXME: hardcoded converters
		auto converter_ = createConverter(decoderProps);
		if (!converter_) {
			auto r_ = new Out::Null;
			auto r = uptr(new PipelinedModule(r_, &pipeline));
			r->connect(decoder->getPin(0));
			pipeline.addModule(std::move(decoder));
			pipeline.addModule(std::move(r));
			continue;
		}
		auto converter = uptr(new PipelinedModule(converter_, &pipeline));
		converter->connect(decoder->getPin(0));

		auto encoder_ = createEncoder(decoderProps);
		if (!encoder_) {
			auto r_ = new Out::Null;
			auto r = uptr(new PipelinedModule(r_, &pipeline));
			converter->connect(decoder->getPin(0));
			r->connect(converter->getPin(0));
			pipeline.addModule(std::move(decoder));
			pipeline.addModule(std::move(converter));
			pipeline.addModule(std::move(r));
			continue;
		}
		auto encoder = uptr(new PipelinedModule(encoder_, &pipeline));
		encoder->connect(converter_->getPin(0));

		std::stringstream filename;
		filename << i;
		auto muxer_ = new Mux::GPACMuxMP4(filename.str(), true);
		auto muxer = uptr(new PipelinedModule(muxer_, &pipeline));
		muxer->connect(encoder->getPin(0));

		Connect(encoder_->declareStream, muxer_, &Mux::GPACMuxMP4::declareStream);
		encoder_->sendOutputPinsInfo();

		dasher->connect(muxer->getPin(0));

		pipeline.addModule(std::move(decoder));
		pipeline.addModule(std::move(converter));
		pipeline.addModule(std::move(encoder));
		pipeline.addModule(std::move(muxer));
	}

	pipeline.addModule(std::move(dasher));
		
	pipeline.start();
	pipeline.waitForCompletion();

	return 0;
}

int main(int argc, char const* argv[]) {
	try {
		return safeMain(argc, argv);
	} catch(std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}
