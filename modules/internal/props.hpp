#pragma once

#include "config.hpp"

namespace Modules {
	/**
	* A generic property container. Help modules communicate with each other.
	*/
	class MODULES_EXPORT IProps {
	};

	class MODULES_EXPORT Props : public IProps {
	public:
		Props() {
		}
		virtual ~Props() {
		}
	};
}
