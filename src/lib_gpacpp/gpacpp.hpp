#pragma once

#include <stdint.h>
#include <string>
#include <stdexcept>

extern "C" {
#include <gpac/tools.h>
#include <gpac/isomedia.h>
#include <gpac/thread.h>
#include <gpac/media_tools.h>
#include <gpac/download.h>
#include <gpac/dash.h>
}

//#define GPAC_MEM_TRACKER

namespace gpacpp {

//------------------------------------------------
// GPAC error
//------------------------------------------------
class Error : public std::exception {
public:
	Error(char const* msg, GF_Err error) throw() : msg_(msg), error_(error) {
		msg_ += ": ";
		msg_ += gf_error_to_string(error_);
	}

	~Error() throw() {
	}

	char const* what() const throw() {
		return msg_.c_str();
	}

	std::string msg_;
	const GF_Err error_;

private:
	Error& operator= (const Error&) = delete;
};

//------------------------------------------------
// wrapper for GF_ISOSample
//------------------------------------------------
class IsoSample : public GF_ISOSample {
public:
	IsoSample() {
		data = NULL;
		dataLength = 0;
		DTS = 0;
		CTS_Offset = 0;
		IsRAP = RAP_NO;
	}
	IsoSample(GF_ISOSample* pOther) {
		GF_ISOSample* pMe = this;
		memcpy(pMe, pOther, sizeof(*pOther));
		gf_free(pOther);
	}
	~IsoSample() {
		gf_free(data);
	}

	IsoSample const& operator=(IsoSample const&) = delete;
};

//------------------------------------------------
// wrapper for GPAC init
//------------------------------------------------
class Init {
public:
	Init(/*bool memTracker = false, */uint32_t globalLogTools = GF_LOG_ALL, uint32_t globalLogLevel = GF_LOG_WARNING) {
#ifdef GPAC_MEM_TRACKER
		gf_sys_init(GF_TRUE);
#else
		gf_sys_init(GF_FALSE);
#endif
		//Romain: gf_log_set_tool_level(globalLogTools, globalLogLevel);
	}

	~Init() {
		gf_sys_close();
	}
};

//------------------------------------------------
// wrapper for GF_ISOFile
//------------------------------------------------
class IsoFile : public Init {
public:
	IsoFile(std::string const& url) {
		u64 missingBytes;
		GF_Err e = gf_isom_open_progressive(url.c_str(), 0, 0, &movie_, &missingBytes);
		if (e != GF_OK && e != GF_ISOM_INCOMPLETE_FILE)
			throw Error("Can't open fragmented MP4 file", e);
	}

	IsoFile(GF_ISOFile* movie) : movie_(movie) {
	}

	~IsoFile() {
		gf_isom_close(movie_);
	}

	IsoFile const& operator=(IsoFile const&) = delete;

	void setSingleMoofMode(bool enable) {
		gf_isom_set_single_moof_mode(movie_, (Bool)enable);
	}

	void refreshFragmented(uint64_t& missingBytes, std::string const& url) {
		GF_Err e = gf_isom_refresh_fragmented(movie_, &missingBytes, url.c_str());
		if (e != GF_OK && e != GF_ISOM_INCOMPLETE_FILE)
			throw Error("Can't refreshing fragmented MP4", e);
	}

	uint32_t getTrackId(int i) const {
		return gf_isom_get_track_id(movie_, i);
	}

	uint32_t getTrackById(uint32_t id) const {
		auto const number = gf_isom_get_track_by_id(movie_, id);
		if (number == 0)
			throw Error("Can't find track ID", GF_BAD_PARAM);
		return number;
	}

	uint32_t getSampleCount(int trackNumber) const {
		return gf_isom_get_sample_count(movie_, trackNumber);
	}

