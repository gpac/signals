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
	virtual size_t getAllocatorSize() const = 0;
};

template<typename Allocator, typename Signal>
class OutputT : public IOutput, public MetadataCap {
	public:
		typedef Allocator AllocatorType;

		OutputT(size_t allocatorSize, IMetadata *metadata = nullptr)
			: MetadataCap(metadata), signal(g_executorOutputSync), allocator(new Allocator(allocatorSize)), allocatorSize(allocatorSize) {
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

		virtual size_t getAllocatorSize() const override {
			return allocatorSize;
		}

		Signals::ISignal<void(Data)>& getSignal() override {
			return signal;
		}

	private:
		Signal signal;
		std::unique_ptr<Allocator> allocator;
		const size_t allocatorSize;
};

template<typename DataType> using OutputDataDefault = OutputT<PacketAllocator<DataType>, SignalDefaultSync>;
typedef OutputDataDefault<DataRaw> OutputDefault;

class IOutputCap {
public:
	virtual ~IOutputCap() noexcept(false) {}
	virtual size_t getNumOutputs() const = 0;
	virtual IOutput* getOutput(size_t i) const = 0;

protected:
	virtual size_t getAllocatorSize() const = 0;
	virtual void addOutputInternal(IOutput *p) = 0;
};

class OutputCap : public virtual IOutputCap {
	public:
		virtual ~OutputCap() noexcept(false) {}

		virtual size_t getNumOutputs() const override {
			return outputs.size();
		}
		virtual IOutput* getOutput(size_t i) const override {
			return outputs[i].get();
		}

protected:
	size_t getAllocatorSize() const override {
		if (outputs.empty())
			throw std::runtime_error("Cannot get allocator size with no pin instantiated.");
		return outputs[0]->getAllocatorSize();
	}
	virtual void addOutputInternal(IOutput *p) override {
		outputs.push_back(uptr(p));
	}

private:
	std::vector<std::unique_ptr<IOutput>> outputs;
};

}
