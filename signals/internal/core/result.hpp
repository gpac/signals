#pragma once

#include <memory>
#include <vector>
#include "../utils/queue.hpp"


template<typename ResultType>
class ResultThreadSafeQueue {
public:
	typedef std::shared_ptr<ThreadSafeQueue<ResultType>> ResultValue;

	explicit ResultThreadSafeQueue() : results(new ThreadSafeQueue<ResultType>()) {
	}

	void set(ResultType r) {
		results->push(r);
	}

	ResultValue& get() {
		return results;
	}

	void clear() {
		results->clear();
	}

private:
	ResultValue results;
};

//specialized for void
template<>
class ResultThreadSafeQueue<void> {
public:
	typedef std::shared_ptr<void> ResultValue;

	explicit ResultThreadSafeQueue() {
	}

	void set(int) {
	}

	std::shared_ptr<void> get() {
		return std::shared_ptr<void>();
	}

	void clear() {
	}
};

#if 0 //TODO: write an interface for Result
template<typename ResultType>
class ResultVector {
public:
	typedef std::vector<ResultType> ResultValue;

	explicit ResultVector() {
	}

	void set(ResultType r) {
		results.push_back(r);
	}

	ResultValue& get() {
		return results;
	}

	void clear() {
		results.clear();
	}

private:
	ResultValue results;
};

//specialized for void
template<>
class ResultVector<void> {
public:
	typedef void ResultValue;

	explicit ResultVector() {
	}

	void set(int) {
	}

	void get() {
	}

	void clear() {
	}
};
#endif