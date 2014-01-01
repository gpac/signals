#pragma once

#include <cassert>


template<typename Callback>
class Connection {
public:
	Callback callback;
	size_t uid;

	explicit Connection(const Callback &callback, const size_t uid) : callback(callback), uid(uid) {
	}

	~Connection() {
	}
};
