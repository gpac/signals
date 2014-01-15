#pragma once

#include "caller.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <functional>
#include <map>
#include <mutex>


template<typename, typename, typename> class ProtoSignal;

template<typename Result, typename Callback, typename... Args, typename Caller>
class ProtoSignal<Result, Callback(Args...), Caller> {
protected:
	typedef std::function<Callback(Args...)> CallbackType;

private:
	typedef typename Result::ResultValue ResultValue;
	typedef typename CallbackType::result_type ResultType;
	typedef ConnectionQueueThreadSafe<CallbackType, ResultType> ConnectionType;
	typedef std::map<size_t, ConnectionType*> ConnectionManager;

public:
	size_t connect(const CallbackType &cb) {
		const size_t connectionId = uid++;
		std::lock_guard<std::mutex> lg(callbacksMutex);
		callbacks[connectionId] = new ConnectionType(cb, connectionId);
		return connectionId;
	}

	bool disconnect(size_t connectionId) {
		std::lock_guard<std::mutex> lg(callbacksMutex);
		if (callbacks[connectionId] != nullptr) {
			delete callbacks[connectionId];
			callbacks[connectionId] = nullptr;
			return true;
		} else {
			return false;
		}
	}

	size_t emit(Args... args) {
		result.clear();
		std::lock_guard<std::mutex> lg(callbacksMutex);
		for (auto &cb : callbacks) {
			if (cb.second) {
				cb.second->futures.push(caller(cb.second->callback, args...));
			}
		}
		return callbacks.size();
	}

	ResultValue results(bool single = false) {
		std::lock_guard<std::mutex> lg(callbacksMutex);
		for (auto &cb : callbacks) {
			if (cb.second) {
				for (;;) {
					auto f = cb.second->futures.tryPop();
					if (f) {
						result.set(std::move(f)->get());
						if (single) {
							break;
						}
					} else {
						break;
					}
				}
				cb.second->futures.clear();
			}
		}
		return result.get();
	}

protected:
	ProtoSignal(const CallbackType &cb) : uid(0) {
		if (cb != nullptr) {
			size_t connectionId = uid++;
			std::lock_guard<std::mutex> lg(callbacksMutex);
			callbacks[connectionId] = new ConnectionType(cb, connectionId);
		}
	}

	ProtoSignal(Caller &caller, const CallbackType &cb) : uid(0), caller(caller) {
		if (cb != nullptr) {
			size_t connectionId = uid++;
			std::lock_guard<std::mutex> lg(callbacksMutex);
			callbacks[connectionId] = new ConnectionType(cb, connectionId);
		}
	}

	~ProtoSignal() {
		Result result;
		std::lock_guard<std::mutex> lg(callbacksMutex);
		for (auto &cb : callbacks) { //delete still connected callbacks
			if (cb.second) {
				for (;;) {
					auto f = cb.second->futures.tryPop();
					if (f) {
						result.set(std::move(f)->get()); //sync on destroy
					} else {
						break;
					}
				}
				bool res = disconnect(cb.first);
				assert(res);
			}
		}
	}

private:
	ProtoSignal(const ProtoSignal&) = delete;
	ProtoSignal& operator= (const ProtoSignal&) = delete;

	ConnectionManager callbacks;
	std::mutex callbacksMutex;
	std::atomic<size_t> uid; //TODO: could be non atomic and protected by callbacksMutex
	Caller caller;
	Result result;
};
