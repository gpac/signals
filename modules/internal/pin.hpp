#pragma once

#include "data.hpp"
#include <../signals/signals.hpp>
#include <vector>


#if 0
template <typename ResultValue, typename... Args>
class EXPORT IPin {
public:
	//TODO: evaluate for change: interface is taken from ProtoSignal
	virtual size_t emit(Args... args);

	Signal<ResultValue(Args...)> signal;
};
#endif

//TODO: this is the sync approach, where data are synced for the Pin to be destroyed.
//      The other option is to invalidate all the data by calling
//TODO: the pin could check the bool result and retry on failure (but is it its role?)
class EXPORT Pin {//FIXME? but not exportable : public IPin<bool, Data*> {
public:
	~Pin() {
		destroy();
	}

	size_t emit(std::shared_ptr<Data> data) {
		size_t numReceivers = signal.emit(data);
		assert(numReceivers == 1);
		return numReceivers;
	}

	void destroy() {
		//getting the result release the shared_ptr, hence the data
		//FIXME: we should lock since emit could be called from any thread
		signal.results();
	}

	//TODO: Signal<bool(std::shared_ptr<Data>)> signal;
	Signal<bool(std::shared_ptr<Data>), ResultVector<bool>, CallerSync, ConnectionQueue<bool(std::shared_ptr<Data>), bool >> signal;
};
