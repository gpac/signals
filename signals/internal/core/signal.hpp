#pragma once

#include "executor.hpp"
#include "connection.hpp"
#include "result.hpp"
#include "protosignal.hpp"


namespace Signals {

template <typename SignalSignature,
          typename Result = ResultQueueThreadSafe<typename std::function<SignalSignature>::result_type>>
class Signal : public ProtoSignal<Result, SignalSignature> {
private:
	typedef typename ProtoSignal<Result, SignalSignature>::CallbackType Callback;

public:
	Signal() : ProtoSignal<Result, SignalSignature>() {
	}
	Signal(IExecutor<SignalSignature> &executor) : ProtoSignal<Result, SignalSignature>(executor) {
	}
};

}
