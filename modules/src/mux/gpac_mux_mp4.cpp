#include "gpac_mux_mp4.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"

extern "C" {
#include <gpac/constants.h>
#include <gpac/isomedia.h>
#include <gpac/internal/media_dev.h>
}

#include "gpacpp.hpp"

#define FRAG_DURATION_IN_180K (10*180000ULL)


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

/**
* A function which takes FFmpeg H265 extradata (SPS/PPS) and bring them ready to be pushed to the MP4 muxer.
* @param extradata
* @param extradata_size
* @param dstcfg
* @returns GF_OK is the extradata was parsed and is valid, other values otherwise.
*/
static GF_Err hevc_import_ffextradata(const u8 *extradata, const u64 extradata_size, GF_HEVCConfig *dstCfg) {
	HEVCState hevc;
	GF_HEVCParamArray *vpss = NULL, *spss = NULL, *ppss = NULL;
	GF_BitStream *bs;
	char *buffer = NULL;
	u32 bufferSize = 0;
	if (!extradata || (extradata_size < sizeof(u32)))
		return GF_BAD_PARAM;
	bs = gf_bs_new((const char*)extradata, extradata_size, GF_BITSTREAM_READ);
	if (!bs)
		return GF_BAD_PARAM;

	memset(&hevc, 0, sizeof(HEVCState));
	hevc.sps_active_idx = -1;

	while (gf_bs_available(bs)) {
		s32 idx;
		GF_AVCConfigSlot *slc;
		u8 NALUnitType, temporalId, layerId;
		u64 NALStart;
		u32 NALSize;

		if (gf_bs_read_u32(bs) != 0x00000001) {
			gf_bs_del(bs);
			return GF_BAD_PARAM;
		}
		NALStart = gf_bs_get_position(bs);
		NALSize = gf_media_nalu_next_start_code_bs(bs);
		if (NALStart + NALSize > extradata_size) {
			gf_bs_del(bs);
			return GF_BAD_PARAM;
		}

		if (NALSize > bufferSize) {
			buffer = (char*)gf_realloc(buffer, NALSize);
			bufferSize = NALSize;
		}
		gf_bs_read_data(bs, buffer, NALSize);
		gf_bs_seek(bs, NALStart);

		gf_media_hevc_parse_nalu(bs, &hevc, &NALUnitType, &temporalId, &layerId);
		if (layerId) {
			gf_bs_del(bs);
			gf_free(buffer);
			return GF_BAD_PARAM;
		}

		switch (NALUnitType) {
		case GF_HEVC_NALU_VID_PARAM:
			idx = gf_media_hevc_read_vps(buffer, NALSize, &hevc);
			if (idx < 0) {
				gf_bs_del(bs);
				gf_free(buffer);
				return GF_BAD_PARAM;
			}

			assert(hevc.vps[idx].state == 1); //we don't expect multiple VPS
			if (hevc.vps[idx].state == 1) {
				hevc.vps[idx].state = 2;
				hevc.vps[idx].crc = gf_crc_32(buffer, NALSize);

				dstCfg->avgFrameRate = hevc.vps[idx].rates[0].avg_pic_rate;
				dstCfg->constantFrameRate = hevc.vps[idx].rates[0].constand_pic_rate_idc;
				//TODO: update GPAC for Signals: dstCfg->numTemporalLayers = hevc.vps[idx].max_sub_layers;
				dstCfg->temporalIdNested = hevc.vps[idx].temporal_id_nesting;

				if (!vpss) {
					GF_SAFEALLOC(vpss, GF_HEVCParamArray);
					vpss->nalus = gf_list_new();
					gf_list_add(dstCfg->param_array, vpss);
					vpss->array_completeness = 1;
					vpss->type = GF_HEVC_NALU_VID_PARAM;
				}

				slc = (GF_AVCConfigSlot*)gf_malloc(sizeof(GF_AVCConfigSlot));
				slc->size = NALSize;
				slc->id = idx;
				slc->data = (char*)gf_malloc(sizeof(char)*slc->size);
				memcpy(slc->data, buffer, sizeof(char)*slc->size);

				gf_list_add(vpss->nalus, slc);
			}
			break;
		case GF_HEVC_NALU_SEQ_PARAM:
			idx = gf_media_hevc_read_sps(buffer, NALSize, &hevc);
			if (idx < 0) {
				gf_bs_del(bs);
				gf_free(buffer);
				return GF_BAD_PARAM;
			}

			assert(!(hevc.sps[idx].state & AVC_SPS_DECLARED)); //we don't expect multiple SPS
			if ((hevc.sps[idx].state & AVC_SPS_PARSED) && !(hevc.sps[idx].state & AVC_SPS_DECLARED)) {
				hevc.sps[idx].state |= AVC_SPS_DECLARED;
				hevc.sps[idx].crc = gf_crc_32(buffer, NALSize);
			}

			dstCfg->configurationVersion = 1;
			dstCfg->profile_space = hevc.sps[idx].ptl.profile_space;
			dstCfg->tier_flag = hevc.sps[idx].ptl.tier_flag;
			dstCfg->profile_idc = hevc.sps[idx].ptl.profile_idc;
			dstCfg->general_profile_compatibility_flags = hevc.sps[idx].ptl.profile_compatibility_flag;
			dstCfg->progressive_source_flag = hevc.sps[idx].ptl.general_progressive_source_flag;
			dstCfg->interlaced_source_flag = hevc.sps[idx].ptl.general_interlaced_source_flag;
			dstCfg->non_packed_constraint_flag = hevc.sps[idx].ptl.general_non_packed_constraint_flag;
			dstCfg->frame_only_constraint_flag = hevc.sps[idx].ptl.general_frame_only_constraint_flag;

			dstCfg->constraint_indicator_flags = hevc.sps[idx].ptl.general_reserved_44bits;
			dstCfg->level_idc = hevc.sps[idx].ptl.level_idc;

			dstCfg->chromaFormat = hevc.sps[idx].chroma_format_idc;
			dstCfg->luma_bit_depth = hevc.sps[idx].bit_depth_luma;
			dstCfg->chroma_bit_depth = hevc.sps[idx].bit_depth_chroma;

			if (!spss) {
				GF_SAFEALLOC(spss, GF_HEVCParamArray);
				spss->nalus = gf_list_new();
				gf_list_add(dstCfg->param_array, spss);
				spss->array_completeness = 1;
				spss->type = GF_HEVC_NALU_SEQ_PARAM;
			}

			slc = (GF_AVCConfigSlot*)gf_malloc(sizeof(GF_AVCConfigSlot));
			slc->size = NALSize;
			slc->id = idx;
			slc->data = (char*)gf_malloc(sizeof(char)*slc->size);
			memcpy(slc->data, buffer, sizeof(char)*slc->size);
			gf_list_add(spss->nalus, slc);
			break;
		case GF_HEVC_NALU_PIC_PARAM:
			idx = gf_media_hevc_read_pps(buffer, NALSize, &hevc);
			if (idx < 0) {
				gf_bs_del(bs);
				gf_free(buffer);
				return GF_BAD_PARAM;
			}

			assert(hevc.pps[idx].state == 1); //we don't expect multiple PPS
			if (hevc.pps[idx].state == 1) {
				hevc.pps[idx].state = 2;
				hevc.pps[idx].crc = gf_crc_32(buffer, NALSize);

				if (!ppss) {
					GF_SAFEALLOC(ppss, GF_HEVCParamArray);
					ppss->nalus = gf_list_new();
					gf_list_add(dstCfg->param_array, ppss);
					ppss->array_completeness = 1;
					ppss->type = GF_HEVC_NALU_PIC_PARAM;
				}

				slc = (GF_AVCConfigSlot*)gf_malloc(sizeof(GF_AVCConfigSlot));
				slc->size = NALSize;
				slc->id = idx;
				slc->data = (char*)gf_malloc(sizeof(char)*slc->size);
				memcpy(slc->data, buffer, sizeof(char)*slc->size);

				gf_list_add(ppss->nalus, slc);
			}
			break;
		default:
			break;
		}

		if (gf_bs_seek(bs, NALStart + NALSize)) {
			assert(NALStart + NALSize <= gf_bs_get_size(bs));
			break;
		}
	}

	gf_bs_del(bs);
	gf_free(buffer);

	return GF_OK;
}

