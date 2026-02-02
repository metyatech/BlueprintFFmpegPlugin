
#pragma once

#include "CoreMinimal.h"

extern "C" {
#include <libavutil/frame.h>
}

/**
 * wrapper for AVFrame of FFmpeg.
 * Threadsafe.
 */
template <ESPMode InMode>
struct BLUEPRINTFFMPEG_API TFFmpegFrameSharedPtr {
public:
	AVFrame* Get() const;

public:
	TFFmpegFrameSharedPtr();
	explicit TFFmpegFrameSharedPtr(AVFrame* InRawFrame);
	explicit operator bool() const;
	AVFrame& operator*() const;
	AVFrame* operator->() const;

private:
	TSharedPtr<AVFrame, InMode> RawFrameSharedPtr;
};

using FFFmpegFrameThreadSafeSharedPtr =
    TFFmpegFrameSharedPtr<ESPMode::ThreadSafe>;

template <ESPMode InMode>
AVFrame* TFFmpegFrameSharedPtr<InMode>::Get() const {
	return RawFrameSharedPtr.Get();
}

template <ESPMode InMode>
TFFmpegFrameSharedPtr<InMode>::TFFmpegFrameSharedPtr()
    : TFFmpegFrameSharedPtr(av_frame_alloc()) {}

template <ESPMode InMode>
TFFmpegFrameSharedPtr<InMode>::TFFmpegFrameSharedPtr(AVFrame* InRawFrame)
    : RawFrameSharedPtr(InRawFrame,
                        [](AVFrame* Frame) { av_frame_free(&Frame); }) {}

template <ESPMode InMode>
inline TFFmpegFrameSharedPtr<InMode>::operator bool() const {
	return RawFrameSharedPtr.operator bool();
}

template <ESPMode InMode>
inline AVFrame& TFFmpegFrameSharedPtr<InMode>::operator*() const {
	return *RawFrameSharedPtr;
}

template <ESPMode InMode>
inline AVFrame* TFFmpegFrameSharedPtr<InMode>::operator->() const {
	return RawFrameSharedPtr.Get();
}
