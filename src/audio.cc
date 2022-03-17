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
  rta.stopStream();
  avformat_close_input(&pFormatContext);
  av_packet_free(&pPacket);
  av_frame_free(&pFrame);
  avcodec_free_context(&pCodecContext);
}

int callback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void* userData) {
  if (status) {
    cerr << "ERROR: Stream underflow detected." << endl;
    return EXIT_FAILURE;
  }

  Audio* audio = (Audio*) userData;

  if (2 * nBufferFrames > audio->buffer.getSize()) {
    cerr << "ERROR: Not enough audio data in buffer." << endl;
    return EXIT_FAILURE;
  }

  audio->buffer.dequeue((float*) outputBuffer, 2 * nBufferFrames);

  return EXIT_SUCCESS;
}

void Audio::initAVCodec() {
  string video_filename = name + ".mp4";

  pFormatContext = avformat_alloc_context();
  if (avformat_open_input(&pFormatContext, ("videos/" + video_filename).c_str(), nullptr, nullptr) != 0) {
    cerr << "ERROR: Unable to open file " << video_filename << " in Audio::initAVCodec()." << endl;
    exit(EXIT_FAILURE);
  }

  if (avformat_find_stream_info(pFormatContext, nullptr) < 0) {
    cerr << "ERROR: Could not get the stream info in Audio::initAVCodec()." << endl;
    exit(EXIT_FAILURE);
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
    cerr << "ERROR: Could not find audio codec in Audio::initAVCodec()." << endl;
    exit(EXIT_FAILURE);
  }

  pCodecContext = avcodec_alloc_context3(pCodec);
  if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
    cerr << "ERROR: Failed to copy codec params to codec context in Audio::initAVCodec()." << endl;
    exit(EXIT_FAILURE);
  }

  if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0) {
    cerr << "ERROR: Failed to open codec in Audio::initAVCodec()." << endl;
    exit(EXIT_FAILURE);
  }

  pFrame = av_frame_alloc();
  pPacket = av_packet_alloc();

  av_log_set_level(AV_LOG_ERROR);

  update();
}

void Audio::initRtAudio() {
  if (rta.getDeviceCount() < 1) {
    cerr << "ERROR: No audio devices found in Audio::initRtAudio()." << endl;
    exit(EXIT_FAILURE);
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
  while (buffer.getSize() < buffer.getTotalSize() / 2) {
    readPacket();
  }
}

void Audio::readPacket() {
  while (av_read_frame(pFormatContext, pPacket) >= 0) {
    // If it's the audio stream.
    if (pPacket->stream_index == audio_index) {
      decodePacket();
      break;
    }
    av_packet_unref(pPacket);
  }
}


void Audio::decodePacket() {
  // Supply raw packet data as input to a decoder
  int response = avcodec_send_packet(pCodecContext, pPacket);

  if (response < 0) {
    cerr << "ERROR: Failed to send packet to the decoder in Audio::decodePacket()." << endl;
    exit(EXIT_FAILURE);
  }

  while (response >= 0) {
    // Return decoded output data (into a frame) from a decoder
    response = avcodec_receive_frame(pCodecContext, pFrame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      cerr << "ERROR: Failed to receive frame from the decoder in Audio::decodePacket()." << endl;
      exit(EXIT_FAILURE);
    }

    if (response >= 0) {
      extractFrameData();
    }
  }
}

void Audio::extractFrameData() {
  float temp[2 * pFrame->nb_samples];
  for (int i = 0; i < pFrame->nb_samples; i++) {
    // Left speaker.
    temp[2 * i] = ((float*)pFrame->data[0])[i];
    // Right speaker.
    temp[2 * i + 1] = ((float*)pFrame->data[1])[i];
  }
  buffer.enqueue(temp, 2 * pFrame->nb_samples);
}

template <typename T>
Audio::CyclicQueue<T>::CyclicQueue(int size) : size(size), front(0), back(0) {
  arr = new T[size];
}

template <typename T>
Audio::CyclicQueue<T>::~CyclicQueue() {
  delete[] arr;
}

template <typename T>
void Audio::CyclicQueue<T>::enqueue(T input[], int n) {
  for (int i = 0; i < n; i++) {
    arr[(back + i) % size] = input[i];
  }
  back = (back + n) % size;
}

template <typename T>
void Audio::CyclicQueue<T>::dequeue(T output[], int n) {
  for (int i = 0; i < n; i++) {
     output[i] = arr[(front + i) % size];
  }
  front = (front + n) % size;
}

template <typename T>
int Audio::CyclicQueue<T>::getSize() {
  return (back - front + size) % size;
}

template <typename T>
int Audio::CyclicQueue<T>::getTotalSize() {
  return size;
}