void fillVideoSample(u8 *bufPtr, u32 bufLen, GF_ISOSample &sample) {
	u32 scSize = 0;
	u32 NALUSize = 0;
	GF_BitStream *out_bs = gf_bs_new(NULL, 2 * bufLen, GF_BITSTREAM_WRITE);
	NALUSize = gf_media_nalu_next_start_code(bufPtr, bufLen, &scSize);
	if (NALUSize != 0) {
		gf_bs_write_u32(out_bs, NALUSize);
		gf_bs_write_data(out_bs, (const char*)bufPtr, NALUSize);
	}
	if (scSize) {
		bufPtr += (NALUSize + scSize);
		bufLen -= (NALUSize + scSize);
	}
	while (bufLen) {
		NALUSize = gf_media_nalu_next_start_code(bufPtr, bufLen, &scSize);
		if (NALUSize != 0) {
			gf_bs_write_u32(out_bs, NALUSize);
			gf_bs_write_data(out_bs, (const char*)bufPtr, NALUSize);
		}

		bufPtr += NALUSize;

		if (!scSize || (bufLen < NALUSize + scSize))
			break;
		bufLen -= NALUSize + scSize;
		bufPtr += scSize;
	}
	gf_bs_get_content(out_bs, &sample.data, &sample.dataLength);
	gf_bs_del(out_bs);
}
}

