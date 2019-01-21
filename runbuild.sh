#! /bin/bash

# Assumes, and requires, user to launch from the same folder as this script.
TOP_DIR="$PWD"
BUILD_ARCH="$1"
BUILD_TYPE="$2"
PI_TOOLCHAIN_FILE="~/dev/Toolchain-rpi.cmake"

case "$BUILD_ARCH" in
x86)
	;;
pi)
	TOOLCHAIN="$PI_TOOLCHAIN_FILE"
	;;
*)
	echo "Please use \"x86\" or \"pi\" as first architecture argument."
	exit 1
	;;
esac

case "$BUILD_TYPE" in
Debug|Release)
	;;
*)
	echo "Please use \"Release\" or \"Debug\" as second build type argument."
	exit 1
	;;
esac

echo "Building for $BUILD_ARCH $BUILD_TYPE."

BUILD_FOLDER="build/$BUILD_ARCH/$BUILD_TYPE"
if [ ! -d "$BUILD_FOLDER" ]; then
	mkdir -p "$BUILD_FOLDER"
fi
pushd "$BUILD_FOLDER"
cmake "$TOP_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" -DCMAKE_INSTALL_PREFIX="$TOP_DIR"
if [ $? -eq 0 ]; then
    make -j4 package
    if [ $? -eq 0 ] && [ "$BUILD_ARCH" = "pi" ]; then
    	echo "Attempting to upload to pi:"
    	scp *.deb pi: || echo "Could not upload to RPi."
    fi
fi
popd