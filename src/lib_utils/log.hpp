#pragma once

#include "format.hpp"
#include <ostream>


class Log {
	public:
		enum Level {
			Quiet = -1,
			Error = 0,
			Warning,
			Info,
			Debug
		};

		template<typename... Arguments>
		static void msg(Level level, const std::string& fmt, Arguments... args) {
			if ((level != Quiet) && (level <= globalLogLevel)) {
				get(level) << getColorBegin(level) << format(fmt, args...) << getColorEnd(level) << std::endl;
				get(level).flush();
			}
		}

		void setLevel(Level level);
		Log::Level getLevel();

	private:
		Log();
		~Log();
		static std::ostream& get(Level level);
		static std::string getColorBegin(Level level);
		static std::string getColorEnd(Level level);

		static Level globalLogLevel;
};
