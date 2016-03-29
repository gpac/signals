#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include "optionparser/optionparser.h"
#include "options.hpp"


namespace {

struct Arg : public option::Arg {
	static void printError(const char* msg1, const option::Option& opt, const char* msg2) {
		fprintf(stderr, "%s", msg1);
		fwrite(opt.name, opt.namelen, 1, stderr);
		fprintf(stderr, "%s", msg2);
	}

	static option::ArgStatus Unknown(const option::Option& option, bool msg) {
		if (msg) printError("Unknown option '", option, "'\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus Required(const option::Option& option, bool msg) {
		if (option.arg != 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires an argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus NonEmpty(const option::Option& option, bool msg) {
		if (option.arg != 0 && option.arg[0] != 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a non-empty argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus Numeric(const option::Option& option, bool msg) {
		char* endptr = 0;
		if (option.arg != 0 && strtol(option.arg, &endptr, 10)) {}
		if (endptr != option.arg && *endptr == 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a numeric argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus Video(const option::Option& option, bool msg) {
		unsigned w, h, bitrate;
		if (option.arg != 0 && sscanf(option.arg, "%ux%u:%u", &w, &h, &bitrate) == 3)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a video (wxh:bitrate) argument\n");
		return option::ARG_ILLEGAL;
	}
};

enum optionIndex { UNKNOWN, HELP, OPT, REQUIRED, NUMERIC, VIDEO, NONEMPTY };
const option::Descriptor usage[] = {
	{ UNKNOWN, 0, "", "", Arg::Unknown, "Usage: dashcastx [options] <URL>\n\n"
	"Options:" },
	{ HELP,    0, "h", "help",    Arg::None,    "  --help,          -h         \tPrint usage and exit." },
	{ OPT,     0, "l", "live",    Arg::None,    "  --live,          -l         \tRun at system clock pace (otherwise runs as fast as possible) with low latency settings (quality may be degraded)." },
	{ NUMERIC, 0, "s", "seg-dur", Arg::Numeric, "  --seg-dur,       -s         \tSet the segment duration (in ms) (default value: 2000)." },
	{ VIDEO,   0, "v", "video",   Arg::Video,   "  --video wxh[:b], -v wxh[:b] \tSet a video resolution and optionally bitrate (enables resize and/or transcoding)." },
	{ UNKNOWN, 0, "",  "",        Arg::None,
	"\nExamples:\n"
	"  dashcastx file.ts\n"
	"  dashcastx udp://226.0.0.1:1234?fifo_size=1000000&overrun_nonfatal=1\n"
	"  dashcastx -l -s 10000 file.mp4\n"
	"  dashcastx --live --seg-dur 10000 --video 320x180:50000 -v 640x360:300000 http://server.com/file.mp4\n"
	"  dashcastx --live -v 1280x720:100000 webcam:video=/dev/video0:audio=/dev/audio1\n"
	},
	{ 0, 0, 0, 0, 0, 0 } };

void printDetectedOptions(option::Parser &parse, option::Option * const options) {
	if (parse.nonOptionsCount() == 1) {
		std::cout << "URL: " << parse.nonOption(0) << std::endl;
	} else {
		std::cout << "Several URLs detected: " << std::endl;
		for (int i = 0; i < parse.nonOptionsCount(); ++i)
			std::cout << "Unknown option: " << parse.nonOption(i) << std::endl;
		throw std::runtime_error("Parse error. Please check message and usage above.");
	}

	for (option::Option* opt = options[NUMERIC]; opt; opt = opt->next())
		std::cout << "Option: " << opt->name << ", value: " << atol(opt->arg) << std::endl;
	for (option::Option* opt = options[VIDEO]; opt; opt = opt->next())
		std::cout << "Option: " << opt->name << ", value: " << opt->arg << std::endl;
	for (option::Option* opt = options[OPT]; opt; opt = opt->next())
		std::cout << "Option: " << opt->name << std::endl;
	std::cout << std::endl;
}

}

dashcastXOptions processArgs(int argc, char const* argv[]) {
	argc -= (argc > 0); argv += (argc > 0);
	option::Stats  stats(usage, argc, argv);
	std::unique_ptr<option::Option[]> options(new option::Option[stats.options_max]);
	std::unique_ptr<option::Option[]>  buffer(new option::Option[stats.buffer_max]);
	option::Parser parse(usage, argc, argv, options.get(), buffer.get());

	if (parse.error()) {
		option::printUsage(std::cout, usage);
		throw std::runtime_error("Parse error. Please check message and usage above.");
	} else if (options[HELP] || argc == 0 || parse.nonOptionsCount() == 0) {
		option::printUsage(std::cout, usage);
		throw std::runtime_error("Please check message and usage above.");
	} else {
		printDetectedOptions(parse, options.get());
	}

	dashcastXOptions opt;
	opt.url = parse.nonOption(0);
	if (options[OPT].first()->desc && options[OPT].first()->desc->shortopt == std::string("l"))
		opt.isLive = true;
	if (options[NUMERIC].first()->desc && options[NUMERIC].first()->desc->shortopt == std::string("s"))
		opt.segmentDuration = atol(options[NUMERIC].first()->arg);
	if (options[VIDEO].first()->desc && options[VIDEO].first()->desc->shortopt == std::string("v")) {
		unsigned w=0, h=0, bitrate=0;
		for (option::Option* o = options[VIDEO]; o; o = o->next()) {
			auto const parsed = sscanf(o->arg, "%ux%u:%u", &w, &h, &bitrate);
			if (parsed < 2) /*bitrate is optional*/
				throw std::runtime_error("Internal error while retrieving resolution.");
			opt.v.push_back(Video(Modules::Resolution(w, h), bitrate));
		}
	}

	if (!opt.v.size())
		opt.v.push_back(Video(Modules::VIDEO_RESOLUTION, 300000));

	return opt;
}
