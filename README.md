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
qmake [ CONFIG+=PulseAudio || CONFIG+=DEBUG || CONFIG+=console ]
make
</pre>

Running
-------
<pre>
./vrok
</pre>

License
-------

Distributed under the terms of GPL 2.0, for more see LICENSE.
