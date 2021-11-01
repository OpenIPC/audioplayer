#include <assert.h>
#include <stdbool.h>

#include <lame/lame.h>
#include <stdio.h>
#include <stdlib.h>

static void hip_debug(const char *format, va_list ap) { printf(format, ap); }

static void hip_error(const char *format, va_list ap) { printf(format, ap); }

static void hip_info(const char *format, va_list ap) { printf(format, ap); }

static void chk_hip_error(int ret) { assert(ret != -1); }

hip_t dec;

static bool init() {
  dec = hip_decode_init();
  if (NULL == dec) {
    printf("hip_decode_init failed.\n");
    return false;
  }

  hip_set_errorf(dec, hip_error);
  hip_set_debugf(dec, hip_debug);
  hip_set_msgf(dec, hip_info);

  return true;
}

static void deinit() {
  int ret = hip_decode_exit(dec);
  chk_hip_error(ret);
}

void write_samples_simple(short *pcm_l, short *pcm_r, int len, FILE *out) {
  for (int i = 0; i < len; i++) {
    fwrite(pcm_l + i, 1, sizeof(short), out);
    fwrite(pcm_r + i, 1, sizeof(short), out);
  }
}

void write_samples(short *pcm_l, short *pcm_r, int len, FILE *out) {
  short *sbuf = alloca(sizeof(short) * len * 2);
  for (int i = 0; i < len; i++) {
    sbuf[i * 2] = pcm_l[i];
    sbuf[i * 2 + 1] = pcm_r[i];
  }

  fwrite(sbuf, 1, sizeof(short) * len * 2, out);
  // fwrite(pcm_l, 1, sizeof(short)*len, out);
  // fwrite(pcm_r, 1, sizeof(short)*len, out);
}

int main() {
  init();

  FILE *f = fopen("pet-shop-boys-go-west.mp3", "rb");
  FILE *out = fopen("out.pcm", "wb");
  assert(f);

  unsigned char mp3buf[4096];
  short pcm_l[0x10000], pcm_r[0x10000];
  int nread;

  fseek(f, 0x00001f00, SEEK_SET);

  while ((nread = fread(mp3buf, 1, sizeof(mp3buf), f))) {
    int samples = hip_decode(dec, mp3buf, nread, pcm_l, pcm_r);
    if (samples) {
      write_samples(pcm_l, pcm_r, samples, out);
      // fwrite(pcm_l, 1, samples, out);
      // fwrite(pcm_r, 1, samples, out);
      // printf("%d\n", samples);
    }
  }

  fclose(out);
  fclose(f);

  deinit();
}
