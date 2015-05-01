#include "mpeg_dash.hpp"
#include "lib_modules/core/clock.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "../out/file.hpp"
#include "../common/libav.hpp"
#include <fstream>

extern "C" {
#include <gpac/isomedia.h>
}

namespace {
std::string getUTC() {
	auto utc = gf_net_get_utc();
	time_t t = utc / 1000;
	auto msecs = (u32)(utc - t * 1000);
	auto AST = *gmtime(&t);
	const size_t maxLen = 25;
	char sAST[maxLen];
	snprintf(sAST, maxLen, "%.4d-%02d-%02dT%02d:%02d:%02d.%03dZ", 1900 + AST.tm_year, AST.tm_mon + 1, AST.tm_mday, AST.tm_hour, AST.tm_min, AST.tm_sec, msecs);
	return sAST;
}

GF_DashSegmenterInput *set_dash_input(GF_DashSegmenterInput *dash_inputs, char *name, u32 *nb_dash_inputs)
{
	GF_DashSegmenterInput *di;
	char *sep;
	// skip ./ and ../, and look for first . to figure out extension
	if ((name[1] == '/') || (name[2] == '/') || (name[1] == '\\') || (name[2] == '\\')) sep = strchr(name + 3, '.');
	else {
		char *s2 = strchr(name, ':');
		sep = strchr(name, '.');
		if (sep && s2 && (s2 - sep) < 0) {
			sep = name;
		}
	}

	//then look for our opt separator :
	sep = strchr(sep ? sep : name, ':');

	if (sep && (sep[1] == '\\')) sep = strchr(sep + 1, ':');

	dash_inputs = (GF_DashSegmenterInput*)gf_realloc(dash_inputs, sizeof(GF_DashSegmenterInput) * (*nb_dash_inputs + 1));
	memset(&dash_inputs[*nb_dash_inputs], 0, sizeof(GF_DashSegmenterInput));
	di = &dash_inputs[*nb_dash_inputs];
	(*nb_dash_inputs)++;

	if (sep) {
		char *opts = sep + 1;
		sep[0] = 0;
		while (opts) {
			sep = strchr(opts, ':');
			while (sep) {
				/* this is a real separator if it is followed by a keyword we are looking for */
				if (!strnicmp(sep, ":id=", 4) ||
					!strnicmp(sep, ":period=", 8) ||
					!strnicmp(sep, ":bandwidth=", 11) ||
					!strnicmp(sep, ":role=", 6) ||
					!strnicmp(sep, ":desc", 5) ||
					!strnicmp(sep, ":duration=", 10) ||
					!strnicmp(sep, ":xlink=", 7)) {
					break;
				} else {
					sep = strchr(sep + 1, ':');
				}
			}
			if (sep && !strncmp(sep, "://", 3)) sep = strchr(sep + 3, ':');
			if (sep) sep[0] = 0;

			if (!strnicmp(opts, "id=", 3)) {
				u32 i;
				di->representationID = gf_strdup(opts + 3);
				/* check to see if this representation Id has already been assigned */
				for (i = 0; i<(*nb_dash_inputs) - 1; i++) {
					GF_DashSegmenterInput *other_di;
					other_di = &dash_inputs[i];
					if (!strcmp(other_di->representationID, di->representationID)) {
						GF_LOG(GF_LOG_ERROR, GF_LOG_DASH, ("[DASH] Error: Duplicate Representation ID \"%s\" in command line\n", di->representationID));
					}
				}
			}
			else if (!strnicmp(opts, "period=", 7)) di->periodID = gf_strdup(opts + 7);
			else if (!strnicmp(opts, "bandwidth=", 10)) di->bandwidth = atoi(opts + 10);
			else if (!strnicmp(opts, "role=", 5)) di->role = gf_strdup(opts + 5);
			else if (!strnicmp(opts, "desc", 4)) {
				u32 *nb_descs = NULL;
				char ***descs = NULL;
				u32 opt_offset = 0;
				u32 len;
				if (!strnicmp(opts, "desc_p=", 7)) {
					nb_descs = &di->nb_p_descs;
					descs = &di->p_descs;
					opt_offset = 7;
				} else if (!strnicmp(opts, "desc_as=", 8)) {
					nb_descs = &di->nb_as_descs;
					descs = &di->as_descs;
					opt_offset = 8;
				} else if (!strnicmp(opts, "desc_as_c=", 8)) {
					nb_descs = &di->nb_as_c_descs;
					descs = &di->as_c_descs;
					opt_offset = 10;
				} else if (!strnicmp(opts, "desc_rep=", 8)) {
					nb_descs = &di->nb_rep_descs;
					descs = &di->rep_descs;
					opt_offset = 9;
				}
				if (opt_offset) {
					(*nb_descs)++;
					opts += opt_offset;
					len = (u32)strlen(opts);
					(*descs) = (char **)gf_realloc((*descs), (*nb_descs)*sizeof(char *));
					(*descs)[(*nb_descs) - 1] = (char *)gf_malloc((len + 1)*sizeof(char));
					strncpy((*descs)[(*nb_descs) - 1], opts, len);
					(*descs)[(*nb_descs) - 1][len] = 0;
				}

			} else if (!strnicmp(opts, "xlink=", 6)) di->xlink = gf_strdup(opts + 6);
			else if (!strnicmp(opts, "duration=", 9)) {
				di->period_duration = (Double)atof(opts + 9);
			}

			if (!sep) break;
			sep[0] = ':';
			opts = sep + 1;
		}
	}
	di->file_name = name;
	if (!di->representationID) {
		char szRep[100];
		sprintf(szRep, "%d", *nb_dash_inputs);
		di->representationID = gf_strdup(szRep);
	}

	return dash_inputs;
}
}


