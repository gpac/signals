#include "log.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <wincon.h>
static HANDLE console = NULL;
static WORD console_attr_ori = 0;
#else /*_WIN32*/
#define RED    "\x1b[31m"
#define YELLOW "\x1b[33m"
#define GREEN  "\x1b[32m"
#define CYAN   "\x1b[36m"
#define WHITE  "\x1b[37m"
#define RESET  "\x1b[0m"
#endif /*_WIN32*/

Log::Level Log::globalLogLevel = Log::Info;


namespace {

static std::chrono::time_point<std::chrono::high_resolution_clock> const m_Start = std::chrono::high_resolution_clock::now();

uint64_t now() {
	auto const timeNow = std::chrono::high_resolution_clock::now();
	auto const timeNowInMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - m_Start);
	return timeNowInMs.count();
}

}


std::ostream& Log::get(Level /*level*/) {
	return std::cerr;
}

std::string Log::getTime() {
	return format("[%s] ", now()/1000.0);
}

std::string Log::getColorBegin(Level level) {
#ifdef _WIN32
	if (console == NULL) {
		CONSOLE_SCREEN_BUFFER_INFO console_info;
		console = GetStdHandle(STD_ERROR_HANDLE);
		assert(console != INVALID_HANDLE_VALUE);
		if (console != INVALID_HANDLE_VALUE) {
			GetConsoleScreenBufferInfo(console, &console_info);
			console_attr_ori = console_info.wAttributes;
		}
	}
	switch (level) {
	case Error: SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_INTENSITY); break;
	case Warning: SetConsoleTextAttribute(console, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN); break;
	case Info: SetConsoleTextAttribute(console, FOREGROUND_INTENSITY | FOREGROUND_GREEN); break;
	case Debug: SetConsoleTextAttribute(console, FOREGROUND_GREEN); break;
	default: break;
	}
#else
	switch (level) {
	case Error: fprintf(stderr, RED); break;
	case Warning: fprintf(stderr, YELLOW); break;
	case Info: fprintf(stderr, GREEN); break;
	case Debug: fprintf(stderr, CYAN); break;
	default: break;
	}
#endif
	return "";
}

std::string Log::getColorEnd(Level level) {
#ifdef _WIN32
	SetConsoleTextAttribute(console, console_attr_ori);
#else
	fprintf(stderr, RESET);
#endif
	return "";
}

void Log::setLevel(Level level) {
	globalLogLevel = level;
}

Log::Level Log::getLevel() {
	return globalLogLevel;
}
