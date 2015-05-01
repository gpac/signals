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
if [ ! -f extra/src/x264/x264.c ] ; then
	mkdir -p extra/src
	rm -rf extra/src/x264
	git clone --depth 100 git://git.videolan.org/x264.git extra/src/x264
	pushd extra/src/x264
 	git checkout dd79a61e0e354a432907f2d1f7137b27a12dfce7
	popd
fi

if [ ! -f extra/build/x264/buildOk ] ; then
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
# vo-aacenc
#-------------------------------------------------------------------------------
if [ ! -f extra/src/vo-aacenc-0.1.3/configure ] ; then
	mkdir -p extra/src
	rm -rf extra/src/vo-aacenc
	wget http://sourceforge.net/projects/opencore-amr/files/vo-aacenc/vo-aacenc-0.1.3.tar.gz/download -O vo-aacenc.tar.gz
	tar xvlf vo-aacenc.tar.gz -C extra/src
	rm vo-aacenc.tar.gz
fi

if [ ! -f extra/build/vo-aacenc/buildOk ] ; then
	mkdir -p extra/build/vo-aacenc
	pushd extra/build/vo-aacenc
	../../src/vo-aacenc-0.1.3/configure \
		--host=$HOST \
		--prefix=$EXTRA_DIR
	$MAKE
	$MAKE install
	popd
	touch extra/build/vo-aacenc/buildOk
fi

#-------------------------------------------------------------------------------
# FFMPEG
#-------------------------------------------------------------------------------
if [ ! -f extra/src/ffmpeg/ffmpeg.c ] ; then
	mkdir -p extra/src
	rm -rf extra/src/ffmpeg
	git clone git://source.ffmpeg.org/ffmpeg.git extra/src/ffmpeg
	pushd extra/src/ffmpeg
	git checkout 27f936eca8a1703a5c203f5d2cbc76862c9219fc
	popd
fi

if [ ! -f extra/build/ffmpeg/buildOk ] ; then
	mkdir -p extra/build/ffmpeg
	pushd extra/build/ffmpeg
	../../src/ffmpeg/configure \
		--host-os=$HOST \
		--disable-programs \
		--disable-shared \
		--enable-static \
		--enable-gpl \
		--enable-libx264 \
		--enable-version3 \
		--enable-libvo-aacenc \
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
if [ ! -f extra/src/gpac/Changelog ] ; then
	mkdir -p extra/src
	rm -rf extra/src/gpac
	git clone https://github.com/gpac/gpac.git extra/src/gpac
	pushd extra/src/gpac
	git checkout -q 4d74713
	popd
fi

if [ ! -f extra/build/gpac/buildOk ] ; then
	mkdir -p extra/build/gpac
	pushd extra/build/gpac
	../../src/gpac/configure \
		--use-ffmpeg=no \
		--use-zlib=no \
		--use-png=no \
		--use-jpeg=no \
		--prefix=$EXTRA_DIR

	$MAKE lib
	$MAKE install-lib
	cp gpac.pc $EXTRA_DIR/lib/pkgconfig
	popd
	touch extra/build/gpac/buildOk 
fi

#-------------------------------------------------------------------------------
# SDL2
#-------------------------------------------------------------------------------
if [ ! -f extra/src/sdl2/configure ] ; then
	mkdir -p extra/src
	rm -rf extra/src/sdl2
	pushd extra/src
	wget http://libsdl.org/release/SDL2-2.0.3.tar.gz
	tar xvlf SDL2-2.0.3.tar.gz
	mv "SDL2-2.0.3" sdl2
	popd
fi

if [ ! -f extra/build/sdl2/buildOk ] ; then
	mkdir -p extra/build/sdl2
	pushd extra/build/sdl2
	../../src/sdl2/configure \
		--prefix=$EXTRA_DIR

	$MAKE
	$MAKE install
	popd
	touch extra/build/sdl2/buildOk
fi

#-------------------------------------------------------------------------------
# ASIO
#-------------------------------------------------------------------------------

if [ ! -f extra/src/asio/asio/include/asio.hpp ] ; then
	mkdir -p extra/src
	rm -rf extra/src/asio
	git clone --depth 1000 https://github.com/chriskohlhoff/asio extra/src/asio
	pushd extra/src/asio
 	git checkout f05ccf18df816f8999dcc6449f65e520787899c6
	popd
fi

if [ ! -f extra/include/asio/asio.hpp ] ; then
	mkdir -p extra/include/asio
	cp -r extra/src/asio/asio/include/* extra/include/asio/
fi

echo "Done"

#-------------------------------------------------------------------------------
# libjpeg-turbo
#-------------------------------------------------------------------------------
if [ ! -f extra/src/libjpeg_turbo_1.3.x/configure.ac ] ; then
	mkdir -p extra/src
	rm -rf extra/src/libjpeg_turbo_1.3.x
	pushd extra/src
	svn co svn://svn.code.sf.net/p/libjpeg-turbo/code/branches/1.3.x -r 1397 libjpeg_turbo_1.3.x
	pushd libjpeg_turbo_1.3.x
	autoreconf -fiv
	popd
	popd
fi

if [ ! -f extra/build/libjpeg_turbo_1.3.x/buildOk ] ; then
	mkdir -p extra/build/libjpeg_turbo_1.3.x
	pushd extra/build/libjpeg_turbo_1.3.x
	../../src/libjpeg_turbo_1.3.x/configure \
		--prefix=$EXTRA_DIR

	$MAKE
	$MAKE install
	popd
	touch extra/build/libjpeg_turbo_1.3.x/buildOk
fi

#-------------------------------------------------------------------------------
# optionparser
#-------------------------------------------------------------------------------
if [ ! -f extra/src/optionparser-1.3/src/optionparser.h ] ; then
	mkdir -p extra/src
	rm -rf extra/src/optionparser-1.3
	wget http://sourceforge.net/projects/optionparser/files/optionparser-1.3.tar.gz/download -O optionparser-1.3.tar.gz
	tar xvlf optionparser-1.3.tar.gz -C extra/src
	rm optionparser-1.3.tar.gz
fi

if [ ! -f extra/include/optionparser/optionparser.h ] ; then
	mkdir -p extra/include/optionparser
	cp extra/src/optionparser-1.3/src/optionparser.h extra/include/optionparser/
fi

echo "Done"
