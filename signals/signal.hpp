#pragma once

#include "result.hpp"
#include "protosignal.hpp"


template <typename SignalSignature, typename Result = ResultDefault<typename std::function<SignalSignature>::result_type>>
class Signal : public ProtoSignal<SignalSignature, Result> {
	typedef typename ProtoSignal<SignalSignature, Result>::Callback Callback;

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
	Signal(const Callback &method = Callback()) : ProtoSignal(method) {
	}
};
