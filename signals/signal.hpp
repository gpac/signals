#pragma once

#include "result.hpp"
#include "protosignal.hpp"


template <typename SignalSignature, typename Result = ResultVector<typename std::function<SignalSignature>::result_type>, typename Caller = CallerSync<Result, SignalSignature>>
class Signal : public ProtoSignal<Result, SignalSignature, Caller> {
	typedef typename ProtoSignal<Result, SignalSignature, Caller>::Callback Callback;

	class Connector {
		friend class Signal;

	public:
		size_t connect(const Callback &cb) {
			return signal.connect(cb);
		}

		bool disconnect(size_t connectionId) {
			return signal.disconnect(connectionId);
		}

	private:
		Connector& operator= (const Connector&) = delete;
		explicit Connector(Signal &signal) : signal(signal) {
		}

		Signal &signal;
	};

public:	
	Signal(const Callback &callback = Callback()) : ProtoSignal(callback) {
	}
};
