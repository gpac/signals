<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [Coding consideration](#coding-consideration)
- [Design](#design)
- [Write an application](#write-an-application)
- [Write a module](#write-a-module)
- [Data and metadata](#data-and-metadata)
- [Debugging](#debugging)
- [Technical considerations](#technical-considerations)
- [Contributing](#contributing)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

Coding consideration
====================

You need a C++11 compiler. ```ccache``` on unix can accelerate rebuilds: consider aliasing your CXX (e.g. ```CXX="ccache g++"```).

Before committing please execute tests (check.sh will reformat, build, and make tests for you).

Design
======================

Signals is layered (from bottom to top):
- signals: a signal-slot system
- modules: a generic set of module interfaces and connection helpers
- mm? TODO multimedia consideration (modules would be agnostic)
- media: some module implementations based on libmodules
- pipeline: pipeline builder and connection helpers

Signals include:
 - lib_signals: an agnostic signal/slot mechanism using C++11. May convey any type of data with any type of messaging.
 - lib_modules: an agnostic modules system. Uses lib_signals to connect the modules.  Modules are: inputs/outputs, a clock, an allocator, a data/metadata system. Everything can be configured thru templates. lib_modules comes with some helpers: a special thread-pool that guarantees thread-safeness of calls on a module and a pipeline class to supervise modules.
 - lib_media: a multimedia-specific layer. Uses lib_modules. Defines types for audio and video, and a lot of modules (encode/decode, mux/demux, transform, stream, render, etc.).
 - others: Signals also include some C++ wrappers for FFmpeg and GPAC, and some lib_utils (logs, profilings, C++ utils).

Write an application
====================

Modules have an easy interface:
```
class Module : public IModule, public InputCap, public OutputCap {
	public:
		Module() = default;
		virtual ~Module() noexcept(false) {}
		virtual void flush() {}
	private:
		Module(Module const&) = delete;
		Module const& operator=(Module const&) = delete;
};

+ PROCESS

```

Optional:
flush()

How to declare an input:

What about dynamic inputs as e.g. required by muxers:

How to declare an output:





All modules derive from several classes:

ADD DOXYGEN

When developping modules, you should not take care of concurrency (threads or mutex). Signals takes care of the hard part for you. However this is tweakable.

Use the Pipeline namespace.

Source called only once. Stops when given nullptr on "virtual" input pin 0.

ORDER OF ALLOC

Interrupting a 


Write a module
==============

Use the Modules namespace.

Constructor: use hard types.
flush() = EOS

* Inputs

In the pipeline, input modules will be called two times on void ::process(Data data): onece at the beginning, and one when asked to exit on each input pin. Therefore the structure should be:
```
void module::process(Data data) {
	while (getInput(0)->tryPop(data)) {
		auto out = outputs[0]->getBuffer(0);
		[do my stuff here]
		out->setTime(time);
		output->emit(out);
	}
}
```
TODO: add a test framework for modules (to prove they behave as expected by the API).

If your last input pin has the type DataLoose, you should ignore it. It means it is a fake input pin for:
 - Dynamic input pins (e.g. muxers).
 - Sources which need to receive input null data.

TODO

* Allocator
MAKE A LIST OF BASIC OBJECTS => link to doxygen
* Data: shared_ptr
* Output
...


Data and metadata
=================

Data size should have a size so that the allocator can recycle => TODO: return a Size class that can be compared or properties (resizable etc.), see #17 and allocator.hpp
ATM we set isRecyclable(). Set it to false if you make ```new``` in your allocator: the destructor will be called.


```
auto const metadata = data->getMetadata();
	if (auto video = std::dynamic_pointer_cast<const MetadataPktLibavVideo>(metadata)) {
		declareStreamVideo(video);
	} else if (auto audio = std::dynamic_pointer_cast<const MetadataPktLibavAudio>(metadata)) {
		declareStreamAudio(audio);
```

Debugging
=========

Mono-thread.


Tests
=====

Tests can also be written within .cpp implementation.


Technical considerations
========================

* Parallelism

With Pipeline, automatic over a thread pool with an ID preventing parallel execution on the same module.

only use a thread if you need the system clock
a clock can be injected in modules

* Data types

* metadata


Generating this doc
===================

Markdown: doctoc for tables of content

Doxygen: http://gpac.github.io/signals/

Contributing
============

TODO
Github, pull requests.

