#ifndef spectrogram_h
#define spectrogram_h

#define NB_BIN 256

extern int init_spectrogram();
extern void free_spectrogram();
extern void display_data();

extern void new_data(float *new_bins, int left_channel);

#endif