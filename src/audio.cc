#include "audio.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

Audio::Audio(string name) : name(name), buffer(buffer_size) {
  initAVCodec();
  initRtAudio();
}

Audio::~Audio() {
  avformat_close_input(&format_ctx);
  av_packet_free(&packet);
  av_frame_free(&frame);
  avcodec_free_context(&codec_ctx);
}

int dequeueFrame(void* output_buffer, void* input_buffer, unsigned int num_frames, double stream_time, RtAudioStreamStatus status, void* user_data) {
  if (status) {
    ERROR("stream underflow detected");
  }

  // dequeue audio frames from cyclic buffer
  Audio* audio = (Audio*) user_data;
  audio->buffer.dequeue((float*) output_buffer, std::min(2 * num_frames, (unsigned int) audio->buffer.size()));

  return audio->buffer.size() == 0;
}

void Audio::initAVCodec() {
  // open video file
  string video_filename = name + ".mp4";
  format_ctx = avformat_alloc_context();
  if (avformat_open_input(&format_ctx, ("videos/" + video_filename).c_str(), nullptr, nullptr) != 0) {
    ERROR("unable to open file \"" + video_filename + "\"");
  }
  if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
    ERROR("could not get stream info");
  }

  // find audio codec
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

  // copy codec params to codec context
  codec_ctx = avcodec_alloc_context3(codec);
  if (avcodec_parameters_to_context(codec_ctx, codec_params) < 0) {
    ERROR("failed to copy codec params to codec context");
  }

  // open codec
  if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
    ERROR("failed to open audio codec");
  }

  // allocate frame and packet
  frame = av_frame_alloc();
  packet = av_packet_alloc();

  // fill cyclic buffer to half-way
  update();
}

void Audio::initRtAudio() {
  if (rta.getDeviceCount() < 1) {
    ERROR("no audio devices found");
  }

  // open audio stream
  RtAudio::StreamParameters stream_params;
  stream_params.deviceId = rta.getDefaultOutputDevice();
  stream_params.nChannels = 2;
  stream_params.firstChannel = 0;
  unsigned int num_frames = 256;
  rta.openStream(&stream_params, nullptr, RTAUDIO_FLOAT32, sample_rate, &num_frames, &dequeueFrame, (void *) this);
}

void Audio::play() {
  rta.startStream();
}

void Audio::update() {
  // read packets and enqueue audio frames to cyclic buffer until half-way filled
  bool packet_found = readPacket();
  while (packet_found && (buffer.size() < buffer.maxSize() / 2)) {
    packet_found = readPacket();
  }
}

bool Audio::readPacket() {
  while (av_read_frame(format_ctx, packet) >= 0) {
    if (packet->stream_index == audio_index) {
      // packet is from audio stream
      decodePacket();
      return true;
    }

    av_packet_unref(packet);
  }

  return false;
}

void Audio::decodePacket() {
  // send packet to decoder
  int response = avcodec_send_packet(codec_ctx, packet);
  if (response < 0) {
    ERROR("failed to send packet to decoder");
  }

  while (response >= 0) {
    // get frame data from decoder
    response = avcodec_receive_frame(codec_ctx, frame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      ERROR("failed to receive frame from decoder");
    }
    if (response >= 0) {
      enqueueFrame();
    }
  }
}

void Audio::enqueueFrame() {
  float temp[2 * frame->nb_samples];
  for (int i = 0; i < frame->nb_samples; i++) {
    // left speaker
    temp[2 * i] = ((float*) frame->data[0])[i];
    // right speaker
    temp[2 * i + 1] = ((float*) frame->data[1])[i];
  }

  buffer.enqueue(temp, 2 * frame->nb_samples);
}
