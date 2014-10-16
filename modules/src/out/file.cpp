#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "file.hpp"

namespace Modules {
namespace Out {

File::File(FILE *file)
	: file(file) {
}

File::~File() {
	fclose(file);
}

File* File::create(std::string const& fn) {
	FILE *f = fopen(fn.c_str(), "wb");
	if (!f) {
		Log::msg(Log::Error, "Can't open file for writing: %s", fn);
		throw std::runtime_error("File not found");
	}
	return new File(f);
}

void File::process(std::shared_ptr<Data> data) {
	fwrite(data->data(), 1, data->size(), file);
}

}
}
