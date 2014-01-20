#
# Download, build and locally deploy external dependencies
#
extra-fetch: extra-fetch-ffmpeg extra-fetch-gpac
extra-build: extra-build-ffmpeg extra-build-gpac

# FFMPEG

extra-fetch-ffmpeg:
	@mkdir -p extra/src
	git clone git://source.ffmpeg.org/ffmpeg.git extra/src/ffmpeg
	cd extra/src/ffmpeg && git checkout 78e39aa7ee12bb61cf34d8ca6bebd129d659d9cd 

extra-build-ffmpeg:
	@mkdir -p extra/build/ffmpeg
	cd extra/build/ffmpeg && ../../src/ffmpeg/configure --prefix=../..
	$(MAKE) -C extra/build/ffmpeg
	$(MAKE) -C extra/build/ffmpeg install

# GPAC

extra-fetch-gpac:
	@rm -rf extra/src/gpac
	@mkdir -p extra/src
	svn checkout svn://svn.code.sf.net/p/gpac/code/trunk/gpac -r 4992 extra/src/gpac

extra-build-gpac:
	@mkdir -p extra/build/gpac
	cd extra/build/gpac && ../../src/gpac/configure --prefix=../..
	$(MAKE) -C extra/build/gpac
	$(MAKE) -C extra/build/gpac install
	$(MAKE) -C extra/build/gpac install-lib

