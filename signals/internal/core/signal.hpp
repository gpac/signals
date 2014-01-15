#pragma once

#include "connection.hpp"
#include "result.hpp"
#include "protosignal.hpp"


template <typename SignalSignature, 
					typename Result = ResultQueueThreadSafe<typename std::function<SignalSignature>::result_type>, 
					template<typename> class CallerTemplate = CallerAsync>
class Signal : public ProtoSignal<Result, SignalSignature, CallerTemplate<SignalSignature>> {
private:
	typedef CallerTemplate<SignalSignature> Caller;
	typedef typename ProtoSignal<Result, SignalSignature, Caller>::CallbackType Callback;

	class Connector {
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
	Signal(const Callback &callback = Callback()) : ProtoSignal<Result, SignalSignature, Caller>(callback) {
	}
	Signal(Caller &caller, const Callback &callback = Callback()) : ProtoSignal<Result, SignalSignature, Caller>(caller, callback) {
	}
};
