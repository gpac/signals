#pragma once

#include "caller.hpp"
#include "connection.hpp"

#include <atomic>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <vector>


template<typename, typename> class ProtoSignal;

template<typename Result, typename Callback, typename... Args>
class ProtoSignal<Result, Callback(Args...)> : private CallerSync<Result, Callback(Args...)> {
protected:
	typedef std::function<Callback(Args...)> Callback;

private:
	typedef typename Result::ResultValue ResultValue;
	typedef typename Callback::result_type ResultType;
	typedef typename Connection<Callback> ConnectionType;
	typedef typename std::unordered_map<size_t, ConnectionType*> ConnectionManager;

public:
	size_t connect(const Callback &cb) {
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
		for each (auto cb in callbacks) {
			call(result, cb.second->callback, args...);
		}
		return callbacks.size();
	}

	ResultValue results() {
		return result.get();
	}

protected:
	ProtoSignal(const Callback &cb) : callbacks(Callback()), uid(0) {
		if (cb != NULL) {
			size_t connectionId = uid++;
			callbacks[connectionId] = new ConnectionType(cb, connectionId);
		}
	}

private:
	ProtoSignal(const ProtoSignal&) = delete;
	ProtoSignal& operator= (const ProtoSignal&) = delete;

	ConnectionManager callbacks; //TODO: make an interface for the ConnectionManager to remove code from this class
	std::atomic<size_t> uid;
	Result result;
};
