
#pragma once

#include "CoreMinimal.h"
#include "FFmpegEncodeThread.h"
#include "FFmpegEncoderConfig.h"

#include "FFmpegEncoder.generated.h"

/**
 * A video encoder that uses FFmpeg and can be used from blueprint.
 * How to use:
 *   1. Create instance of this class
 *   2. call Open function
 *   3. call AddFrame function for each frames you want to encode
 *   4. call Close function
 * then the video is output to the OutputFilePath specified in Open function.
 */
UCLASS(Blueprintable, BlueprintType)
class BLUEPRINTFFMPEG_API UFFmpegEncoder: public UObject {
	GENERATED_BODY()

	// type aliases
public:
	using TTask_Frame = FFFmpegEncodeThread::TTask_Frame;
	using TTask_Image = FFFmpegEncodeThread::TTask_Image;

	// blueprint functions
public:
	/**
	 * Initialize and put into encoding standby status.
	 * @param FFmpegEncoderConfig   setting.
	 * @param OutputFilePath   Output destination file path.
	 *                         The output format is determined by the
	 *                         extension of this path.
	 * @param[out] Result   result.
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "Result"))
	void Open(const FFFmpegEncoderConfig& FFmpegEncoderConfig,
	          const FString& OutputFilePath, FFmpegEncoderOpenResult& Result,
	          FString& ErrorMessage);

	/**
	 * Terminate encoding. The encoding result is output to the file specified by
	 * OutputFilePath of the Open function.
	 */
	UFUNCTION(BlueprintCallable)
	void Close();

	/**
	 * Add a frame. The argument is converted to a YUV420P format image, added as
	 * a frame, and appended to the file immediately after the frame data is
	 * finalized.
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "Result"))
	void AddFrameFromRenderTarget(
	    const UTextureRenderTarget2D* TextureRenderTarget,
	    FFmpegEncoderAddFrameResult& Result, FString& ErrorMessage);

	/**
	 * Add a frame. The argument is converted to a YUV420P format image, added as
	 * a frame, and appended to the file immediately after the frame data is
	 * finalized.
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "Result"))
	void AddFrameFromImagePath(const FString&               ImagePath,
	                           FFmpegEncoderAddFrameResult& Result,
	                           FString&                     ErrorMessage);

	// C++ functions
public:
	/**
	 * Add a frame. The argument is converted to a YUV420P format image, added as
	 * a frame, and appended to the file immediately after the frame data is
	 * finalized.
	 */
	template <typename FTextureRHIRef_T>
	  requires std::is_same_v<FTextureRHIRef,
	                          std::remove_cvref_t<FTextureRHIRef_T>>
	void AddFrame(FTextureRHIRef_T&&           TextureRHI,
	              FFmpegEncoderAddFrameResult& Result, FString& ErrorMessage);

	/**
	 * Add a frame. The argument is converted to a YUV420P format image, added as
	 * a frame, and appended to the file immediately after the frame data is
	 * finalized.
	 */
	void AddFrame(const TTask_Image&           ImageTask,
	              FFmpegEncoderAddFrameResult& Result, FString& ErrorMessage);

	/**
	 * Add a frame. The argument is converted to a YUV420P format image, added as
	 * a frame, and appended to the file immediately after the frame data is
	 * finalized.
	 */
	template <typename TTaskFFFmpegFrameThreadSafeSharedPtr_T>
	  requires std::is_same_v<
	      UFFmpegEncoder::TTask_Frame,
	      std::remove_cvref_t<TTaskFFFmpegFrameThreadSafeSharedPtr_T>>
	void AddFrame(TTaskFFFmpegFrameThreadSafeSharedPtr_T&& Frame,
	              FFmpegEncoderAddFrameResult& Result, FString& ErrorMessage);

	// private fields
private:
	FFFmpegEncodeThread FFmpegEncodeThread;
};

#pragma region definition of template functions
template <typename FTextureRHIRef_T>
  requires std::is_same_v<FTextureRHIRef, std::remove_cvref_t<FTextureRHIRef_T>>
void UFFmpegEncoder::AddFrame(FTextureRHIRef_T&&           TextureRHI,
                              FFmpegEncoderAddFrameResult& Result,
                              FString&                     ErrorMessage) {
	FFmpegEncodeThread.AddFrame(Forward<FTextureRHIRef_T>(TextureRHI), Result,
	                            ErrorMessage);
}

template <typename TTaskFFFmpegFrameThreadSafeSharedPtr_T>
  requires std::is_same_v<
      UFFmpegEncoder::TTask_Frame,
      std::remove_cvref_t<TTaskFFFmpegFrameThreadSafeSharedPtr_T>>
void UFFmpegEncoder::AddFrame(TTaskFFFmpegFrameThreadSafeSharedPtr_T&& Frame,
                              FFmpegEncoderAddFrameResult&             Result,
                              FString& ErrorMessage) {
	return FFmpegEncodeThread.AddFrame(
	    Forward<TTaskFFFmpegFrameThreadSafeSharedPtr_T>(Frame), Result,
	    ErrorMessage);
}
#pragma endregion
