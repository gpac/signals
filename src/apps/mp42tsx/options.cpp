#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include "optionparser/optionparser.h"
#include "options.hpp"


mp42tsXOptions processArgs(int argc, char const* argv[]) {
	mp42tsXOptions opt;
	opt.url = "input.mp4"; //FIXME: hardcoded
	return opt;
}
