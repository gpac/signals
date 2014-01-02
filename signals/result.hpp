#pragma once

#include <vector>


//FIXME: needs to make a copy of the result value
template<typename Result>
class ResultVector {
public:
	typedef std::vector<Result> ResultValue;

	explicit ResultVector() {
	}

	bool operator() (Result r) {
		results.push_back(r);
		return true;
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

template<typename Result>
class ResultDefault : public ResultVector<Result> {
};
