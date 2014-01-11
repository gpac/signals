#pragma once

#include "internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <string>


class EXPORT File : public Module {
public:
	static File* create(const Param &p);
	~File();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	File(FILE *file);

	FILE *file;
};
