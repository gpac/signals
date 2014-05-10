#!/bin/bash
#
# Download, build and locally deploy external dependencies
#

set -e
EXTRA_DIR=$PWD/extra

export PKG_CONFIG_PATH=$EXTRA_DIR/lib/pkgconfig

if [ -z "$MAKE" ]; then
	MAKE="make -j8"
fi

#-------------------------------------------------------------------------------
# X264
#-------------------------------------------------------------------------------

if [ ! -f extra/src/x264/x264.c ] ;
then
	mkdir -p extra/src
	rm -rf extra/src/x264
	git clone --depth 100 git://git.videolan.org/x264.git extra/src/x264
	pushd extra/src/x264
 	git checkout d6b4e63d2ed8d444b77c11b36c1d646ee5549276
	popd
fi

if [ ! -f extra/build/x264/buildOk ] ;
then
	mkdir -p extra/build/x264
	pushd extra/build/x264
 	../../src/x264/configure \
		--disable-shared \
		--enable-static \
		--prefix=$EXTRA_DIR
	popd
	$MAKE -C extra/build/x264
	$MAKE -C extra/build/x264 install
	touch extra/build/x264/buildOk 
fi

#-------------------------------------------------------------------------------
# FFMPEG
#-------------------------------------------------------------------------------

if [ ! -f extra/src/ffmpeg/ffmpeg.c ] ;
then
	mkdir -p extra/src
	rm -rf extra/src/ffmpeg
	git clone --depth 8000 git://source.ffmpeg.org/ffmpeg.git extra/src/ffmpeg
	pushd extra/src/ffmpeg
 	git checkout 78e39aa7ee12bb61cf34d8ca6bebd129d659d9cd 
	popd
fi

if [ ! -f extra/build/ffmpeg/buildOk ] ;
then
	mkdir -p extra/build/ffmpeg
	pushd extra/build/ffmpeg
	../../src/ffmpeg/configure \
		--disable-programs \
		--disable-shared \
		--enable-static \
		--enable-gpl \
		--enable-libx264 \
		--enable-swresample \
		--enable-swscale \
		--extra-cflags="`pkg-config x264 --cflags`" \
		--extra-ldflags="`pkg-config x264 --libs`" \
		--prefix=$EXTRA_DIR
	popd
	$MAKE -C extra/build/ffmpeg
	$MAKE -C extra/build/ffmpeg install
	touch extra/build/ffmpeg/buildOk
fi

#-------------------------------------------------------------------------------
# GPAC
#-------------------------------------------------------------------------------

if [ ! -f extra/src/gpac/Changelog ] ;
then
	mkdir -p extra/src
	rm -rf extra/src/gpac
	svn checkout svn://svn.code.sf.net/p/gpac/code/trunk/gpac -r 5246 extra/src/gpac
fi

if [ ! -f extra/build/gpac/buildOk ] ;
then
	mkdir -p extra/build/gpac
	pushd extra/build/gpac
	../../src/gpac/configure \
		--prefix=$EXTRA_DIR

	# workaround gpac configure script not creating all needed directories;
	mkdir -p applications/dashcast
	cp ../../src/gpac/applications/dashcast/Makefile applications/dashcast

	$MAKE
	$MAKE install
	$MAKE install-lib
	popd
	touch extra/build/gpac/buildOk 
fi

echo "Done"
