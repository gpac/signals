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
