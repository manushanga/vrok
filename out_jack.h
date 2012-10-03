#ifndef OUT_JACK_H
#define OUT_JACK_H

#define JACK_SAMPLES 65536*8

int jack_init(char *name);
void jack_run(float *samples, unsigned count);
int jack_end();
unsigned jack_samplerate();
unsigned jack_channels();

#endif // OUT_JACK_H
