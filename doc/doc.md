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
MAKE A LIST OF BASIC OBJECTS
* Data: shared_ptr
* Output
...


Debugging
=========

Mono-thread.




Technical considerations
========================

* Parallelism

With Pipeline, automatic over a thread pool with an ID preventing parallel execution on the same module.

only use a thread if you need the system clock
a clock can be injected in modules

* Data types

* metadata

Contributing
============

TODO

