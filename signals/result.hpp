#pragma once


//get the last value when multiple calls //TODO: alt implem with a vector of results?
template<typename Result>
class ResultLast {
public:
	typedef Result ResultValue;

	explicit ResultLast() : last() {
	}

	bool operator() (Result r) {
		last = r;
		return true;
	}

	Result result() {
		return last;
	}

private:
	Result last;
};

template<typename Result>
class ResultDefault : public ResultLast<Result> {
};
