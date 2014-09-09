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
	: file(file), m_Dts(0) {
}

GPACMuxMP4::~GPACMuxMP4() {
	GF_Err ret = gf_isom_close(file);
	if (ret != GF_OK) {
		Log::msg(Log::Error, "%s: gf_isom_close", gf_error_to_string(ret));
		throw std::runtime_error("Cannot close output file.");
	}
}

void GPACMuxMP4::declareStreamAudio(std::shared_ptr<StreamAudio> stream) {
  GF_Err ret;
  u32 di, track;
  u8 bpsample;
  GF_ESD *esd;
#ifndef GPAC_DISABLE_AV_PARSERS
  GF_M4ADecSpecInfo acfg;
#endif
  AVCodecContext *audio_codec_ctx = audio_output_file->codec_ctx;

  audio_output_file->isof = gf_isom_open(filename, GF_ISOM_OPEN_WRITE, NULL);
  if (!audio_output_file->isof) {
    Log::msg(Log::Warning, "Cannot open iso file %s\n", filename));
    return -1;
  }

  esd = gf_odf_desc_esd_new(2);
  if (!esd) {
    Log::msg(Log::Warning, "Cannot create GF_ESD\n"));
    return -1;
  }

  esd->decoderConfig = (GF_DecoderConfig *)gf_odf_desc_new(GF_ODF_DCD_TAG);
  esd->slConfig = (GF_SLConfig *)gf_odf_desc_new(GF_ODF_SLC_TAG);
  esd->decoderConfig->streamType = GF_STREAM_AUDIO;
  if (!strcmp(audio_output_file->codec_ctx->codec->name, "aac")) { //TODO: find an automatic table
    esd->decoderConfig->objectTypeIndication = GPAC_OTI_AUDIO_AAC_MPEG2_LCP;
    esd->decoderConfig->bufferSizeDB = 20;
    esd->slConfig->timestampResolution = audio_codec_ctx->sample_rate;
    esd->decoderConfig->decoderSpecificInfo = (GF_DefaultDescriptor *)gf_odf_desc_new(GF_ODF_DSI_TAG);
    esd->ESID = 1;

#ifndef GPAC_DISABLE_AV_PARSERS
    memset(&acfg, 0, sizeof(GF_M4ADecSpecInfo));
    acfg.base_object_type = GF_M4A_AAC_LC;
    acfg.base_sr = audio_codec_ctx->sample_rate;
    acfg.nb_chan = audio_codec_ctx->channels;
    acfg.sbr_object_type = 0;
    acfg.audioPL = gf_m4a_get_profile(&acfg);

    ret = gf_m4a_write_config(&acfg, &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
    assert(ret == GF_OK);
#endif
  } else {
    if (strcmp(audio_output_file->codec_ctx->codec->name, "mp2")) {
      Log::msg(Log::Warning, "Unlisted codec, setting GPAC_OTI_AUDIO_MPEG1 descriptor.\n"));
    }
    esd->decoderConfig->objectTypeIndication = GPAC_OTI_AUDIO_MPEG1;
    esd->decoderConfig->bufferSizeDB = 20;
    esd->slConfig->timestampResolution = audio_codec_ctx->sample_rate;
    esd->decoderConfig->decoderSpecificInfo = (GF_DefaultDescriptor *)gf_odf_desc_new(GF_ODF_DSI_TAG);
    esd->ESID = 1;

#ifndef GPAC_DISABLE_AV_PARSERS
    memset(&acfg, 0, sizeof(GF_M4ADecSpecInfo));
    acfg.base_object_type = GF_M4A_LAYER2;
    acfg.base_sr = audio_codec_ctx->sample_rate;
    acfg.nb_chan = audio_codec_ctx->channels;
    acfg.sbr_object_type = 0;
    acfg.audioPL = gf_m4a_get_profile(&acfg);

    ret = gf_m4a_write_config(&acfg, &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
    assert(ret == GF_OK);
#endif
  }

  //gf_isom_store_movie_config(video_output_file->isof, 0);
  track = gf_isom_new_track(audio_output_file->isof, esd->ESID, GF_ISOM_MEDIA_AUDIO, audio_codec_ctx->sample_rate);
  Log::msg(Log::Warning, "TimeScale: %d \n", audio_codec_ctx->time_base.den));
  if (!track) {
    Log::msg(Log::Warning, "Cannot create new track\n"));
    return -1;
  }

  ret = gf_isom_set_track_enabled(audio_output_file->isof, track, 1);
  if (ret != GF_OK) {
    Log::msg(Log::Warning, "%s: gf_isom_set_track_enabled\n", gf_error_to_string(ret)));
    return -1;
  }

  //	if (!esd->ESID) esd->ESID = gf_isom_get_track_id(audio_output_file->isof, track);

  ret = gf_isom_new_mpeg4_description(audio_output_file->isof, track, esd, NULL, NULL, &di);
  if (ret != GF_OK) {
    Log::msg(Log::Warning, "%s: gf_isom_new_mpeg4_description\n", gf_error_to_string(ret)));
    return -1;
  }

  gf_odf_desc_del((GF_Descriptor *)esd);
  esd = NULL;

  bpsample = av_get_bytes_per_sample(audio_output_file->codec_ctx->sample_fmt) * 8;

  ret = gf_isom_set_audio_info(audio_output_file->isof, track, di, audio_codec_ctx->sample_rate, audio_output_file->codec_ctx->channels, bpsample);
  if (ret != GF_OK) {
    Log::msg(Log::Warning, "%s: gf_isom_set_audio_info\n", gf_error_to_string(ret)));
    return -1;
  }

