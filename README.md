Vrok - smokin' audio
======================

* Plays FLAC, MPEG and Ogg Vorbis through their native decoders and plays other formats via FFmpeg
* Shibatch SuperEQ with Self Learning
* Windows and Linux(PulseAudio and ALSA)
* Gapless

Compiling
---------

Linux
-----
Make sure you have these dependencies,

* ALSA development headers
* libFLAC and headers
* libmpg123 and headers
* libogg and headers
* libvorbis and headers
* libpulse-simple for PulseAudio support
* FFmpeg

Run these commands in the project directory(use must have qmake-qt4 or qmake-qt5
installed, with all the development libraries. On Linux vrok will build with GCC and
Clang.

<pre>
qmake CONFIG+=Alsa || CONFIG+=PulseAudio  [ CONFIG+=RELEASE || CONFIG+=DEBUG || CONFIG+=console ]
make
</pre>

Running
-------
<pre>
./vrok
</pre>

Windows
-------

Windows build now builds with MSVC, because of a few mising compiler checks in
code building with MinGW will be broken at the moment. It'll be fixed in the future.

For the build you need to build these libraries separately, DSound output and Ao
(libao from Xiph Foundation) are supported. If you intend to use Ao output compile
it too. I recommend DSound, it's the most tested output plugin on Windows.

* libFLAC
* libmpg123
* libogg
* libvorbis
* FFmpeg

You can use QtCreator on Windows set up with MSVC to build or you can use MS Visual
Studio with the Qt addin from qt-project.org to compile for Windows. Make sure
you change the library path in the .pro file before runnnig qmake.

<pre>
qmake CONFIG+DSound || CONFIG+=Ao
</pre>

License
-------

Distributed under the terms of GPL 2.0, for more see LICENSE.
