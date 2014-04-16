#
# Download, build and locally deploy external dependencies
#

extra-build: extra-build-ffmpeg extra-build-gpac extra-build-x264
extra-clean:
	rm -rf extra
	mkdir extra

extra-build-ffmpeg: extra/build/ffmpeg/buildOk
extra-build-gpac: extra/build/gpac/buildOk
extra-build-x264: extra/build/x264/buildOk

EXTRA_DIR=$(abspath extra)

#-------------------------------------------------------------------------------
# FFMPEG
#-------------------------------------------------------------------------------

extra/src/ffmpeg/ffmpeg.c:
	@mkdir -p extra/src
	@rm -rf extra/src/ffmpeg
	git clone --depth 8000 git://source.ffmpeg.org/ffmpeg.git extra/src/ffmpeg
	cd extra/src/ffmpeg && git checkout 78e39aa7ee12bb61cf34d8ca6bebd129d659d9cd 

extra/build/ffmpeg/buildOk: extra/src/ffmpeg/ffmpeg.c extra-build-x264
	@mkdir -p extra/build/ffmpeg
	cd extra/build/ffmpeg && $(EXTRA_DIR)/src/ffmpeg/configure \
		--disable-programs \
		--enable-gpl \
		--enable-libx264 \
		--enable-swresample \
		--enable-swscale \
		--extra-cflags="-I$(EXTRA_DIR)/src/x264 -I$(EXTRA_DIR)/build/x264" \
		--extra-ldflags="-L$(EXTRA_DIR)/build/x264 -ldl" \
		--prefix=../..
	$(MAKE) -C extra/build/ffmpeg
	$(MAKE) -C extra/build/ffmpeg install
	touch "$@"

#-------------------------------------------------------------------------------
# GPAC
#-------------------------------------------------------------------------------

extra/src/gpac/Changelog:
	@mkdir -p extra/src
	@rm -rf extra/src/gpac
	svn checkout svn://svn.code.sf.net/p/gpac/code/trunk/gpac -r 5128 extra/src/gpac

extra/build/gpac/buildOk: extra/src/gpac/Changelog
	@mkdir -p extra/build/gpac
	cd extra/build/gpac && ../../src/gpac/configure \
		--prefix=../..
	$(MAKE) -C extra/build/gpac
	$(MAKE) -C extra/build/gpac install
	$(MAKE) -C extra/build/gpac install-lib
	touch "$@"

#-------------------------------------------------------------------------------
# X264
#-------------------------------------------------------------------------------

extra/src/x264/x264.c:
	@mkdir -p extra/src
	@rm -rf extra/src/x264
	git clone --depth 100 git://git.videolan.org/x264.git extra/src/x264
	cd extra/src/x264 && git checkout d6b4e63d2ed8d444b77c11b36c1d646ee5549276

extra/build/x264/buildOk: extra/src/x264/x264.c
	@mkdir -p extra/build/x264
	cd extra/build/x264 && ../../src/x264/configure \
		--prefix=../..
	$(MAKE) -C extra/build/x264
	$(MAKE) -C extra/build/x264 install
	touch "$@"

