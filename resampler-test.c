#include <speex/speex_resampler.h>
#include <samplerate.h>
#include <sndfile.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <libavresample/avresample.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

int main(void)
{
#define DELTA_SAMPLES (32 * 1024)
#define MIN_RATIO 2
#define MAX_RATIO 8
#define BASE_FREQ 22050
#define OUT_FREQ(ratio) (BASE_FREQ * ratio)

#ifdef TEST_ALIAS
#define TARGET_NAME "alias"
#elif defined(TEST_ALIAS_DOWN)
#undef MIN_RATIO
#undef MAX_RATIO
#define MIN_RATIO 2
#define MAX_RATIO 2
#undef OUT_FREQ
#define OUT_FREQ(ratio) (BASE_FREQ / ratio)
#define TARGET_NAME "alias_down"
#else
#define TARGET_NAME "impulse"
#endif

   for (unsigned ratio = MIN_RATIO; ratio <= MAX_RATIO; ratio++)
   {
      float test_signal[DELTA_SAMPLES] = { [DELTA_SAMPLES / 2] = 1.0f}; // Delta pulse.
      float out_buffer[DELTA_SAMPLES * MAX_RATIO];

#ifdef TEST_ALIAS
      static const double freqs[] = {0.80, 0.85, 0.90, 0.91, 0.92, 0.93, 0.94, 0.95, 0.96, 0.97, 0.98, 0.99, 0.995, 0.999};
      memset(test_signal, 0, sizeof(test_signal));
      for (unsigned f = 0; f < ARRAY_SIZE(freqs); f++)
      {
         for (unsigned i = 0; i < DELTA_SAMPLES; i++)
            test_signal[i] += (0.5 / ARRAY_SIZE(freqs)) * sin(M_PI * freqs[f] * i); // Make sure not to clip.
      }
#elif defined(TEST_ALIAS_DOWN)

      static const double freqs[] = {
         0.1, 0.2, 0.3, 0.4, // Should pass.
         0.501, 0.502, 0.503, 0.504, 0.505, 0.51, 0.52, 0.53, 0.54, 0.55, // Should be stopped completely.
      };
      memset(test_signal, 0, sizeof(test_signal));
      for (unsigned f = 0; f < ARRAY_SIZE(freqs); f++)
      {
         for (unsigned i = 0; i < DELTA_SAMPLES; i++)
            test_signal[i] += (0.5 / ARRAY_SIZE(freqs)) * sin(M_PI * freqs[f] * i); // Make sure not to clip.
      }
#endif

      // Speex
      for (unsigned qual = SPEEX_RESAMPLER_QUALITY_MIN; qual <= SPEEX_RESAMPLER_QUALITY_MAX; qual++)
      {
         memset(out_buffer, 0, sizeof(out_buffer));
         int err = 0;
         SpeexResamplerState *state = speex_resampler_init(1, BASE_FREQ, OUT_FREQ(ratio), qual, &err);
         assert(state);

         spx_uint32_t input  = DELTA_SAMPLES;
         spx_uint32_t output = DELTA_SAMPLES * ratio;
         speex_resampler_process_float(state, 0, test_signal, &input, out_buffer, &output);

         char out_name[PATH_MAX];
         snprintf(out_name, sizeof(out_name), TARGET_NAME "_speex_q%u_%u.wav", qual, ratio);

         SNDFILE *wav = sf_open(out_name, SFM_WRITE,
               &(struct SF_INFO) {
               .samplerate = OUT_FREQ(ratio),
               .channels = 1,
               .format = SF_FORMAT_WAV | SF_FORMAT_FLOAT,
               });

         assert(wav);
         sf_write_float(wav, out_buffer, output);
         sf_close(wav);

         speex_resampler_destroy(state);
      }

      // libsamplerate
      for (unsigned qual = 0; qual < 3; qual++)
      {
         memset(out_buffer, 0, sizeof(out_buffer));
         SRC_DATA src = {
            .data_in  = test_signal,
            .data_out = out_buffer,
            .input_frames = DELTA_SAMPLES,
            .output_frames = DELTA_SAMPLES * ratio,
            .src_ratio = (double)OUT_FREQ(ratio) / BASE_FREQ,
         };

         static const int quals[] = {SRC_SINC_FASTEST, SRC_SINC_MEDIUM_QUALITY, SRC_SINC_BEST_QUALITY};
         static const char *names[] = {"fastest", "medium", "best"};
         src_simple(&src, quals[qual], 1);

         char out_name[PATH_MAX];
         snprintf(out_name, sizeof(out_name), TARGET_NAME "_src_%s_%u.wav", names[qual], ratio);
         SNDFILE *wav = sf_open(out_name, SFM_WRITE,
               &(struct SF_INFO) {
               .samplerate = OUT_FREQ(ratio),
               .channels = 1,
               .format = SF_FORMAT_WAV | SF_FORMAT_FLOAT,
               });

         assert(wav);
         sf_write_float(wav, out_buffer, src.output_frames_gen);
         sf_close(wav);
      }

      // libavresample
      {
         memset(out_buffer, 0, sizeof(out_buffer));
         AVAudioResampleContext *avr = avresample_alloc_context();
         assert(avr);
         av_opt_set_int(avr, "in_channel_layout", AV_CH_LAYOUT_MONO, 0);
         av_opt_set_int(avr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
         av_opt_set_int(avr, "in_sample_rate", BASE_FREQ, 0);
         av_opt_set_int(avr, "out_sample_rate", OUT_FREQ(ratio), 0);
         av_opt_set_int(avr, "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
         av_opt_set_int(avr, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
         av_opt_set_int(avr, "force_resampling", 1, 0);

         assert(avresample_open(avr) == 0);

         int written = avresample_convert(avr, (uint8_t*[]) { (uint8_t*)out_buffer }, OUT_FREQ(ratio), OUT_FREQ(ratio),
               (uint8_t*[]) { (uint8_t*)test_signal }, DELTA_SAMPLES, DELTA_SAMPLES);

         avresample_close(avr);
         avresample_free(&avr);

         char out_name[PATH_MAX];
         snprintf(out_name, sizeof(out_name), TARGET_NAME "_avr_%u.wav", ratio);
         SNDFILE *wav = sf_open(out_name, SFM_WRITE,
               &(struct SF_INFO) {
               .samplerate = OUT_FREQ(ratio),
               .channels = 1,
               .format = SF_FORMAT_WAV | SF_FORMAT_FLOAT,
               });

         assert(wav);
         sf_write_float(wav, out_buffer, written);
         sf_close(wav);
      }

      // libswresample (SWR + SoX)
      for (unsigned type = 0; type < 2; type++)
      {
         memset(out_buffer, 0, sizeof(out_buffer));
         SwrContext *swr = swr_alloc();
         assert(swr);
         av_opt_set_int(swr, "in_channel_layout", AV_CH_LAYOUT_MONO, 0);
         av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
         av_opt_set_int(swr, "in_sample_rate", BASE_FREQ, 0);
         av_opt_set_int(swr, "out_sample_rate", OUT_FREQ(ratio), 0);
         av_opt_set_int(swr, "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
         av_opt_set_int(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
         av_opt_set_int(swr, "force_resampling", 1, 0);

         static const char *names[] = {"swr", "soxr"};
         static const int engines[] = {SWR_ENGINE_SWR, SWR_ENGINE_SOXR};
         av_opt_set_int(swr, "resampler", engines[type], 0);

         assert(swr_init(swr) == 0);

         int written = swr_convert(swr, (uint8_t*[]) { (uint8_t*)out_buffer }, DELTA_SAMPLES * ratio,
               (const uint8_t*[]) { (const uint8_t*)test_signal }, DELTA_SAMPLES);

         swr_free(&swr);

         char out_name[PATH_MAX];
         snprintf(out_name, sizeof(out_name), TARGET_NAME "_swr_%s_%u.wav", names[type], ratio);
         SNDFILE *wav = sf_open(out_name, SFM_WRITE,
               &(struct SF_INFO) {
               .samplerate = OUT_FREQ(ratio),
               .channels = 1,
               .format = SF_FORMAT_WAV | SF_FORMAT_FLOAT,
               });

         assert(wav);
         sf_write_float(wav, out_buffer, written);
         sf_close(wav);
      }

      // Shibatch, only available as CLI tool.
      // Need a really long impulse response as SSRC will crap out otherwise.
      {
         SNDFILE *wav = sf_open("impulse_test.wav", SFM_WRITE,
               &(struct SF_INFO) {
               .samplerate = BASE_FREQ,
               .channels = 1,
               .format = SF_FORMAT_WAV | SF_FORMAT_PCM_24, // SSRC only supports up to 24-bit integer.
               });

         assert(wav);
         sf_write_float(wav, test_signal, DELTA_SAMPLES);
         sf_close(wav);

         char out_name[PATH_MAX];
         snprintf(out_name, sizeof(out_name), TARGET_NAME "_ssrc_%u.wav", ratio);
         char cmd[PATH_MAX];
         snprintf(cmd, sizeof(cmd), "ssrc --rate %u --bits 24 \"%s\" \"%s\"", OUT_FREQ(ratio),
               "impulse_test.wav", out_name);
         fprintf(stderr, "SSRC CMD: %s\n", cmd);
         int ret = system(cmd);
         assert(ret == 0);
      }
   }
}

