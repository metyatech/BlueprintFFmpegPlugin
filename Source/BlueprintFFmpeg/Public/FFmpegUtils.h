
#pragma once

#include "CoreMinimal.h"
#include "FFmpegFrameSharedPtr.h"
#include "ImageCore.h"
#include "ImageUtils.h"

#include <optional>

extern "C" {
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

#include "FFmpegUtils.generated.h"

/**
 *
 */
UCLASS()
class BLUEPRINTFFMPEG_API UFFmpegUtils: public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void GenerateVideoFromImageFiles(
	    const FString& OutputFilePath, const TArray<FString>& InputImagePaths,
	    const FFFmpegEncoderConfig& FFmpegEncoderConfig);

public:
	static constexpr AVPixelFormat
	    FFmpegFrameFormatOf(ERawImageFormat::Type UEImageFormat) noexcept;

	template <ESPMode InMode = ESPMode::ThreadSafe>
	static TFFmpegFrameSharedPtr<InMode> CreateFrame(
	    const FString& ImagePath, int FrameIndex,
	    std::optional<int> FrameWidth = {}, std::optional<int> FrameHeight = {},
	    AVPixelFormat PixelFormat = AVPixelFormat::AV_PIX_FMT_YUV420P);

	template <ESPMode InMode = ESPMode::ThreadSafe>
	static TFFmpegFrameSharedPtr<InMode> CreateFrame(
	    const FImage& Image, int FrameIndex, std::optional<int> FrameWidth = {},
	    std::optional<int> FrameHeight = {},
	    AVPixelFormat      PixelFormat = AVPixelFormat::AV_PIX_FMT_YUV420P);
};

#pragma region          definition of inline functions
constexpr AVPixelFormat UFFmpegUtils::FFmpegFrameFormatOf(
    ERawImageFormat::Type UEImageFormat) noexcept {
	using UEFormat = ERawImageFormat::Type;

	switch (UEImageFormat) {
	case UEFormat::G8:
		return AV_PIX_FMT_GRAY8; // 8-bit grayscale
	case UEFormat::BGRA8:
		return AV_PIX_FMT_BGRA; // 8-bit BGRA
	case UEFormat::BGRE8:
		return AV_PIX_FMT_BGR0; // 8-bit BGRX (similar to BGRE8, no alpha channel)
	case UEFormat::RGBA16:
		return AV_PIX_FMT_RGBA64; // 16-bit RGBA
	case UEFormat::RGBA16F:
		return AV_PIX_FMT_RGBA64; // No direct 16F in FFmpeg, using 64-bit RGBA as
		                          // closest
	case UEFormat::RGBA32F:
		return AV_PIX_FMT_RGBA; // No direct 32F, using 8-bit RGBA as an
		                        // approximation
	case UEFormat::G16:
		return AV_PIX_FMT_GRAY16; // 16-bit grayscale
	case UEFormat::R16F:
		return AV_PIX_FMT_GRAY16; // No direct support, using 16-bit grayscale as
		                          // closest
	case UEFormat::R32F:
		return AV_PIX_FMT_GRAYF32; // 32-bit float grayscale, FFmpeg uses
		                           // AV_PIX_FMT_GRAYF32 for float grayscale
	case UEFormat::MAX:
		return AV_PIX_FMT_NONE; // No valid conversion, use a placeholder
	case UEFormat::Invalid:
		return AV_PIX_FMT_NONE; // Invalid format, return a placeholder
	default:
		return AV_PIX_FMT_NONE; // Fallback for unspecified formats
	}
}

template <ESPMode InMode>
TFFmpegFrameSharedPtr<InMode>
    UFFmpegUtils::CreateFrame(const FString& ImagePath, const int FrameIndex,
                              std::optional<int> FrameWidth,
                              std::optional<int> FrameHeight,
                              AVPixelFormat      PixelFormat) {
	FImage Image;
	FImageUtils::LoadImage(*ImagePath, Image);
	return CreateFrame(Image, FrameIndex, FrameWidth, FrameHeight, PixelFormat);
}

template <ESPMode InMode>
TFFmpegFrameSharedPtr<InMode> UFFmpegUtils::CreateFrame(
    const FImage& Image, const int FrameIndex, std::optional<int> FrameWidth,
    std::optional<int> FrameHeight, AVPixelFormat PixelFormat) {
	TFFmpegFrameSharedPtr<InMode> FFmpegFrame;

	const auto& SrcFormat = UFFmpegUtils::FFmpegFrameFormatOf(Image.Format);
	const auto& SrcWidth  = Image.GetWidth();
	const auto& SrcHeight = Image.GetHeight();

	const auto& RawFrame = FFmpegFrame.Get();

	RawFrame->pts    = FrameIndex;
	RawFrame->format = PixelFormat;
	RawFrame->width  = FrameWidth.value_or(SrcWidth);
	RawFrame->height = FrameHeight.value_or(SrcHeight);

	// initialize frame buffer
	if (av_frame_get_buffer(RawFrame, 0) < 0) {
		UE_LOG(LogTemp, Error, TEXT("Failed to allocate AVFrame buffer"));
		return FFmpegFrame;
	}

	SwsContext* SwsConvertFormatContext =
	    sws_getContext(SrcWidth, SrcHeight, SrcFormat, SrcWidth, SrcHeight,
	                   PixelFormat, SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (nullptr == SwsConvertFormatContext) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create SwsContext."));
		return FFmpegFrame;
	}

	const auto&    RawImageData  = Image.RawData;
	const auto&    BytesPerPixel = Image.GetBytesPerPixel();
	const auto&    DestImageData = RawFrame->data[0];
	const uint8_t* SrcData[8]    = {RawImageData.GetData(),
	                                nullptr,
	                                nullptr,
	                                nullptr,
	                                nullptr,
	                                nullptr,
	                                nullptr,
	                                nullptr};
	const int SrcLineSize[8] = {SrcWidth * BytesPerPixel, 0, 0, 0, 0, 0, 0, 0};
	sws_scale(SwsConvertFormatContext, SrcData, SrcLineSize, 0, SrcHeight,
	          RawFrame->data, RawFrame->linesize);

	sws_freeContext(SwsConvertFormatContext);

	return FFmpegFrame;
}
#pragma endregion
