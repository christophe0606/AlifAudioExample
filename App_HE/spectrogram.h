#ifndef spectrogram_h
#define spectrogram_h

#define NB_BIN 32

extern int init_spectrogram();
extern void free_spectrogram();
extern void update_data();
extern void display_data();

extern void new_data(float *bins);

#endif