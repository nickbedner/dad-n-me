// Launches a window for rendering then closes window and exits program
#undef __cplusplus

#include <mana/core/memoryallocator.h>
//
#include <mana/mana.h>
#define SOUNDIO_STATIC_LIBRARY
#include <sndfile.h>
#include <soundio.h>
#include <stdbool.h>

struct AudioClip {
  SF_INFO sfinfo;
  SNDFILE* infile;
  float seconds_offset;
  bool loop;
  float volume;
};

// Lower the value, the more accurate the audio, but risk overloading audio device queue
/*#define AUDIO_BUFFER 2 * 1024
static void write_callback(struct SoundIoOutStream* outstream, int frame_count_min, int frame_count_max) {
  // Needed to determine max output channels
  const struct SoundIoChannelLayout* layout = &outstream->layout;
  struct SoundIoChannelArea* areas;
  float float_sample_rate = (float)outstream->sample_rate;
  float seconds_per_frame = 1.0f / float_sample_rate;
  int err;

  struct ArrayList* audio_clips = outstream->userdata;

  int device_channels = layout->channel_count;
  int buffer_size = device_channels * AUDIO_BUFFER;

  if ((err = soundio_outstream_begin_write(outstream, &areas, &buffer_size))) {
    fprintf(stderr, "%s\n", soundio_strerror(err));
    exit(1);
  }

  if (!buffer_size)
    return;

  for (int audio_clip_num = 0; audio_clip_num < array_list_size(audio_clips); audio_clip_num++) {
    struct AudioClip* audio_clip = array_list_get(audio_clips, audio_clip_num);
    SNDFILE* infile = audio_clip->infile;
    SF_INFO sfinfo = audio_clip->sfinfo;
    soundio_outstream_set_volume(outstream, audio_clip->volume);
    int channels = sfinfo.channels;

    int frames_left = AUDIO_BUFFER;
    int readcount = 0;
    while (frames_left > 0) {
      float* buff = calloc(channels * frames_left, sizeof(float));
      sf_seek(infile, audio_clip->seconds_offset * (float_sample_rate * ((float)sfinfo.samplerate / float_sample_rate)), SEEK_SET);
      readcount += sf_readf_float(infile, buff + ((AUDIO_BUFFER - frames_left) * device_channels), frames_left);
      //printf("Frame count: %d Read count: %d\n", frames_left, readcount);

      for (int frame = 0; frame < readcount; frame++) {
        for (int channel = 0; channel < channels; channel++) {
          // Mono, only one speaker?
          //float sample = buff[(int)(seconds_offset + frame * float_sample_rate) + channel];
          // Stereo
          float sample = buff[(int)(audio_clip->seconds_offset + frame * ((float)sfinfo.samplerate / float_sample_rate)) * 2 + channel];
          float* ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
          *ptr = sample;
          if (*ptr > 1.0f)
            *ptr = 1.0f;
        }
      }

      audio_clip->seconds_offset = audio_clip->seconds_offset + seconds_per_frame * readcount;
      if (readcount < frames_left && audio_clip->loop == true)
        audio_clip->seconds_offset = 0.0f;
      else
        break;

      frames_left -= readcount;
      free(buff);
    }
  }

  if ((err = soundio_outstream_end_write(outstream))) {
    fprintf(stderr, "%s\n", soundio_strerror(err));
    exit(1);
  }
}*/

// Lower the value, the more accurate the audio, but risk overloading audio device queue
#define AUDIO_BUFFER 2 * 1024
static void write_callback(struct SoundIoOutStream* outstream, int frame_count_min, int frame_count_max) {
  // Needed to determine max output channels
  const struct SoundIoChannelLayout* layout = &outstream->layout;
  struct SoundIoChannelArea* areas;
  int err;

  struct ArrayList* audio_clips = outstream->userdata;
  struct AudioClip* audio_clip = array_list_get(audio_clips, 0);
  SNDFILE* infile = audio_clip->infile;
  SF_INFO sfinfo = audio_clip->sfinfo;
  int channels = sfinfo.channels;
  soundio_outstream_set_volume(outstream, audio_clip->volume);

  float float_sample_rate = (float)outstream->sample_rate;
  float seconds_per_frame = 1.0f / float_sample_rate;

  float* buff = calloc(channels * AUDIO_BUFFER, sizeof(float));

  int frames_left = AUDIO_BUFFER;
  int readcount = 0;
  while (audio_clip->loop == true && frames_left > 0) {
    sf_seek(infile, audio_clip->seconds_offset * (float_sample_rate * ((float)sfinfo.samplerate / float_sample_rate)), SEEK_SET);
    readcount += sf_readf_float(infile, buff + ((AUDIO_BUFFER - frames_left) * channels), frames_left);
    //printf("Frame count: %d Read count: %d\n", frames_left, readcount);

    audio_clip->seconds_offset = audio_clip->seconds_offset + seconds_per_frame * readcount;
    if (readcount < frames_left)
      audio_clip->seconds_offset = 0.0f;

    frames_left -= readcount;
  }

  if ((err = soundio_outstream_begin_write(outstream, &areas, &readcount))) {
    fprintf(stderr, "%s\n", soundio_strerror(err));
    exit(1);
  }

  if (!readcount)
    return;

  for (int frame = 0; frame < readcount; frame++) {
    for (int channel = 0; channel < channels; channel++) {
      // Mono, only one speaker?
      //float sample = buff[(int)(seconds_offset + frame * float_sample_rate) + channel];
      // Stereo
      float sample = buff[(int)(audio_clip->seconds_offset + frame * ((float)sfinfo.samplerate / float_sample_rate)) * 2 + channel];
      float* ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
      *ptr = sample;
    }
  }

  if ((err = soundio_outstream_end_write(outstream))) {
    fprintf(stderr, "%s\n", soundio_strerror(err));
    exit(1);
  }

  free(buff);
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

  struct ArrayList audio_clips = {0};
  array_list_init(&audio_clips);

  SNDFILE* infile = NULL;
  SF_INFO sfinfo;
  if ((infile = sf_open("assets/audio/music/Dad_n_Me.wav", SFM_READ, &sfinfo)) == NULL)
    return 1;
  struct AudioClip* audio_clip = calloc(1, sizeof(struct AudioClip));
  audio_clip->infile = infile;
  audio_clip->sfinfo = sfinfo;
  audio_clip->loop = true;
  audio_clip->volume = 0.75f;

  array_list_add(&audio_clips, audio_clip);

  outstream->userdata = &audio_clips;
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
