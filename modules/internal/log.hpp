#pragma once

#include <ostream>


class Log {
public:
	enum Level {
		Error = 0,
		Warning,
		Info,
		Debug
	};

	static std::ostream& get(Level level);

private:
	Log();
	~Log();
	static Level globalLevel;
};
