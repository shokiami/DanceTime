#ifndef AUDIO_H_
#define AUDIO_H_

#include "rtaudio/RtAudio.h"
extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
}
#include "defs.h"

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
  void initAVCodec();
  void initRtAudio();
  bool readPacket();
  void decodePacket();
  void extractFrameData();

  template <typename T>
  class CyclicQueue {
    public:
    CyclicQueue(int size);
    ~CyclicQueue();
    void enqueue(T input[], int n);
    void dequeue(T output[], int n);
    int size();
    int maxSize();

    private:
    T* arr;
    int max_size;
    int front;
    int back;
  };
  CyclicQueue<float> buffer;
};

#endif
