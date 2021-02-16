// Launches a window for rendering then closes window and exits program

#include <mana/core/memoryallocator.h>
//
#include <mana/mana.h>
#define SOUNDIO_STATIC_LIBRARY
#include <sndfile.h>
#include <soundio.h>

struct PassThrough {
  SF_INFO sfinfo;
  SNDFILE* infile;
};

static float seconds_offset = 0.0f;
static void write_callback(struct SoundIoOutStream* outstream, int frame_count_min, int frame_count_max) {
  const struct SoundIoChannelLayout* layout = &outstream->layout;
  struct SoundIoChannelArea* areas;
  int frames_left = frame_count_max;
  int err;

  struct PassThrough* pass = outstream->userdata;
  SNDFILE* infile = pass->infile;
  SF_INFO sfinfo = pass->sfinfo;
  int channels = sfinfo.channels;

  float float_sample_rate = (float)outstream->sample_rate;
  float seconds_per_frame = 1.0f / float_sample_rate;

  while (frames_left > 0) {
    int frame_count = frames_left;

    if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
      fprintf(stderr, "%s\n", soundio_strerror(err));
      exit(1);
    }

    if (!frame_count)
      break;

    float* buff = calloc(channels * frame_count, sizeof(float));

    int readcount;
    sf_count_t frames;

    frames = frame_count;

    sf_seek(infile, seconds_offset * (float_sample_rate * ((float)sfinfo.samplerate / float_sample_rate)), SEEK_SET);
    readcount = sf_readf_float(infile, buff, frames);

    printf("Frame count: %d Read count: %d\n", frame_count, readcount);

    for (int frame = 0; frame < readcount; frame++) {
      for (int channel = 0; channel < channels; channel++) {
        // Mono
        //float sample = buff[(int)(seconds_offset + frame * float_sample_rate) + channel];
        // Stereo
        float sample = buff[(int)(seconds_offset + frame * ((float)sfinfo.samplerate / float_sample_rate)) * 2 + channel];
        float* ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
        *ptr = sample;
      }
    }
    seconds_offset = seconds_offset + seconds_per_frame * frame_count;

    if ((err = soundio_outstream_end_write(outstream))) {
      fprintf(stderr, "%s\n", soundio_strerror(err));
      exit(1);
    }

    frames_left -= frame_count;
  }
}

int main(int argc, char* argv[]) {
  setbuf(stdout, NULL);

  struct Mana mana = {0};
  mana_init(&mana, (struct EngineSettings){GLFW_LIBRARY, VULKAN_API});
  struct Window window = {0};
  window_init(&window, &mana.engine, 1280, 720);

  // Play audio file
  int err;
  struct SoundIo* soundio = soundio_create();
  if (!soundio) {
    fprintf(stderr, "out of memory\n");
    return 1;
  }

  if ((err = soundio_connect(soundio))) {
    fprintf(stderr, "error connecting: %s", soundio_strerror(err));
    return 1;
  }

  soundio_flush_events(soundio);

  int default_out_device_index = soundio_default_output_device_index(soundio);
  if (default_out_device_index < 0) {
    fprintf(stderr, "no output device found");
    return 1;
  }

  struct SoundIoDevice* device = soundio_get_output_device(soundio, default_out_device_index);
  if (!device) {
    fprintf(stderr, "out of memory");
    return 1;
  }

  fprintf(stderr, "Output device: %s\n", device->name);

  struct SoundIoOutStream* outstream = soundio_outstream_create(device);
  if (!outstream) {
    fprintf(stderr, "out of memory\n");
    return 1;
  }

  SNDFILE* infile = NULL;
  SF_INFO sfinfo;

  if ((infile = sf_open("assets/audio/music/Dad_n_Me.wav", SFM_READ, &sfinfo)) == NULL)
    return 1;

  struct PassThrough* pass = calloc(1, sizeof(struct PassThrough));
  pass->infile = infile;
  pass->sfinfo = sfinfo;

  outstream->userdata = pass;

  outstream->format = SoundIoFormatFloat32NE;
  outstream->write_callback = write_callback;

  if ((err = soundio_outstream_open(outstream))) {
    fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
    return 1;
  }

  if (outstream->layout_error)
    fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));

  if ((err = soundio_outstream_start(outstream))) {
    fprintf(stderr, "unable to start device: %s", soundio_strerror(err));
    return 1;
  }

  for (;;)
    soundio_wait_events(soundio);

  soundio_outstream_destroy(outstream);
  soundio_device_unref(device);
  soundio_destroy(soundio);
  // End of audio file code

  window_delete(&window);
  mana_cleanup(&mana);

  return 0;
}
