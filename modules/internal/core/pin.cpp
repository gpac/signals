#include "pin.hpp"

namespace Modules {

PinDefaultFactory::PinDefaultFactory() {
}

Pin* PinDefaultFactory::createPin(IProps *props) {
	return new PinDefault(props);
}

}
