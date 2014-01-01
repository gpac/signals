#pragma once

#include "connection.hpp"

#include <atomic>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <vector>


template<typename, typename> class ProtoSignal;

template<typename Result, typename R, typename... Args>
class ProtoSignal<R(Args...), Result> {
protected:
	typedef std::function<R(Args...)> Callback;

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

	ResultValue emit(Args... args) {
		Result result; //TODO: test with an alternate collector
		for each (auto cb in callbacks) {
			result(cb.second->callback(args...));
		}
		return result.result();
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
};
