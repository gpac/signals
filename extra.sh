#!/bin/bash
#
# Download, build and locally deploy external dependencies
#

set -e
EXTRA_DIR=$PWD/extra
HOST=$(gcc -dumpmachine)
export CFLAGS=-w

export PKG_CONFIG_PATH=$EXTRA_DIR/lib/pkgconfig

if [ -z "$MAKE" ]; then
	CORES=$(nproc)
	MAKE="make -j$CORES"
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
		--host=$HOST \
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
		--host-os=$HOST \
		--disable-programs \
		--disable-shared \
		--enable-static \
		--enable-gpl \
		--enable-libx264 \
		--enable-swresample \
		--enable-swscale \
		--extra-cflags="`pkg-config x264 --cflags`" \
		--extra-libs="`pkg-config x264 --libs`" \
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
		--use-ffmpeg=no \
		--prefix=$EXTRA_DIR

	$MAKE
	$MAKE install
	$MAKE install-lib
	popd
	touch extra/build/gpac/buildOk 
fi

#-------------------------------------------------------------------------------
# SDL2
#-------------------------------------------------------------------------------

if [ ! -f extra/src/sdl2/configure ] ;
then
	mkdir -p extra/src
	rm -rf extra/src/sdl2
	pushd extra/src
	wget http://libsdl.org/release/SDL2-2.0.3.tar.gz
	tar xvlf SDL2-2.0.3.tar.gz
	mv "SDL2-2.0.3" sdl2
	popd
fi

if [ ! -f extra/build/sdl2/buildOk ] ;
then

	mkdir -p extra/build/sdl2
	pushd extra/build/sdl2
	../../src/sdl2/configure \
		--prefix=$EXTRA_DIR

	$MAKE
	$MAKE install
	popd
	touch extra/build/sdl2/buildOk
fi

echo "Done"