	std::unique_ptr<IsoSample> getSample(int trackNumber, int sampleIndex, int& descriptorIndex) const {
		GF_ISOSample* sample = gf_isom_get_sample(movie_, trackNumber, sampleIndex, (u32*)&descriptorIndex);
		if (!sample)
			throw Error("Sample does not exist", gf_isom_last_error(movie_));
		return std::unique_ptr<IsoSample>(new IsoSample(sample));
	}

	GF_DecoderConfig* getDecoderConfig(int trackHandle, int descriptorIndex) const {
		return gf_isom_get_decoder_config(movie_, trackHandle, descriptorIndex);
	}

	void resetTables(bool flag) {
		gf_isom_reset_tables(movie_, (Bool)flag);
	}

	void resetDataOffset(uint64_t& newBufferStart) {
		gf_isom_reset_data_offset(movie_, &newBufferStart);
	}

	uint64_t getMissingBytes(int trackNumber) {
		return gf_isom_get_missing_bytes(movie_, trackNumber);
	}

	GF_Err lastError() const {
		return gf_isom_last_error(movie_);
	}

private:
	GF_ISOFile* movie_;
};

//------------------------------------------------
// wrapper for GF_Config
//------------------------------------------------
class Config {
public:
	Config(const char *filePath, const char *fileName) {
		cfg = gf_cfg_new(filePath, fileName);
	}

	~Config() {
		gf_cfg_del(cfg);
	}

	GF_Config* get() const {
		return cfg;
	}

	void setKey(const char *secName, const char *keyName, const char *keyValue) {
		GF_Err e = gf_cfg_set_key(cfg, secName, keyName, keyValue);
		if (e < 0)
			throw Error(format("[GPACPP] Cannot set config (%s) key [%s] %s=%s", cfg, secName, keyName, keyValue).c_str(), e);
	}

	const char* getKey(const char *secName, const char *keyName) const {
		return gf_cfg_get_key(cfg, secName, keyName);
	}

	Config const& operator=(Config const&) = delete;
	
private:
	GF_Config *cfg;
};

//------------------------------------------------
// wrapper for GF_DownloadManager
//------------------------------------------------
class DownloadManager : public Init {
public:
	DownloadManager(const Config *cfg) {
		dm = gf_dm_new(cfg->get());
	}

	~DownloadManager() {
		gf_dm_del(dm);
	}

	GF_DownloadManager* get() const {
		return dm;
	}

	GF_DownloadManager const& operator=(GF_DownloadManager const&) = delete;

private:
	GF_DownloadManager *dm;
};

//------------------------------------------------
// wrapper for GF_DashClient
//------------------------------------------------
class DashClient : public Init {
public:
	DashClient(GF_DASHFileIO *dashIO, Config *cfg) : dashIO(dashIO), cfg(cfg), dm(new DownloadManager(cfg)) {
		dashClient = gf_dash_new(this->dashIO.get(), 10, 0, GF_FALSE, GF_TRUE, GF_DASH_SELECT_QUALITY_LOWEST, GF_FALSE, 0);
		if (dashClient == nullptr)
			throw std::runtime_error("Can't create DASH Client");
		state = Init;
	}

	void start(std::string const& url) {
		assert(state == Init);
		GF_Err e = gf_dash_open(dashClient, url.c_str());
		if (e)
			throw Error(format("[MPEG DASH Client] Can't open URL %s", url).c_str(), e);
		state = Started;
	}

	void stop() {
		gf_dash_close(dashClient);
		state = Stopped;
	}

	~DashClient() {
		if (state == Started)
			stop();
		gf_dash_del(dashClient);
	}

	GF_DashClient* get() const {
		return dashClient;
	}

	DownloadManager* downloader() {
		return dm.get();
	}

private:
	enum State {
		Init,
		Started,
		Stopped
	};

	State state;
	std::unique_ptr<GF_DASHFileIO> dashIO;
	std::unique_ptr<Config> cfg;
	std::unique_ptr<DownloadManager> dm;
	GF_DashClient *dashClient;
};

}
