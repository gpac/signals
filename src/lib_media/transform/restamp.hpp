#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
	namespace Transform {

		/* /!\ FIXMe: modifies the data in-place: use this module synchronously */
		class Restamp : public ModuleS {
		public:
			enum Mode {
				Reset,   /*set the first received timestamp to 0 - aside from the offsetIn180k param*/
				Passthru /*offset only*/
			};

			/*offset will be added to the current time*/
			Restamp(Mode mode, int64_t offsetIn180k = 0);
			~Restamp();
			void process(Data data) override;

			/*returned offset in 180k timescale*/
			int64_t getOffset() const;

		private:
			void ensureInit(uint64_t time);

			int64_t offset;
			int64_t initTime = -1; //TODO: add a boolean if we don't need to expose it
			Mode mode;
		};

	}
}
