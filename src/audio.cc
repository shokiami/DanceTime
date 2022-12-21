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
  avformat_close_input(&format_ctx);
  av_packet_free(&packet);
  av_frame_free(&frame);
  avcodec_free_context(&codec_ctx);
}

int callback(void* output_buffer, void* input_buffer, unsigned int num_frames, double stream_time, RtAudioStreamStatus status, void* user_data) {
  if (status) {
    ERROR("stream underflow detected");
  }
  Audio* audio = (Audio*) user_data;
  audio->buffer.dequeue((float*) output_buffer, std::min(2 * num_frames, (unsigned int) audio->buffer.size()));
  return audio->buffer.size() == 0;
}

void Audio::initAVCodec() {
  string video_filename = name + ".mp4";
  format_ctx = avformat_alloc_context();
  if (avformat_open_input(&format_ctx, ("videos/" + video_filename).c_str(), nullptr, nullptr) != 0) {
    ERROR("unable to open file \"" + video_filename + "\"");
  }
  if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
    ERROR("could not get the stream info");
  }
  const AVCodec* codec = nullptr;
  AVCodecParameters* codec_params = nullptr;
  audio_index = -1;
  for (int i = 0; i < format_ctx->nb_streams; i++) {
    AVCodecParameters* curr_codec_params = format_ctx->streams[i]->codecpar;
    const AVCodec* curr_codec = avcodec_find_decoder(curr_codec_params->codec_id);
    if (curr_codec_params->codec_type == AVMEDIA_TYPE_AUDIO) {
      codec = curr_codec;
      codec_params = curr_codec_params;
      sample_rate = codec_params->sample_rate;
      audio_index = i;
    }
  }
  if (audio_index == -1) {
    ERROR("could not find audio codec");
  }
  codec_ctx = avcodec_alloc_context3(codec);
  if (avcodec_parameters_to_context(codec_ctx, codec_params) < 0) {
    ERROR("failed to copy codec params to codec context");
  }
  if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
    ERROR("failed to open codec");
  }
  frame = av_frame_alloc();
  packet = av_packet_alloc();
  av_log_set_level(AV_LOG_ERROR);
  update();
}

void Audio::initRtAudio() {
  if (rta.getDeviceCount() < 1) {
    ERROR("no audio devices found");
  }
  RtAudio::StreamParameters stream_params;
  stream_params.deviceId = rta.getDefaultOutputDevice();
  stream_params.nChannels = 2;
  stream_params.firstChannel = 0;
  unsigned int num_frames = 256;
  rta.openStream(&stream_params, nullptr, RTAUDIO_FLOAT32, sample_rate, &num_frames, &callback, (void *) this);
}

void Audio::play() {
  rta.startStream();
}

void Audio::update() {
  bool packet_found = readPacket();
  while (packet_found && (buffer.size() < buffer.maxSize() / 2)) {
    packet_found = readPacket();
  }
}

bool Audio::readPacket() {
  while (av_read_frame(format_ctx, packet) >= 0) {
    // if it's the audio stream
    if (packet->stream_index == audio_index) {
      decodePacket();
      return true;
    }
    av_packet_unref(packet);
  }
  return false;
}

void Audio::decodePacket() {
  // packet data -> decoder
  int response = avcodec_send_packet(codec_ctx, packet);
  if (response < 0) {
    ERROR("failed to send packet to the decoder");
  }
  while (response >= 0) {
    // decoder -> frame data
    response = avcodec_receive_frame(codec_ctx, frame);
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
  float temp[2 * frame->nb_samples];
  for (int i = 0; i < frame->nb_samples; i++) {
    // left speaker
    temp[2 * i] = volume * ((float*) frame->data[0])[i];
    // right speaker
    temp[2 * i + 1] = volume * ((float*) frame->data[1])[i];
  }
  buffer.enqueue(temp, 2 * frame->nb_samples);
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
