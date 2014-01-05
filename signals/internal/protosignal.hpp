#pragma once

#include "caller.hpp"
#include "connection.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <functional>
#include <map>


template<typename, typename, typename> class ProtoSignal;

template<typename Result, typename Callback, typename... Args, typename Caller>
class ProtoSignal<Result, Callback(Args...), Caller> {
protected:
	typedef std::function<Callback(Args...)> CallbackType;

private:
	typedef typename Result::ResultValue ResultValue;
	typedef typename CallbackType::result_type ResultType;
	typedef Connection<CallbackType, ResultType> ConnectionType;
	typedef std::map<size_t, ConnectionType*> ConnectionManager;

public:
	size_t connect(const CallbackType &cb) {
		size_t connectionId = uid++;
		callbacks[connectionId] = new ConnectionType(cb, connectionId);
		return connectionId;
	}

	bool disconnect(size_t connectionId) {
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
		for (auto &cb : callbacks) {
			if (cb.second) {
				cb.second->futures.push_back(caller(cb.second->callback, args...));
			}
		}
		return callbacks.size();
	}

	ResultValue results() {
		for (auto &cb : callbacks) {
			if (cb.second) {
				for (auto &f : cb.second->futures) {
					result.set(std::move(f).get());
				}
				cb.second->futures.clear(); //FIXME: this may delete the future queue while new elements are added
			}
		}
		return result.get();
	}

protected:
	ProtoSignal(const CallbackType &cb) : uid(0) {
		if (cb != nullptr) {
			size_t connectionId = uid++;
			callbacks[connectionId] = new ConnectionType(cb, connectionId);
		}
	}

	~ProtoSignal() {
		Result result;
		for (auto &cb : callbacks) { //delete still connected callbacks
			if (cb.second) {
				for (auto &f : cb.second->futures) {
					result.set(std::move(f).get()); //sync on destroy //TODO: move to the Caller's class responsability to avoid lazy computation?
				}
				bool res = disconnect(cb.first);
				assert(res);
			}
		}
	}

private:
	ProtoSignal(const ProtoSignal&) = delete;
	ProtoSignal& operator= (const ProtoSignal&) = delete;

	ConnectionManager callbacks; //TODO: make an interface for the ConnectionManager to remove code from this class
	std::atomic<size_t> uid;
	Caller caller;
	Result result;
};
