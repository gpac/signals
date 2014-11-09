#include "pin.hpp"

namespace Modules {

Pin* PinDefaultFactory::createPin(IProps *props) {
	return new PinDefault(props);
}

}
