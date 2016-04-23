#pragma once

#include "stranded_pool_executor.hpp"
#include "../core/module.hpp"
#include "lib_signals/utils/helper.hpp"
#include <memory>


namespace Modules {

template<typename Class>
Signals::MemberFunctor<void, Class, void(Class::*)()>
MEMBER_FUNCTOR_PROCESS(Class* objectPtr) {
	return Signals::MemberFunctor<void, Class, void(Class::*)()>(objectPtr, &IProcessor::process);
}

template<typename ModuleType>
size_t ConnectOutputToInput(IOutput* prev, ModuleType* next, IProcessExecutor * const executor = defaultExecutor) {
	auto prevMetadata = safe_cast<const IMetadataCap>(prev)->getMetadata();
	auto nextMetadata = next->getMetadata();
	if (prevMetadata && nextMetadata) {
		if (prevMetadata->getStreamType() != next->getMetadata()->getStreamType())
			throw std::runtime_error("Module connection: incompatible types");
		Log::msg(Info, "--------- Connect: metadata OK");
	} else {
		if (prevMetadata && !nextMetadata) {
#if 0 //rely on data to propagate type instead of inputs or outputs - this way sent data type is on the output, processed data is on the input
			next->setMetadata(prevMetadata);
			log(Info, "--------- Connect: metadata Propagate to next");
#endif
		} else if (!prevMetadata && nextMetadata) {
			safe_cast<IMetadataCap>(prev)->setMetadata(nextMetadata);
			Log::msg(Info, "--------- Connect: metadata Propagate to prev (backward)");
		} else {
			Log::msg(Info, "--------- Connect: no metadata");
		}
	}

	next->connect();
	return prev->getSignal().connect(
		[=](Data data)
	{
		next->push(data);
		(*executor)(MEMBER_FUNCTOR_PROCESS(next));
	}
	);
}

template<typename ModuleType>
size_t ConnectOutputToInput(IOutput* prev, std::unique_ptr<ModuleType>& next, IProcessExecutor& executor = defaultExecutor) {
	return ConnectOutputToInput(prev, next->getInput(0), &executor);
}

template<typename ModuleType1, typename ModuleType2>
size_t ConnectModules(ModuleType1 *prev, size_t outputIdx, ModuleType2 *next, size_t inputIdx, IProcessExecutor& executor = defaultExecutor) {
	auto output = prev->getOutput(outputIdx);
	return ConnectOutputToInput(output, next->getInput(inputIdx), executor);
}

template <typename T = IMetadata>
std::shared_ptr<const T> getMetadataFromOutput(IOutput const * const out) {
	auto const metadata = safe_cast<const IMetadataCap>(out)->getMetadata();
	return safe_cast<const T>(metadata);
}

template <typename T>
std::shared_ptr<const T> getMetadataFromOutputUnsafe(IOutput const * const out) {
	auto const metadata = safe_cast<const IMetadataCap>(out)->getMetadata();
	return std::dynamic_pointer_cast<const T>(metadata);
}

}
