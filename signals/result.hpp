#pragma once

#include <vector>


template<typename Result>
class ResultVector {
public:
	typedef std::vector<Result> ResultValue;

	explicit ResultVector() {
	}

	void set(Result r) {
		results.push_back(r);
	}

	std::vector<Result>& get() {
		return results;
	}

	void clear() {
		results.clear();
	}

private:
	std::vector<Result> results;
};
