# About #

Some time ago while working on a video management system streaming frontend I had the need to mux VP8 files myself. To validate the result I made a simple SDL based player (https://github.com/rubu/webmplayer) which is now in a broken state. I could not use VLC or ffmpeg since when I had something wrong with the file layout I needed to find exactly where and what was wrong. So I made the player with stuff like EBML tree printing. 

Some time later I read a bit on AV1 and saw a post on SO (https://stackoverflow.com/questions/49984026/av1-video-frame-data-format-in-matroska-webm-block) about AV1 in WebM. I tried to patch the old player to validate AV1, but failed due to lack of valid samples. Then I wanted to add transcoding to my old player to decode VP8 and encode AV1. During that I got contacted by a person that asked me if I could unbreak the stuff in the old player and make it work for MacOS, so I made a new nice clean repo with the thought of moving from SDL to OpenGL and platform native UI for Windows/MacOS/Linux. Well, let's see if that works out:)

# Current state #

MacOS - can play VP8 and has a window for inspecting the EBML element tree. The current AV1 samples I've got my hands on did not work due to AV1 decoder returning invalid bitstream errors. Either I'm doing someting wrong of the files are bad.

Windows - need to make OpenGL work, no functionality is working for now.

Linux - nothing has been done:D

# Dependencies #

## libvpx ##

Currently libvpx-1.7.0 release is used

### MacOS ###

./configure --enable-pic --disable-install-bins --disable-examples --disable-tools --disable-docs --enable-runtime-cpu-detect --target=x86_64-darwin13-gcc --extra-cxxflags="-stdlib=libc++" --disable-webm-io 

### Windows ###

## av1 ##

Currently builds from the commit 265d15d46455ab4e208d3f9baaa840e05bd025fc are used

### MacOS ###

### Windows ###
