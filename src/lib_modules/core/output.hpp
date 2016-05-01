#pragma once

#include "allocator.hpp"
#include "data.hpp"
#include "metadata.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include <lib_signals/signals.hpp>


namespace Modules {

class IModule;

typedef Signals::Signal<void(Data), Signals::ResultQueue<NotVoid<void>>> SignalAsync;
typedef Signals::Signal<void(Data), Signals::ResultVector<NotVoid<void>>> SignalSync;
static Signals::ExecutorSync<void(Data)> g_executorOutputSync;
typedef SignalSync SignalDefaultSync;

class IOutput {
public:
	virtual ~IOutput() noexcept(false) {}
	virtual size_t emit(Data data) = 0;
	virtual Signals::ISignal<void(Data)>& getSignal() = 0;
};

template<typename Allocator, typename Signal>
class OutputT : public IOutput, public MetadataCap {
	public:
		typedef Allocator AllocatorType;

		OutputT(size_t allocatorSize, IMetadata *metadata = nullptr)
			: MetadataCap(metadata), signal(g_executorOutputSync), allocator(new Allocator(allocatorSize)) {
		}
		virtual ~OutputT() noexcept(false) {
			allocator->unblock();
		}

		size_t emit(Data data) override {
			updateMetadata(data);
			size_t numReceivers = signal.emit(data);
			if (numReceivers == 0)
				Log::msg(Debug, "emit(): Output had no receiver");
			return numReceivers;
		}

		template<typename T = typename Allocator::MyType>
		std::shared_ptr<T> getBuffer(size_t size) {
			return allocator->template getBuffer<T>(size);
		}

		Signals::ISignal<void(Data)>& getSignal() override {
			return signal;
		}

	private:
		Signal signal;
		std::unique_ptr<Allocator> allocator;
};

template<typename DataType> using OutputDataDefault = OutputT<PacketAllocator<DataType>, SignalDefaultSync>;
typedef OutputDataDefault<DataRaw> OutputDefault;

template <typename InstanceType, typename ...Args>
InstanceType* createOutput(size_t allocatorSize, Args&&... args) {
	return new InstanceType(allocatorSize, std::forward<Args>(args)...);
}

class IOutputCap {
public:
	virtual ~IOutputCap() noexcept(false) {}
	virtual size_t getNumOutputs() const = 0;
	virtual IOutput* getOutput(size_t i) const = 0;

protected:
	template <typename InstanceType, typename ...Args>
	InstanceType* addOutput(Args&&... args) {
		auto p = createOutput<InstanceType>(getAllocatorSize(), std::forward<Args>(args)...);
		addOutputInternal(p);
		return safe_cast<InstanceType>(p);
	}

private:
	virtual size_t getAllocatorSize() const = 0;
	virtual void addOutputInternal(IOutput *p) = 0;
};

class OutputCap : public virtual IOutputCap {
	public:
		OutputCap(size_t allocatorSize) : allocatorSize(allocatorSize) {}
		virtual ~OutputCap() noexcept(false) {}

		virtual size_t getNumOutputs() const override {
			return outputs.size();
		}
		virtual IOutput* getOutput(size_t i) const override {
			return outputs[i].get();
		}

private:
	virtual size_t getAllocatorSize() const override {
		assert(allocatorSize > 0);
		return allocatorSize;
	}
	virtual void addOutputInternal(IOutput *p) override {
		outputs.push_back(uptr(p));
	}

	std::vector<std::unique_ptr<IOutput>> outputs;
	const size_t allocatorSize;
};

}
