#pragma once

#include <vector>
#include "../utils/queue.hpp"


namespace Signals {

class IResult {
public:
	virtual ~IResult() {
	}
};


template<typename ResultType>
class ResultQueue : public IResult {
public:
	typedef std::shared_ptr<Queue<ResultType>> ResultValue;

	explicit ResultQueue() : results(new Queue<ResultType>()) {
	}

	virtual ~ResultQueue() {
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
class ResultQueue<void> : public IResult {
public:
	typedef std::shared_ptr<void> ResultValue;

	explicit ResultQueue() {
	}

	virtual ~ResultQueue() {
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

	virtual ~ResultVector() {
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

	virtual ~ResultVector() {
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
 * because emit() (which reset results) and results() are called in different threads. Thus would
 * require an external lock to protect the result.
 */
template<typename ResultType>
class ResultLast : public IResult {
public:
	typedef ResultType ResultValue;

	explicit ResultLast() {
	}

	virtual ~ResultLast() {
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