#ifndef GPAC_DISABLE_AV_PARSERS
  ret = gf_isom_set_pl_indication(audio_output_file->isof, GF_ISOM_PL_AUDIO, acfg.audioPL);
  if (ret != GF_OK) {
    Log::msg(Log::Warning, "%s: gf_isom_set_pl_indication\n", gf_error_to_string(ret)));
    return -1;
  }
#endif

  Log::msg(Log::Warning, "time scale: %d  sample dur: %d \n", audio_codec_ctx->time_base.den, audio_output_file->codec_ctx->frame_size));

#if USE_SEGMENTS
  ret = gf_isom_setup_track_fragment(audio_output_file->isof, track, 1, audio_output_file->codec_ctx->frame_size, 0, 0, 0, 0);
  if (ret != GF_OK) {
    Log::msg(Log::Warning, "%s: gf_isom_setutrack_fragment\n", gf_error_to_string(ret)));
    return -1;
  }
#endif

  //gf_isom_add_track_to_root_od(video_output_file->isof,1);

#if USE_SEGMENTS
  ret = gf_isom_finalize_for_fragment(audio_output_file->isof, 1);
  if (ret != GF_OK) {
    Log::msg(Log::Warning, "%s: gf_isom_finalize_for_fragment\n", gf_error_to_string(ret)));
    return -1;
  }
#endif
}

void GPACMuxMP4::declareStreamVideo(std::shared_ptr<StreamVideo> stream) {
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

void GPACMuxMP4::declareStream(std::shared_ptr<Stream> stream) {
  if (std::dynamic_pointer_cast<StreamVideo>(stream)) {
    declareStreamVideo(std::dynamic_pointer_cast<StreamVideo>(stream));
  } else if (std::dynamic_pointer_cast<StreamAudio>(stream)) {
    declareStreamAudio(std::dynamic_pointer_cast<StreamAudio>(stream));
  } else {
    Log::msg(Log::Warning, "[GPACMuxMP4] Invalid stream declared. Ignoring.");
  }
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

	gpacpp::IsoSample sample;
	gf_bs_get_content(out_bs, &sample.data, &sample.dataLength);
	sample.DTS = m_Dts;

#if USE_SEGMENTS
TODO: missing open segment
	GF_Err ret = gf_isom_fragment_add_sample(file, trackId, &sample, 1, 1, 0, 0, GF_FALSE);
	if (ret != GF_OK) {
		gf_bs_del(out_bs);
		Log::msg(Log::Error, "%s: gf_isom_fragment_add_sample", gf_error_to_string(ret));
		return false;
	}
#else
	GF_Err ret = gf_isom_add_sample(file, trackId, 1, &sample);
	if (ret != GF_OK) {
		gf_bs_del(out_bs);
		Log::msg(Log::Error, "%s: gf_isom_add_sample", gf_error_to_string(ret));
		return false;
	}
#endif

	m_Dts++;

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
