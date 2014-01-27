#include "gpac_mux_mp4.hpp"
#include "../utils/log.hpp"

extern "C" {
#include <gpac/constants.h>
#include <gpac/isomedia.h>
#include <gpac/internal/media_dev.h>
}

#include "gpacpp.hpp"

namespace {
static GF_Err avc_import_ffextradata(const u8 *extradata, const u64 extradataSize, GF_AVCConfig *dstcfg) {
	u8 nalSize;
	AVCState avc;
	GF_BitStream *bs;
	if (!extradata || !extradataSize) {
		Log::msg(Log::Warning, "No initial SPS/PPS provided.");
		return GF_OK;
	}
	bs = gf_bs_new((const char*)extradata, extradataSize, GF_BITSTREAM_READ);
	if (!bs) {
		return GF_BAD_PARAM;
	}
	if (gf_bs_read_u32(bs) != 0x00000001) {
		gf_bs_del(bs);
		return GF_BAD_PARAM;
	}

	//SPS
	{
		s32 idx;
		char *buffer = NULL;
		const u64 nalStart = 4;
		nalSize = gf_media_nalu_next_start_code_bs(bs);
		if (nalStart + nalSize > extradataSize) {
			gf_bs_del(bs);
			return GF_BAD_PARAM;
		}
		buffer = (char*)gf_malloc(nalSize);
		gf_bs_read_data(bs, buffer, nalSize);
		gf_bs_seek(bs, nalStart);
		if ((gf_bs_read_u8(bs) & 0x1F) != GF_AVC_NALU_SEQ_PARAM) {
			gf_bs_del(bs);
			gf_free(buffer);
			return GF_BAD_PARAM;
		}

		idx = gf_media_avc_read_sps(buffer, nalSize, &avc, 0, NULL);
		if (idx < 0) {
			gf_bs_del(bs);
			gf_free(buffer);
			return GF_BAD_PARAM;
		}

		dstcfg->configurationVersion = 1;
		dstcfg->profile_compatibility = avc.sps[idx].prof_compat;
		dstcfg->AVCProfileIndication = avc.sps[idx].profile_idc;
		dstcfg->AVCLevelIndication = avc.sps[idx].level_idc;
		dstcfg->chroma_format = avc.sps[idx].chroma_format;
		dstcfg->luma_bit_depth = 8 + avc.sps[idx].luma_bit_depth_m8;
		dstcfg->chroma_bit_depth = 8 + avc.sps[idx].chroma_bit_depth_m8;

		{
			GF_AVCConfigSlot *slc = (GF_AVCConfigSlot*)gf_malloc(sizeof(GF_AVCConfigSlot));
			slc->size = nalSize;
			slc->id = idx;
			slc->data = buffer;
			gf_list_add(dstcfg->sequenceParameterSets, slc);
		}
	}

	//PPS
	{
		s32 idx;
		char *buffer = NULL;
		const u64 nalStart = 4 + nalSize + 4;
		gf_bs_seek(bs, nalStart);
		nalSize = gf_media_nalu_next_start_code_bs(bs);
		if (nalStart + nalSize > extradataSize) {
			gf_bs_del(bs);
			return GF_BAD_PARAM;
		}
		buffer = (char*)gf_malloc(nalSize);
		gf_bs_read_data(bs, buffer, nalSize);
		gf_bs_seek(bs, nalStart);
		if ((gf_bs_read_u8(bs) & 0x1F) != GF_AVC_NALU_PIC_PARAM) {
			gf_bs_del(bs);
			gf_free(buffer);
			return GF_BAD_PARAM;
		}

		idx = gf_media_avc_read_pps(buffer, nalSize, &avc);
		if (idx < 0) {
			gf_bs_del(bs);
			gf_free(buffer);
			return GF_BAD_PARAM;
		}

		{
			GF_AVCConfigSlot *slc = (GF_AVCConfigSlot*)gf_malloc(sizeof(GF_AVCConfigSlot));
			slc->size = nalSize;
			slc->id = idx;
			slc->data = buffer;
			gf_list_add(dstcfg->pictureParameterSets, slc);
		}
	}

	gf_bs_del(bs);
	return GF_OK;
}
}

