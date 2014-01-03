#pragma once

#include <vector>


template<typename ResultType>
class ResultVector {
public:
	typedef std::vector<ResultType> ResultValue;

	explicit ResultVector() {
	}

	void set(ResultType r) {
		results.push_back(r);
	}

	std::vector<ResultType>& get() {
		return results;
	}

	void clear() {
		results.clear();
	}

private:
	std::vector<ResultType> results;
};

template<>
class ResultVector<void> {
public:
	typedef std::vector<void> ResultValue;

	explicit ResultVector() {
	}

	void set() {
	}

	void get() {
	}

	void clear() {
	}
};
