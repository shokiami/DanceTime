#ifndef AUDIO_H_
#define AUDIO_H_

#include "defs.h"
#include "util.h"
#include "rtaudio/RtAudio.h"
extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
}

class Audio {
  public:
  Audio(string name);
  ~Audio();
  void play();
  void update();
  friend int callback(void* output_buffer, void* input_buffer, unsigned int num_frames, double stream_time, RtAudioStreamStatus status, void* user_data);

  private:
  string name;
  RtAudio rta;
  AVFormatContext* format_ctx;
  AVCodecContext* codec_ctx;
  AVFrame* frame;
  AVPacket* packet;
  int audio_index;
  int sample_rate;
  CyclicQueue<float> buffer;
  void initAVCodec();
  void initRtAudio();
  bool readPacket();
  void decodePacket();
  void extractFrameData();
  static constexpr int buffer_size = 100000;
  static constexpr double volume = 0.1;
};

#endif
