#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include "optionparser/optionparser.h"
#include "options.hpp"


namespace {

struct Arg : public option::Arg
{
	static void printError(const char* msg1, const option::Option& opt, const char* msg2)
	{
		fprintf(stderr, "%s", msg1);
		fwrite(opt.name, opt.namelen, 1, stderr);
		fprintf(stderr, "%s", msg2);
	}

	static option::ArgStatus Unknown(const option::Option& option, bool msg)
	{
		if (msg) printError("Unknown option '", option, "'\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus Required(const option::Option& option, bool msg)
	{
		if (option.arg != 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires an argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
	{
		if (option.arg != 0 && option.arg[0] != 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a non-empty argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus Numeric(const option::Option& option, bool msg)
	{
		char* endptr = 0;
		if (option.arg != 0 && strtol(option.arg, &endptr, 10)){};
		if (endptr != option.arg && *endptr == 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a numeric argument\n");
		return option::ARG_ILLEGAL;
	}
};

enum optionIndex { UNKNOWN, HELP, OPTIONAL, REQUIRED, NUMERIC, NONEMPTY };
const option::Descriptor usage[] = {
		{ UNKNOWN, 0, "", "", Arg::Unknown, "Usage: dashcastx [options] <URL>\n\n"
		"Options:" },
		{ HELP, 0, "h", "help", Arg::None, "  \t--help, -h  \tPrint usage and exit." },
		{ OPTIONAL, 0, "live", "live", Arg::None, "  --live, -live \tRun at system clock pace (otherwise runs as fast as possible)." },
		{ NUMERIC, 0, "s", "seg-dur", Arg::Numeric, "  --seg-dur, -s \tSet the segment duration in millisecond (default value: 2000)." },
		{ UNKNOWN, 0, "", "", Arg::None,
		"\nExamples:\n"
		"  dashcastx file.ts\n"
		"  dashcastx udp://226.0.0.1:1234\n"
		"  dashcastx -live -s 10000 file.mp4\n"
		"  dashcastx --live --seg-dur 10000 http://server.com/file.mp4\n"
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
	}

	if (options[HELP] || argc == 0) {
		option::printUsage(std::cout, usage);
	} else {
		printDetectedOptions(parse, options.get());
	}

	dashcastXOptions opt;
	opt.url = parse.nonOption(0);
	if (options[OPTIONAL].first()->desc && options[OPTIONAL].first()->desc->shortopt == std::string("l"))
		opt.isLive = true;
	if (options[NUMERIC].first()->desc->shortopt != std::string("s")) throw std::runtime_error("Internal error when parsing '-s'. Please contact your reseller.");
	opt.segmentDuration = atol(options[NUMERIC].first()->arg);

	return opt;
}
