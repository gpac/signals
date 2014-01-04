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
		for (auto &cb : callbacks) {
			cb.second->future = caller(cb.second->callback, args...);
		}
		return callbacks.size();
	}

	ResultValue results() {
		Result result;
		for (auto &cb : callbacks) {
			auto &f = std::move(cb.second->future);
			result.set(f.get());
		}
		return result.get();
	}

protected:
	ProtoSignal(const CallbackType &cb) : uid(0) {
		if (cb != NULL) {
			size_t connectionId = uid++;
			callbacks[connectionId] = new ConnectionType(cb, connectionId);
		}
	}

	~ProtoSignal() {
		//delete still connected callbacks
		for (auto &cb : callbacks) {
			auto id = cb.first;
			if (callbacks[id] != nullptr) {
				bool res = disconnect(id);
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
};
