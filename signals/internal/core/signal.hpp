#pragma once

#include "connection.hpp"
#include "result.hpp"
#include "protosignal.hpp"


namespace Signals {

template <typename SignalSignature,
          typename Result = ResultQueueThreadSafe<typename std::function<SignalSignature>::result_type>,
          template<typename> class CallerTemplate = CallerAsync>
class Signal : public ProtoSignal<Result, SignalSignature> {
private:
	typedef CallerTemplate<SignalSignature> Caller;
	typedef typename ProtoSignal<Result, SignalSignature>::CallbackType Callback;

public:
	Signal(const Callback &callback = Callback()) : ProtoSignal<Result, SignalSignature>(callback) {
	}
	Signal(Caller &caller, const Callback &callback = Callback()) : ProtoSignal<Result, SignalSignature>(caller, callback) {
	}
};

}
