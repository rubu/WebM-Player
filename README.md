# Dependencies #

## libvpx ##

Currently libvpx-1.7.0 release is used

### MacOS ###

./configure --enable-pic --disable-install-bins --disable-examples --disable-tools --disable-docs --enable-runtime-cpu-detect --target=x86_64-darwin13-gcc --extra-cxxflags="-stdlib=libc++" --disable-webm-io 

### Windows ###