namespace Mux {

//TODO: segments start with RAP
GPACMuxMP4::GPACMuxMP4(const std::string &baseName, bool useSegments)
	: m_DTS(0), m_curFragDur(0), m_segNum(0), m_useSegments(useSegments), m_useFragments(useSegments) {
	std::stringstream fileName;
	fileName << baseName;
	fileName << ".mp4";

	if (baseName == "") {
		//TODO: open in memory - apparrently we have to use the gmem:// protocol
		assert(0);
	} else {
		m_iso = gf_isom_open(fileName.str().c_str(), GF_ISOM_OPEN_WRITE, NULL);
		if (!m_iso) {
			Log::msg(Log::Warning, "Cannot open iso file %s", fileName.str());
			throw std::runtime_error("Cannot open output file.");
		}
	}

	GF_Err e = gf_isom_set_storage_mode(m_iso, GF_ISOM_STORE_INTERLEAVED);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "Cannot make iso file %s interleaved", fileName.str());
		throw std::runtime_error("Cannot make iso file interleaved.");
	}

	//FIXME: this is signalling only (no data)
	if (1 || m_useSegments) { //Romain
		signals.push_back(uptr(pinFactory->createPin()));
	}
}

void GPACMuxMP4::closeSegment() {
	if (m_useSegments) {
		GF_Err e = gf_isom_close_segment(m_iso, 0, 0, 0, 0, 0, GF_FALSE, GF_TRUE, 0, NULL, NULL);
		if (e != GF_OK) {
			Log::msg(Log::Error, "%s: gf_isom_close_segment", gf_error_to_string(e));
			throw std::runtime_error("Cannot close output segment.");
		}

		auto out = signals[0]->getBuffer(0);
		auto mediaTimescale = gf_isom_get_media_timescale(m_iso, gf_isom_get_track_by_id(m_iso, m_trackId));
		out->setTime((m_DTS * IClock::Rate + mediaTimescale / 2) / mediaTimescale);
		out->setDuration(m_curFragDur, mediaTimescale);
		signals[0]->emit(out);
	}
}

GPACMuxMP4::~GPACMuxMP4() {
	GF_Err e;
	if (m_useFragments) {
		gf_isom_flush_fragments(m_iso, GF_TRUE);
	}
	closeSegment();
	e = gf_isom_close(m_iso);
	if (e != GF_OK) {
		Log::msg(Log::Error, "%s: gf_isom_close", gf_error_to_string(e));
		throw std::runtime_error("Cannot close output file.");
	}
}

