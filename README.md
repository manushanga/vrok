Vrok - smokin' audio
======================

* Plays FLAC, MPEG and Ogg Vorbis
* Shibatch SuperEQ with Self Learning
* Windows and Linux(PulseAudio and ALSA)
* Gapless

Compiling
---------

Make sure you have these dependencies,

* ALSA development headers
* libFLAC and headers
* libmpg123 and headers
* libogg and headers
* libvorbis and headers
* libpulse-simple for PulseAudio support

Run these commands in the project directory

<pre>
qmake CONFIG+=Alsa || CONFIG+=PulseAudio  [ CONFIG+=RELEASE || CONFIG+=DEBUG || CONFIG+=console ]
make
</pre>

On Windows DSound is selected automatically. On Linux you must specify what to use.

Running
-------
<pre>
./vrok
</pre>

License
-------

Distributed under the terms of GPL 2.0, for more see LICENSE.