namespace Mux {

GPACMuxMP4* GPACMuxMP4::create(const std::string &baseName) {
	std::stringstream fileName;
	fileName << baseName;
	fileName << ".mp4";

	GF_ISOFile *file = gf_isom_open(fileName.str().c_str(), GF_ISOM_OPEN_WRITE, NULL);
	if (!file) {
		Log::msg(Log::Warning, "Cannot open iso file %s", fileName.str());
		throw std::runtime_error("Cannot open output file.");
	}

	return new GPACMuxMP4(file);
}

GPACMuxMP4::GPACMuxMP4(GF_ISOFile *file)
: file(file), sample(new gpacpp::IsoSample()) {
}

GPACMuxMP4::~GPACMuxMP4() {
	GF_Err ret = gf_isom_close(file);
	if (ret != GF_OK) {
		Log::msg(Log::Error, "%s: gf_isom_close\n", gf_error_to_string(ret));
		throw std::runtime_error("Cannot close output file.");
	}
}

void GPACMuxMP4::declareStream(std::shared_ptr<StreamVideo> stream) {
	GF_AVCConfig *avccfg = gf_odf_avc_cfg_new();
	if (!avccfg) {
		Log::msg(Log::Warning, "Cannot create AVCConfig");
		throw std::runtime_error("Container format import failed");
	}

	GF_Err e = avc_import_ffextradata(stream->extradata, stream->extradataSize, avccfg);
	if (e) {
		Log::msg(Log::Warning, "Cannot parse H264 SPS/PPS");
		gf_odf_avc_cfg_del(avccfg);
		throw std::runtime_error("Container format import failed");
	}

	u32 trackNum = gf_isom_new_track(file, 0, GF_ISOM_MEDIA_VISUAL, stream->timeScale);
	if (!trackNum) {
		Log::msg(Log::Warning, "Cannot create new track");
		throw std::runtime_error("Cannot create new track");
	}

	trackId = gf_isom_get_track_id(file, trackNum);

	e = gf_isom_set_track_enabled(file, trackNum, GF_TRUE);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "%s: gf_isom_set_track_enabled", gf_error_to_string(e));
		throw std::runtime_error("Cannot enable track");
	}

	u32 di;
	e = gf_isom_avc_config_new(file, trackNum, avccfg, NULL, NULL, &di);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "%s: gf_isom_avc_config_new", gf_error_to_string(e));
		throw std::runtime_error("Cannot create AVC config");
	}

	gf_odf_avc_cfg_del(avccfg);

	gf_isom_set_visual_info(file, trackNum, di, stream->width, stream->height);
	gf_isom_set_sync_table(file, trackNum);

	//inband SPS/PPS
#if 0
	if (video_output_file->muxer_type == GPAC_INIT_VIDEO_MUXER_AVC3) {
		e = gf_isom_avc_set_inband_config(file, trackNum, 1);
		if (e != GF_OK) {
			Log::msg(Log::Warning, "%s: gf_isom_avc_set_inband_config", gf_error_to_string(e));
			throw std::runtime_error("Cannot set inband PPS/SPS for AVC track");
		}
	}
#endif

#if USE_SEGMENTS
	e = gf_isom_setup_track_fragment(file, trackId, 1, 1, 0, 0, 0, 0);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "%s: gf_isom_setutrack_fragment", gf_error_to_string(e));
		throw std::runtime_error("Cannot setup track as fragmented");
	}

	e = gf_isom_finalize_for_fragment(file, 0/*no segment*/);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "%s: gf_isom_finalize_for_fragment", gf_error_to_string(e));
		throw std::runtime_error("Cannot prepare track for movie fragmentation");
	}
#endif
}

bool GPACMuxMP4::process(std::shared_ptr<Data> data) {
	u32 sc_size = 0;
	u32 nalu_size = 0;
	u32 buf_len = (u32)data->size();
	u8 *buf_ptr = data->data();

	GF_BitStream *out_bs = gf_bs_new(NULL, 2 * buf_len, GF_BITSTREAM_WRITE);
	nalu_size = gf_media_nalu_next_start_code(buf_ptr, buf_len, &sc_size);
	if (nalu_size != 0) {
		gf_bs_write_u32(out_bs, nalu_size);
		gf_bs_write_data(out_bs, (const char*)buf_ptr, nalu_size);
	}
	if (sc_size) {
		buf_ptr += (nalu_size + sc_size);
		buf_len -= (nalu_size + sc_size);
	}
	while (buf_len) {
		nalu_size = gf_media_nalu_next_start_code(buf_ptr, buf_len, &sc_size);
		if (nalu_size != 0) {
			gf_bs_write_u32(out_bs, nalu_size);
			gf_bs_write_data(out_bs, (const char*)buf_ptr, nalu_size);
		}

		buf_ptr += nalu_size;

		if (!sc_size || (buf_len < nalu_size + sc_size))
			break;
		buf_len -= nalu_size + sc_size;
		buf_ptr += sc_size;
	}

	gf_bs_get_content(out_bs, &sample->data, &sample->dataLength);

#if USE_SEGMENTS
	TODO: missing open segment
	GF_Err ret = gf_isom_fragment_add_sample(file, trackId, sample.get(), 1, 1, 0, 0, GF_FALSE);
	if (ret != GF_OK) {
		gf_bs_del(out_bs);
		Log::msg(Log::Error, "%s: gf_isom_fragment_add_sample\n", gf_error_to_string(ret));
		return false;
	}
#else
	GF_Err ret = gf_isom_add_sample(file, trackId, 1, sample.get());
	if (ret != GF_OK) {
		gf_bs_del(out_bs);
		Log::msg(Log::Error, "%s: gf_isom_add_sample\n", gf_error_to_string(ret));
		return false;
	}
#endif

	sample->DTS++;

	gf_free(sample->data); //free data but keep sample structure alive
	gf_bs_del(out_bs);
	return true;
}

bool GPACMuxMP4::handles(const std::string &url) {
	return true;
}

bool GPACMuxMP4::canHandle(const std::string &url) {
	return true;
}

}
