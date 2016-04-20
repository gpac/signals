#include "lib_utils/profiler.hpp"
#include "pipeliner.hpp"
#include <csignal>

using namespace Pipelines;

Pipeline *g_Pipeline = nullptr;

#ifdef _MSC_VER
BOOL WINAPI signalHandler(_In_ DWORD dwCtrlType) {
	switch (dwCtrlType) {
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		printf("Exit event received.\n\n");
		if (g_Pipeline) {
			g_Pipeline->exitSync();
			g_Pipeline = nullptr;
		}
		return TRUE;
	default:
		return FALSE;
	}
}
#else
void sigTermHandler(int sig) {
	if (sig == SIGTERM) {
		std::cerr << "Caught signal(SIGTERM), exiting." << std::endl;
		if (g_Pipeline) {
			g_Pipeline->exitSync();
			g_Pipeline = nullptr;
		}
	}
}
#endif

int safeMain(int argc, char const* argv[]) {
#ifdef _MSC_VER
	SetConsoleCtrlHandler(signalHandler, TRUE);
#else
	std::signal(SIGTERM, sigTermHandler);
#endif

	dashcastXOptions opt = processArgs(argc, argv);

	Tools::Profiler profilerGlobal("DashcastX");

	auto pipeline = uptr(new Pipeline(opt.isLive));
	declarePipeline(*pipeline, opt);
	g_Pipeline = pipeline.get();

	Tools::Profiler profilerProcessing("DashcastX - processing time");
	std::cerr << "DashcastX - close window or ctrl-c to exit cleanly." << std::endl;
	pipeline->start();
	pipeline->waitForCompletion();
	g_Pipeline = nullptr;

	return 0;
}