void GPACMuxMP4::declareStreamAudio(std::shared_ptr<StreamAudio> stream) {
	GF_Err e;
	u32 di, trackNum;
	GF_M4ADecSpecInfo acfg;

	GF_ESD *esd = gf_odf_desc_esd_new(2);
	if (!esd) {
		Log::msg(Log::Warning, "Cannot create GF_ESD\n");
		throw std::runtime_error("Cannot create GF_ESD");
	}

	esd->decoderConfig = (GF_DecoderConfig *)gf_odf_desc_new(GF_ODF_DCD_TAG);
	esd->slConfig = (GF_SLConfig *)gf_odf_desc_new(GF_ODF_SLC_TAG);
	esd->decoderConfig->streamType = GF_STREAM_AUDIO;
	if (stream->codecName == "aac") { //TODO: find an automatic table, we only know about MPEG1 Layer 2 and AAC-LC
		esd->decoderConfig->objectTypeIndication = GPAC_OTI_AUDIO_AAC_MPEG4;

		esd->decoderConfig->bufferSizeDB = 20;
		esd->slConfig->timestampResolution = stream->sampleRate;
		esd->decoderConfig->decoderSpecificInfo = (GF_DefaultDescriptor *)gf_odf_desc_new(GF_ODF_DSI_TAG);
		esd->ESID = 1;

		esd->decoderConfig->decoderSpecificInfo->dataLength = (u32)stream->extradataSize;
		esd->decoderConfig->decoderSpecificInfo->data = (char*)gf_malloc(stream->extradataSize);
		memcpy(esd->decoderConfig->decoderSpecificInfo->data, stream->extradata, stream->extradataSize);

		memset(&acfg, 0, sizeof(GF_M4ADecSpecInfo));
		acfg.base_object_type = GF_M4A_AAC_LC;
		acfg.base_sr = stream->sampleRate;
		acfg.nb_chan = stream->numChannels;
		acfg.sbr_object_type = 0;
		acfg.audioPL = gf_m4a_get_profile(&acfg);

		/*e = gf_m4a_write_config(&acfg, &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
		assert(e == GF_OK);*/
	}
	else {
		if (stream->codecName != "mp2") {
			Log::msg(Log::Warning, "Unlisted codec, setting GPAC_OTI_AUDIO_MPEG1 descriptor.\n");
		}
		esd->decoderConfig->objectTypeIndication = GPAC_OTI_AUDIO_MPEG1;
		esd->decoderConfig->bufferSizeDB = 20;
		esd->slConfig->timestampResolution = stream->sampleRate;
		esd->decoderConfig->decoderSpecificInfo = (GF_DefaultDescriptor *)gf_odf_desc_new(GF_ODF_DSI_TAG);
		esd->ESID = 1;

		memset(&acfg, 0, sizeof(GF_M4ADecSpecInfo));
		acfg.base_object_type = GF_M4A_LAYER2;
		acfg.base_sr = stream->sampleRate;
		acfg.nb_chan = stream->numChannels;
		acfg.sbr_object_type = 0;
		acfg.audioPL = gf_m4a_get_profile(&acfg);

		e = gf_m4a_write_config(&acfg, &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
		assert(e == GF_OK);
	}

	trackNum = gf_isom_new_track(m_iso, esd->ESID, GF_ISOM_MEDIA_AUDIO, stream->sampleRate);
	Log::msg(Log::Warning, "TimeScale: %s\n", stream->sampleRate);
	if (!trackNum) {
		Log::msg(Log::Warning, "Cannot create new track\n");
		throw std::runtime_error("Cannot create new track");
	}

	m_trackId = gf_isom_get_track_id(m_iso, trackNum);

	e = gf_isom_set_track_enabled(m_iso, trackNum, 1);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "%s: gf_isom_set_track_enabled\n", gf_error_to_string(e));
		throw std::runtime_error("gf_isom_set_track_enabled");
	}

	e = gf_isom_new_mpeg4_description(m_iso, trackNum, esd, NULL, NULL, &di);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "%s: gf_isom_new_mpeg4_description\n", gf_error_to_string(e));
		throw std::runtime_error("gf_isom_new_mpeg4_description");
	}

	gf_odf_desc_del((GF_Descriptor *)esd);
	esd = NULL;

	e = gf_isom_set_audio_info(m_iso, trackNum, di, stream->sampleRate, stream->numChannels, stream->bitsPerSample);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "%s: gf_isom_set_audio_info\n", gf_error_to_string(e));
		throw std::runtime_error("gf_isom_set_audio_info");
	}

	e = gf_isom_set_pl_indication(m_iso, GF_ISOM_PL_AUDIO, acfg.audioPL);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "%s: gf_isom_set_pl_indication\n", gf_error_to_string(e));
		throw std::runtime_error("Container format import failed");
	}

	if (m_useFragments) { //FIXME: this is duplicated for audio and video
		e = gf_isom_setup_track_fragment(m_iso, m_trackId, 1, 1, 0, 0, 0, 0);
		if (e != GF_OK) {
			Log::msg(Log::Warning, "%s: gf_isom_setup_track_fragment", gf_error_to_string(e));
			throw std::runtime_error("Cannot setup track as fragmented");
		}
	}

	//gf_isom_add_track_to_root_od(video_output_file->isof, 1);

	if (m_useFragments) {
		if (m_useSegments) {
			e = gf_isom_finalize_for_fragment(m_iso, 1);
			if (e != GF_OK) {
				Log::msg(Log::Warning, "%s: gf_isom_finalize_for_fragment", gf_error_to_string(e));
				throw std::runtime_error("Cannot prepare track for movie fragmentation");
			}

			std::stringstream ss;
			ss << gf_isom_get_filename(m_iso) << "_" << m_segNum;
			e = gf_isom_start_segment(m_iso, (char*)ss.str().c_str(), GF_TRUE);
			if (e != GF_OK) {
				Log::msg(Log::Warning, "%s: gf_isom_start_segment %s\n", gf_error_to_string(e), m_segNum);
				throw std::runtime_error("Impossible to start the segment");
			}
		}
		else {
			e = gf_isom_finalize_for_fragment(m_iso, 0);
			if (e != GF_OK) {
				Log::msg(Log::Warning, "%s: gf_isom_finalize_for_fragment", gf_error_to_string(e));
				throw std::runtime_error("Cannot prepare track for movie fragmentation");
			}
		}

		e = gf_isom_start_fragment(m_iso, GF_TRUE);
		if (e != GF_OK) {
			Log::msg(Log::Warning, "%s: gf_isom_start_fragment\n", gf_error_to_string(e));
			throw std::runtime_error("Impossible to create the moof");
		}
	}
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

	u32 trackNum = gf_isom_new_track(m_iso, 0, GF_ISOM_MEDIA_VISUAL, stream->timeScale);
	if (!trackNum) {
		Log::msg(Log::Warning, "Cannot create new track");
		throw std::runtime_error("Cannot create new track");
	}

	m_trackId = gf_isom_get_track_id(m_iso, trackNum);

	e = gf_isom_set_track_enabled(m_iso, trackNum, GF_TRUE);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "%s: gf_isom_set_track_enabled", gf_error_to_string(e));
		throw std::runtime_error("Cannot enable track");
	}

	u32 di;
	e = gf_isom_avc_config_new(m_iso, trackNum, avccfg, NULL, NULL, &di);
	if (e != GF_OK) {
		Log::msg(Log::Warning, "%s: gf_isom_avc_config_new", gf_error_to_string(e));
		throw std::runtime_error("Cannot create AVC config");
	}

	gf_odf_avc_cfg_del(avccfg);

	gf_isom_set_visual_info(m_iso, trackNum, di, stream->width, stream->height);
	gf_isom_set_sync_table(m_iso, trackNum);

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

	if (m_useFragments) { //FIXME: this is duplicated for audio and video
		e = gf_isom_setup_track_fragment(m_iso, m_trackId, 1, 1, 0, 0, 0, 0);
		if (e != GF_OK) {
			Log::msg(Log::Warning, "%s: gf_isom_setup_track_fragment", gf_error_to_string(e));
			throw std::runtime_error("Cannot setup track as fragmented");
		}
	}

	//gf_isom_add_track_to_root_od(video_output_file->isof, 1);

	if (m_useFragments) {
		if (m_useSegments) {
			e = gf_isom_finalize_for_fragment(m_iso, 1);
			if (e != GF_OK) {
				Log::msg(Log::Warning, "%s: gf_isom_finalize_for_fragment", gf_error_to_string(e));
				throw std::runtime_error("Cannot prepare track for movie fragmentation");
			}

			std::stringstream ss;
			ss << gf_isom_get_filename(m_iso) << "_" << m_segNum;
			e = gf_isom_start_segment(m_iso, (char*)ss.str().c_str(), GF_TRUE);
			if (e != GF_OK) {
				Log::msg(Log::Warning, "%s: gf_isom_start_segment %s\n", gf_error_to_string(e), m_segNum);
				throw std::runtime_error("Impossible to start the segment");
			}
		}
		else {
			e = gf_isom_finalize_for_fragment(m_iso, 0);
			if (e != GF_OK) {
				Log::msg(Log::Warning, "%s: gf_isom_finalize_for_fragment", gf_error_to_string(e));
				throw std::runtime_error("Cannot prepare track for movie fragmentation");
			}
		}

		e = gf_isom_start_fragment(m_iso, GF_TRUE);
		if (e != GF_OK) {
			Log::msg(Log::Warning, "%s: gf_isom_start_fragment\n", gf_error_to_string(e));
			throw std::runtime_error("Impossible to create the moof");
		}
	}
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

