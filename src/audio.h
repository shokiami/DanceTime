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
  friend int callback(void* outputBuffer, void* inputBuffer,
                      unsigned int nBufferFrames, double streamTime,
                      RtAudioStreamStatus status, void* userData);

  private:
  string name;
  RtAudio rta;
  AVFormatContext *pFormatContext;
  AVCodecContext *pCodecContext;
  AVFrame* pFrame;
  AVPacket* pPacket;
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