namespace Modules {
namespace Stream {

MPEG_DASH::MPEG_DASH(Type type, uint64_t segDurationInMs)
: type(type), segDurationInMs(segDurationInMs), dashCtx(gf_cfg_new(NULL, NULL)) {
	addInput(new Input<DataAVPacket>(this));
}

void MPEG_DASH::endOfStream() {
	if (workingThread.joinable()) {
		for (size_t i = 0; i < inputs.size(); ++i)
			inputs[i]->push(nullptr);
		workingThread.join();
	}
}

MPEG_DASH::~MPEG_DASH() {
	endOfStream();

	if (dashCtx)
		gf_cfg_del(dashCtx);
}

//needed because of the use of system time for live - otherwise awake on data as for any multi-input module
//TODO: add clock to the scheduler
void MPEG_DASH::DASHThread() {
	for (;;) {
		GF_DashSegmenterInput *dash_inputs = nullptr;
		u32 nb_dash_inputs = 0;
		std::vector<Data> data;
		data.resize(getNumInputs() - 1);
		for (size_t i = 0; i < getNumInputs() - 1; ++i) {
			data[i] = inputs[i]->pop();
			if (!data[i]) {
				return;
			} else {
				char *f = gf_strdup(safe_cast<const MetadataFile>(data[i]->getMetadata())->getFilename().c_str());
				dash_inputs = set_dash_input(dash_inputs, f, &nb_dash_inputs);
			}
		}

		u32 nextInMs = GenerateMPD(dash_inputs);

#if 0 //debug traces
		gf_cfg_set_filename(dashCtx, format("%s.ctx.txt", dash_inputs[0].file_name).c_str());
		gf_cfg_save(dashCtx);
#endif

		if (type == Live) {
			auto dur = std::chrono::milliseconds(nextInMs);
			Log::msg(Log::Info, "[MPEG_DASH] Going to sleep for %s ms.", std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
			std::this_thread::sleep_for(dur);
		}

		if (dash_inputs) {
			u32 i, j;
			for (i = 0; i<nb_dash_inputs; i++) {
				GF_DashSegmenterInput *di = &dash_inputs[i];
				if (di->rep_descs) {
					for (j = 0; j<di->nb_rep_descs; j++) {
						gf_free(di->rep_descs[j]);
					}
					gf_free(di->rep_descs);
				}
				if (di->as_descs) {
					for (j = 0; j<di->nb_as_descs; j++) {
						gf_free(di->as_descs[j]);
					}
					gf_free(di->as_descs);
				}
				if (di->as_c_descs) {
					for (j = 0; j<di->nb_as_c_descs; j++) {
						gf_free(di->as_c_descs[j]);
					}
					gf_free(di->as_c_descs);
				}
				if (di->p_descs) {
					for (j = 0; j<di->nb_p_descs; j++) {
						gf_free(di->p_descs[j]);
					}
					gf_free(di->p_descs);
				}
				if (di->representationID) gf_free(di->representationID);
				if (di->periodID) gf_free(di->periodID);
				if (di->xlink) gf_free(di->xlink);
				if (di->role) gf_free(di->role);
			}
			gf_free(dash_inputs);
		}
	}
}

void MPEG_DASH::process() {
	numDataQueueNotify = (int)getNumInputs() - 1; //FIXME: connection/disconnection cannot occur dynamically. Lock inputs?
	if (!workingThread.joinable())
		workingThread = std::thread(&MPEG_DASH::DASHThread, this);
}

u32 MPEG_DASH::GenerateMPD(GF_DashSegmenterInput *dasherInputs) {
	u32 nb_dash_inputs = (u32)getNumInputs() - 1;
	
	char szMPD[GF_MAX_PATH];
	strcpy(szMPD, "dashcastx.mpd");

	Double dash_duration = (double)segDurationInMs / 1000;
	Double mpd_update_time = dash_duration;
	Bool dash_live = (type == Live) ? GF_TRUE : GF_FALSE;
	
	u32 use_url_template = 0;
	Bool segment_timeline = GF_FALSE;
	GF_DashSwitchingMode bitstream_switching = GF_DASH_BSMODE_DEFAULT;
	Bool seg_at_rap = GF_TRUE, frag_at_rap = GF_TRUE;
	char *seg_name = "seg_$RepresentationID$_";
	char *seg_ext = nullptr;
	u32 segment_marker_4cc = 0;
	Double min_buffer = 1.5;
	s32 ast_offset_ms = 0;
	u32 dash_scale = 1000;
	Bool fragments_in_memory = GF_TRUE;

	GF_Err e = gf_dasher_segment_files(szMPD, dasherInputs, nb_dash_inputs, GF_DASH_PROFILE_LIVE,
		nullptr, nullptr, nullptr, nullptr, nullptr, 0,
		use_url_template, segment_timeline, GF_FALSE, GF_FALSE,
		bitstream_switching, seg_at_rap, dash_duration,
		seg_name, seg_ext, segment_marker_4cc, dash_duration, 0, GF_FALSE, frag_at_rap, nullptr,
		dashCtx, GF_DASH_DYNAMIC/*_LAST TODO: on last segment/flush()*/, mpd_update_time, 0, 0.0, min_buffer,
		ast_offset_ms, dash_scale, fragments_in_memory, 0, 0, GF_FALSE,
		GF_FALSE, GF_FALSE, GF_FALSE, 0.0, GF_FALSE, GF_FALSE, nullptr);

	//this happens when reading file while writing them (local playback of the live session ...)
	if (dash_live && (e == GF_IO_ERR))
		Log::msg(Log::Warning, "[MPEG_DASH] Error dashing file (%s) but continuing ...\n", gf_error_to_string(e));

	return gf_dasher_next_update_time(dashCtx, mpd_update_time);
}

void MPEG_DASH::flush() {
	numDataQueueNotify--;
	if ((type == Live) && (numDataQueueNotify == 0))
		endOfStream();
}

}
}