void GPACMuxMP4::process(std::shared_ptr<Data> data) {
	GF_ISOSample sample;
	memset(&sample, 0, sizeof(sample));
	bool sampleDataMustBeDeleted = false;

	{
		u32 bufLen = (u32)data->size();
		u8 *bufPtr = data->data();

		u32 mediaType = gf_isom_get_media_type(m_iso, 1);
		if (mediaType == GF_ISOM_MEDIA_VISUAL) {
			fillVideoSample(bufPtr, bufLen, sample);
			sampleDataMustBeDeleted = true;
			sample.DTS = m_DTS;
		} else if (mediaType == GF_ISOM_MEDIA_AUDIO) {
			sample.data = (char*)bufPtr;
			sample.dataLength = bufLen;
			sample.DTS = m_DTS;
		} else {
			assert(0); //TODO: only audio or video supported yet
		}
	}

	auto mediaTimescale = gf_isom_get_media_timescale(m_iso, gf_isom_get_track_by_id(m_iso, m_trackId));
	u32 deltaDTS = u32(data->getDuration() * mediaTimescale) / IClock::Rate;
	m_DTS += deltaDTS;

	if (m_useFragments) {
		m_curFragDur += deltaDTS;

		//TODO: gf_isom_set_traf_base_media_decode_time(m_iso, 1, audio_output_file->first_dts * audio_output_file->codec_ctx->frame_size);
		GF_Err e = gf_isom_fragment_add_sample(m_iso, m_trackId, &sample, 1, deltaDTS, 0, 0, GF_FALSE);
		if (e != GF_OK) {
			Log::msg(Log::Error, "%s: gf_isom_fragment_add_sample", gf_error_to_string(e));
			return;
		}

		if ((m_curFragDur * IClock::Rate) > (mediaTimescale * FRAG_DURATION_IN_180K)) {
			e = gf_isom_flush_fragments(m_iso, GF_FALSE); //Romain: Jean me dit que flush ne sert pas dans notre cas
			if (e != GF_OK) {
				Log::msg(Log::Error, "%s: gf_isom_add_sample", gf_error_to_string(e));
				return;
			}
			closeSegment();
			if (m_useSegments) {
				m_segNum++;

				std::stringstream ss;
				ss << gf_isom_get_filename(m_iso) << "_" << m_segNum;
				e = gf_isom_start_segment(m_iso, (char*)ss.str().c_str(), GF_TRUE);
				if (e != GF_OK) {
					Log::msg(Log::Warning, "%s: gf_isom_start_segment %s\n", gf_error_to_string(e), m_segNum);
					throw std::runtime_error("Impossible to start the segment");
				}
			}

			e = gf_isom_start_fragment(m_iso, GF_TRUE);
			if (e != GF_OK) {
				Log::msg(Log::Error, "%s: gf_isom_start_fragment", gf_error_to_string(e));
				return;
			}

			const u64 oneFragDurInTimescale = (FRAG_DURATION_IN_180K * mediaTimescale + IClock::Rate / 2) / IClock::Rate;
			m_curFragDur = m_DTS - oneFragDurInTimescale * (m_DTS / oneFragDurInTimescale);
		}
	} else {
		GF_Err e = gf_isom_add_sample(m_iso, m_trackId, 1, &sample);
		if (e != GF_OK) {
			Log::msg(Log::Error, "%s: gf_isom_add_sample", gf_error_to_string(e));
			return;
		}
	}

	if (sampleDataMustBeDeleted) {
		gf_free(sample.data);
	}
}

}
