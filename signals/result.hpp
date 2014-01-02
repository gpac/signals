#pragma once

#include <vector>


//get the last value when multiple calls
template<typename Result>
class ResultLast {
public:
	typedef Result ResultValue;

	explicit ResultLast() {
	}

	bool operator() (Result r) {
		last = r;
		return true;
	}

	Result get() {
		return last;
	}

private:
	Result last;
};

//FIXME: needs to make a copy of the value
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

private:
	std::vector<Result> results;
};

template<typename Result>
class ResultDefault : public ResultVector<Result> {
};
