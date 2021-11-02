#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <lame/lame.h>

#include <hi_comm_adec.h>
#include <mpi_audio.h>
#include <mpi_sys.h>

#include "errors.h"

static void hip_debug(const char *format, va_list ap) { printf(format, ap); }

static void hip_error(const char *format, va_list ap) { printf(format, ap); }

static void hip_info(const char *format, va_list ap) { printf(format, ap); }

static void chk_hip_error(int ret) { assert(ret != -1); }

hip_t dec;

static bool init_decoder() {
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

static void deinit_decoder() {
  int ret = hip_decode_exit(dec);
  chk_hip_error(ret);
}

static AUDIO_DEV AoDev = 0;
static AO_CHN AoChn = 0;
static ADEC_CHN AdChn = 0;
#define SAMPLE_AUDIO_PTNUMPERFRM 320

static HI_S32 AUDIO_AoBindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn) {
  MPP_CHN_S stSrcChn, stDestChn;
  stSrcChn.enModId = HI_ID_ADEC;
  stSrcChn.s32DevId = 0;
  stSrcChn.s32ChnId = AdChn;
  stDestChn.enModId = HI_ID_AO;
  stDestChn.s32DevId = AoDev;
  stDestChn.s32ChnId = AoChn;
  return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}

static void init_hw() {
  int ret;

  ret = HI_MPI_SYS_Init();
  if (ret != HI_SUCCESS) {
    printf("error %s\n", hi_errstr(ret));
  }

  ADEC_ATTR_LPCM_S stAdecLpcm = {0};
  ADEC_CHN_ATTR_S stAdecAttr = {
      .enType = PT_LPCM,
      .u32BufSize = 20,
      .pValue = &stAdecLpcm,
      .enMode = ADEC_MODE_PACK,
  };
  ret = HI_MPI_ADEC_CreateChn(AdChn, &stAdecAttr);
  if (ret != HI_SUCCESS) {
    printf("1error %s\n", hi_errstr(ret));
  }

  AIO_ATTR_S stAioAttr = {
      .enSamplerate = 44100,
      .enBitwidth = AUDIO_BIT_WIDTH_16,
      .enWorkmode = AIO_MODE_I2S_MASTER,
      .enSoundmode = AUDIO_SOUND_MODE_MONO,
      .u32FrmNum = 30,
      .u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM,
      .u32ChnCnt = 1,
      .enI2sType = AIO_I2STYPE_INNERCODEC,
  };
  ret = HI_MPI_AO_SetPubAttr(AoDev, &stAioAttr);
  if (ret != HI_SUCCESS) {
    printf("error %s\n", hi_errstr(ret));
  }

  ret = HI_MPI_AO_Enable(AoDev);
  if (ret != HI_SUCCESS) {
    printf("error %s\n", hi_errstr(ret));
  }

  ret = HI_MPI_AO_EnableChn(AoDev, AdChn);
  if (ret != HI_SUCCESS) {
    printf("error %s\n", hi_errstr(ret));
  }

  ret = AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
  if (ret != HI_SUCCESS) {
    printf("error %s\n", hi_errstr(ret));
  }

  ret = HI_MPI_AO_SetVolume(AoDev, -10);
  if (ret != HI_SUCCESS) {
    printf("error %s\n", hi_errstr(ret));
  }
  printf("bind adec:%d to ao(%d,%d) ok \n", AdChn, AoDev, AoChn);
}

static void deinit_hw() {}

int main() {
  init_decoder();
  init_hw();

  FILE *f = fopen("pet-shop-boys-go-west.mp3", "rb");
  assert(f);

  unsigned char mp3buf[4096];
  short pcm_l[0x10000], pcm_r[0x10000];
  int nread;

  fseek(f, 0x00001f00, SEEK_SET);

  while ((nread = fread(mp3buf, 1, sizeof(mp3buf), f))) {

    int samples = hip_decode(dec, mp3buf, nread, pcm_l, pcm_r);
    unsigned char *sptr = (unsigned char *)pcm_l;
    unsigned char *end = sptr + samples * sizeof(short);
    while (sptr < end) {
      int len = end - sptr;
      if (len > 640)
        len = 640;
      AUDIO_STREAM_S stAudioStream = {
          .pStream = sptr,
          .u32Len = len,
      };
      int ret = HI_MPI_ADEC_SendStream(AdChn, &stAudioStream, HI_TRUE);
      if (ret != HI_SUCCESS) {
        printf("error %s\n", hi_errstr(ret));
      }

      ret = HI_MPI_ADEC_SendEndOfStream(AdChn, HI_FALSE);
      if (ret != HI_SUCCESS) {
        printf("error %s\n", hi_errstr(ret));
      }
      sptr += len;
    }
  }

  fclose(f);

  deinit_hw();
  deinit_decoder();
}

const unsigned short int *__ctype_b;

int __fgetc_unlocked(FILE *stream) { return fgetc(stream); }

size_t _stdlib_mb_cur_max(void) { return 0; }
