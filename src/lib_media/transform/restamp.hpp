#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
	namespace Transform {

		/* /!\ FIXME: modifies the data in-place: use this module synchronously */
		class Restamp : public ModuleS {
		public:
			enum Mode {
				Passthru,    /*offset only*/
				Reset,       /*set the first received timestamp to 0 - aside from the offsetIn180k param*/
				ClockSystem, /*the system clock: starts at 0 on first packet*/
			};

			/*offset will be added to the current time*/
			Restamp(Mode mode, int64_t offsetIn180k = 0);
			~Restamp();
			void process(Data data) override;

		private:
			int64_t offset;
			Mode mode;
			bool isInitTime = false;
		};

	}
}
