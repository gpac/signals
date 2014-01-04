
namespace Tests {
	namespace Module {
		class Signaller {
		public:
			Signal<int(int)> signal;
		};

		class Slot {
		public:
			int slot(int a) {
				std::cout << "a=" << a << std::endl;
				return a;
			}
		};

		int main(int argc, char **argv) {
			Signaller sender;
			Slot receiver;
			std::function<int(int)> f = std::bind(&Slot::slot, receiver, std::placeholders::_1);
			sender.signal.connect(f);
			sender.signal.emit(100);
			sender.signal.results();
			return 0;
		}
	}
}

