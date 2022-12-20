#include "audio.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

Audio::Audio(string name) : name(name), buffer(100000) {
  initAVCodec();
  initRtAudio();
}

Audio::~Audio() {
  avformat_close_input(&pFormatContext);
  av_packet_free(&pPacket);
  av_frame_free(&pFrame);
  avcodec_free_context(&pCodecContext);
}

int callback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void* userData) {
  if (status) {
    ERROR("stream underflow detected");
  }
  Audio* audio = (Audio*) userData;
  audio->buffer.dequeue((float*) outputBuffer, std::min(2 * nBufferFrames, (unsigned int) audio->buffer.size()));
  return audio->buffer.size() == 0;
}

void Audio::initAVCodec() {
  string video_filename = name + ".mp4";
  pFormatContext = avformat_alloc_context();
  if (avformat_open_input(&pFormatContext, ("videos/" + video_filename).c_str(), nullptr, nullptr) != 0) {
    ERROR("unable to open file \"" + video_filename + "\"");
  }
  if (avformat_find_stream_info(pFormatContext, nullptr) < 0) {
    ERROR("could not get the stream info");
  }
  const AVCodec* pCodec = nullptr;
  AVCodecParameters* pCodecParameters = nullptr;
  audio_index = -1;
  for (int i = 0; i < pFormatContext->nb_streams; i++) {
    AVCodecParameters* pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
    const AVCodec* pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
    if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
      pCodec = pLocalCodec;
      pCodecParameters = pLocalCodecParameters;
      sample_rate = pCodecParameters->sample_rate;
      audio_index = i;
    }
  }
  if (audio_index == -1) {
    ERROR("could not find audio codec");
  }
  pCodecContext = avcodec_alloc_context3(pCodec);
  if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
    ERROR("failed to copy codec params to codec context");
  }
  if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0) {
    ERROR("failed to open codec");
  }
  pFrame = av_frame_alloc();
  pPacket = av_packet_alloc();
  av_log_set_level(AV_LOG_ERROR);
  update();
}

void Audio::initRtAudio() {
  if (rta.getDeviceCount() < 1) {
    ERROR("no audio devices found");
  }
  RtAudio::StreamParameters parameters;
  parameters.deviceId = rta.getDefaultOutputDevice();
  parameters.nChannels = 2;
  parameters.firstChannel = 0;
  unsigned int sampleRate = 44100;
  unsigned int bufferFrames = 256;
  rta.openStream(&parameters, nullptr, RTAUDIO_FLOAT32, sample_rate, &bufferFrames, &callback, (void *) this);
}

void Audio::play() {
  rta.startStream();
}

void Audio::update() {
  while (buffer.size() < buffer.maxSize() / 2 && readPacket()) {}
}

bool Audio::readPacket() {
  bool found = false;
  while (av_read_frame(pFormatContext, pPacket) >= 0) {
    // if it's the audio stream
    if (pPacket->stream_index == audio_index) {
      found = true;
      decodePacket();
      break;
    }
    av_packet_unref(pPacket);
  }
  return found;
}

void Audio::decodePacket() {
  // packet data -> decoder
  int response = avcodec_send_packet(pCodecContext, pPacket);
  if (response < 0) {
    ERROR("failed to send packet to the decoder");
  }
  while (response >= 0) {
    // decoder -> frame data
    response = avcodec_receive_frame(pCodecContext, pFrame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      ERROR("failed to receive frame from the decoder");
    }
    if (response >= 0) {
      extractFrameData();
    }
  }
}

void Audio::extractFrameData() {
  float temp[2 * pFrame->nb_samples];
  for (int i = 0; i < pFrame->nb_samples; i++) {
    // left speaker
    temp[2 * i] = ((float*) pFrame->data[0])[i];
    // right speaker
    temp[2 * i + 1] = ((float*) pFrame->data[1])[i];
  }
  buffer.enqueue(temp, 2 * pFrame->nb_samples);
}

template <typename T>
Audio::CyclicQueue<T>::CyclicQueue(int max_size) : max_size(max_size), front(0), back(0) {
  arr = new T[max_size];
}

template <typename T>
Audio::CyclicQueue<T>::~CyclicQueue() {
  delete[] arr;
}

template <typename T>
void Audio::CyclicQueue<T>::enqueue(T input[], int n) {
  for (int i = 0; i < n; i++) {
    arr[(back + i) % max_size] = input[i];
  }
  back = (back + n) % max_size;
}

template <typename T>
void Audio::CyclicQueue<T>::dequeue(T output[], int n) {
  for (int i = 0; i < n; i++) {
     output[i] = arr[(front + i) % max_size];
  }
  front = (front + n) % max_size;
}

template <typename T>
int Audio::CyclicQueue<T>::size() {
  return (back - front + max_size) % max_size;
}

template <typename T>
int Audio::CyclicQueue<T>::maxSize() {
  return max_size;
}
