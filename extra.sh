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

if [ -z "$var" ]; then 
	if [[ "$OSTYPE" == "linux-gnu" ]]; then
		CPREFIX=x86_64-linux-gnu
	elif [[ "$OSTYPE" == "msys" ]]; then
		CPREFIX=x86_64-w64-mingw32
	else
		echo "Unknown platform. Please specify manually your compiler prefix with the CPREFIX environment variable."
		exit 1
	fi
fi
echo "Using compiler host prefix: $CPREFIX"

#-------------------------------------------------------------------------------
# zenbuild
#-------------------------------------------------------------------------------
if [ ! -f extra/src/zenbuild/zenbuild.sh ] ; then
	mkdir -p extra/src
	rm -rf extra/src/zenbuild
	git clone https://github.com/gpac/zenbuild extra/src/zenbuild
	pushd extra/src/zenbuild
	git checkout e3886072c464f73
	patch -p1 < ../../patches/gpac_01_revision.diff
	popd
fi

if [ ! -f extra/src/zenbuild/zenbuild.built ] ; then
	## x264
	if [ ! -f extra/build/flags/$CPREFIX/x264.built ] ; then
		pushd extra/src/zenbuild
		./zenbuild.sh "$PWD/../../../extra/build" x264 $CPREFIX
		popd
	fi
	## vo-aacenc
	if [ ! -f extra/build/flags/$CPREFIX/voaac-enc.built ] ; then
		pushd extra/src/zenbuild
		./zenbuild.sh "$PWD/../../../extra/build" voaac-enc $CPREFIX
		popd
	fi
	## FFmpeg
	if [ ! -f extra/build/flags/$CPREFIX/ffmpeg.built ] ; then
		pushd extra/src/zenbuild
		./zenbuild.sh "$PWD/../../../extra/build" ffmpeg $CPREFIX
		popd
	fi
	## GPAC
	if [ ! -f extra/build/flags/$CPREFIX/gpac.built ] ; then
		pushd extra/src/zenbuild
		./zenbuild.sh "$PWD/../../../extra/build" gpac $CPREFIX
		popd
	fi
	## SDL2
	if [ ! -f extra/build/flags/$CPREFIX/libsdl2.built ] ; then
		pushd extra/src/zenbuild
		./zenbuild.sh "$PWD/../../../extra/build" libsdl2 $CPREFIX
		popd
	fi
	## move files
	rsync -ar --remove-source-files extra/build/release/$CPREFIX/* extra/
	touch extra/src/zenbuild/zenbuild.built
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
