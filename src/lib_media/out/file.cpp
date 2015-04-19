#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "file.hpp"

namespace Modules {
namespace Out {

File::File(std::string const& path) {
	file = fopen(path.c_str(), "wb");
	if (!file) {
		Log::msg(Log::Error, "Can't open file for writing: %s", path);
		throw std::runtime_error("File not found");
	}
	auto input = addInput(new Input<RawData>(this));
}

File::~File() {
	fclose(file);
}

void File::process(Data data_) {
	auto data = safe_cast<const RawData>(data_);
	fwrite(data->data(), 1, (size_t)data->size(), file);
}

}
}
