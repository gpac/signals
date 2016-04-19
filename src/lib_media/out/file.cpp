#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "file.hpp"

namespace Modules {
namespace Out {

File::File(std::string const& path) {
	file = fopen(path.c_str(), "wb");
	if (!file)
		throw std::runtime_error(format("Can't open file for writing: %s", path));

	addInput(new Input<DataBase>(this));
}

File::~File() {
	fclose(file);
}

void File::process(Data data_) {
	auto data = safe_cast<const DataBase>(data_);
	fwrite(data->data(), 1, (size_t)data->size(), file);
}

}
}
