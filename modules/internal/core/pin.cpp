#include "pin.hpp"

namespace Modules {

IPin* PinDefaultFactory::createPin(IProps *props) {
	return new PinDefault(props);
}

}
