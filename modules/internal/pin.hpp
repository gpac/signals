#pragma once

#include "data.hpp"
#include <../signals/signals.hpp>


#if 0
template <typename ResultValue, typename... Args>
class EXPORT IPin {
public:
	//TODO: evaluate for change: interface is taken from ProtoSignal
	virtual size_t emit(Args... args);

	Signal<ResultValue(Args...)> signal;
};
#endif

class EXPORT Pin {//FIXME? but not exportable : public IPin<bool, Data*> {
public:
	size_t emit(Data *data) {
		size_t numReceivers = signal.emit(data);
		assert(numReceivers == 1); //FIXME: we eed a smart_ptr to count them all
		return numReceivers;
	}

	Signal<bool(Data*), ResultVector<bool>, CallerSync<bool(Data*)>, ConnectionQueue<bool(Data*), bool>> signal; //FIXME: forced to sync
};
