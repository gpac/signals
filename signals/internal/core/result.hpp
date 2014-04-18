#pragma once

#include <vector>
#include "../utils/queue.hpp"


namespace Signals {

class IResult {
};


template<typename ResultType>
class ResultQueueThreadSafe : public IResult {
public:
	typedef std::shared_ptr<QueueThreadSafe<ResultType>> ResultValue;

	explicit ResultQueueThreadSafe() : results(new QueueThreadSafe<ResultType>()) {
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
class ResultQueueThreadSafe<void> : public IResult {
public:
	typedef std::shared_ptr<void> ResultValue;

	explicit ResultQueueThreadSafe() {
	}

	void set(int) {
	}

	std::shared_ptr<void> get() {
		return std::shared_ptr<void>();
	}

	void clear() {
	}
};

template<typename ResultType>
class ResultVector : public IResult {
public:
	typedef std::shared_ptr<std::vector<ResultType>> ResultValue;

	explicit ResultVector() : results(new std::vector<ResultType>()) {
	}

	void set(ResultType r) {
		results->push_back(r);
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
class ResultVector<void> : public IResult {
public:
	typedef std::shared_ptr<void> ResultValue;

	explicit ResultVector()  {
	}

	void set(int) {
	}

	std::shared_ptr<void> get() {
		return std::shared_ptr<void>();
	}

	void clear() {
	}
};

/**
 * A class which gets a copy from the last result. We don't want a shared_ptr to result in somes cases,
 * because emit() (which reset results) an results() are called in different threads. Thus would
 * require an external lock to protect the result.
 */
template<typename ResultType>
class ResultLast : public IResult {
public:
	typedef ResultType ResultValue;
	explicit ResultLast() {
	}
	void set(ResultType r) {
		last = r;
	}
	ResultValue& get() {
		return last;
	}
	void clear() {
	}

private:
	ResultType last;
};

}
