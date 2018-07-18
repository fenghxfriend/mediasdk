# Set these variables to suit your needs
NDK_PATH=/home/john/Android/Sdk/ndk-bundle
BUILD_PLATFORM="linux-x86_64"
TOOLCHAIN_VERSION="4.9"
ANDROID_VERSION="18"

# It should not be necessary to modify the rest
HOST=arm-linux-androideabi
SYSROOT=${NDK_PATH}/platforms/android-${ANDROID_VERSION}/arch-arm
ANDROID_CFLAGS="-march=armv7-a -mfloat-abi=softfp -fprefetch-loop-arrays \
  -D__ANDROID_API__=${ANDROID_VERSION} --sysroot=${SYSROOT} \
  -isystem ${NDK_PATH}/sysroot/usr/include \
  -isystem ${NDK_PATH}/sysroot/usr/include/${HOST}"

TOOLCHAIN=${NDK_PATH}/toolchains/${HOST}-${TOOLCHAIN_VERSION}/prebuilt/${BUILD_PLATFORM}
export CPP=${TOOLCHAIN}/bin/${HOST}-cpp
export AR=${TOOLCHAIN}/bin/${HOST}-ar
export NM=${TOOLCHAIN}/bin/${HOST}-nm
export CC=${TOOLCHAIN}/bin/${HOST}-gcc
export LD=${TOOLCHAIN}/bin/${HOST}-ld
export RANLIB=${TOOLCHAIN}/bin/${HOST}-ranlib
export OBJDUMP=${TOOLCHAIN}/bin/${HOST}-objdump
export STRIP=${TOOLCHAIN}/bin/${HOST}-strip
cd ../build/jpeg
sh ../../libjpeg-turbo/configure --host=${HOST} \
  CFLAGS="${ANDROID_CFLAGS} -O3 -fPIE" \
  CPPFLAGS="${ANDROID_CFLAGS}" \
  LDFLAGS="${ANDROID_CFLAGS} -pie" --with-simd ${1+"$@"}
make
