
#pragma once

#include "CoreMinimal.h"
#include "CreateImageFromTextureRHI.h"
#include "Engine/TextureRenderTarget2D.h"
#include "FFmpegEncoderConfig.h"
#include "FFmpegFrameSharedPtr.h"
#include "FFmpegUtils.h"
#include "LogFFmpegEncoder.h"

#include <atomic>
#include <condition_variable>

/**
 * Result type of UFFmpegEncoder::Open
 */
UENUM(BlueprintType)
enum class FFmpegEncoderOpenResult : uint8 { Success, Failure };

/**
 * Result type of UFFmpegEncoder::Close
 */
UENUM(BlueprintType)
enum class FFmpegEncoderCloseResult : uint8 { Success, Failure };

/**
 * Result type of UFFmpegEncoder::AddFrame
 */
UENUM(BlueprintType)
enum class FFmpegEncoderAddFrameResult : uint8 { Success, Failure };

enum class FFmpegEncoderThreadResult {
	Success = 0,
	CodecH264IsNotFound,
	FailedToAllocateCodecContext,
	FailedToInitializeCodecContext,
	FailedToInitializeIOContext,
	FailedToAllocateFormatContext,
	FailedToAddANewStream,
	FailedToSetCodecParameters,
	FailedToWriteHeader,

	FailedToSendFrame,
	FailedToAllocatePacket,
	FailedToWritePacket,

	FailedToFlushSendFrame,
	FailedToWriteTrailer
};

/**
 * A video encoder that uses FFmpeg and can be used from blueprint.
 * How to use:
 *   1. Create instance of this class
 *   2. call Open function
 *   3. call AddFrame function for each frames you want to encode
 *   4. call Close function
 * then the video is output to the OutputFilePath specified in Open function.
 */
class BLUEPRINTFFMPEG_API FFFmpegEncodeThread: public FRunnable {
	// type aliases
public:
	using TTask_Frame = UE::Tasks::TTask<FFFmpegFrameThreadSafeSharedPtr>;
	using TTask_Image = UE::Tasks::TTask<FImage>;

	// public functions
public:
	/**
	 * Initialize and put into encoding standby status.
	 * @param FFmpegEncoderConfig   setting.
	 * @param OutputFilePath   Output destination file path.
	 *                         The output format is determined by the
	 *                         extension of this path.
	 * @param[out] Result   result.
	 */
	void Open(const FFFmpegEncoderConfig& FFmpegEncoderConfig,
	          const FString& OutputFilePath, FFmpegEncoderOpenResult& Result,
	          FString& ErrorMessage);

	/**
	 * Terminate encoding. The encoding result is output to the file specified by
	 * OutputFilePath of the Open function.
	 */
	void Close();

	/**
	 * Add a frame. The argument is converted to a YUV420P format image, added as
	 * a frame, and appended to the file immediately after the frame data is
	 * finalized.
	 */
	void AddFrame(const UTextureRenderTarget2D* TextureRenderTarget,
	              FFmpegEncoderAddFrameResult& Result, FString& ErrorMessage);

	/**
	 * Add a frame. The argument is converted to a YUV420P format image, added as
	 * a frame, and appended to the file immediately after the frame data is
	 * finalized.
	 */
	void AddFrame(const FString& ImagePath, FFmpegEncoderAddFrameResult& Result,
	              FString& ErrorMessage);

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
	      FFFmpegEncodeThread::TTask_Frame,
	      std::remove_cvref_t<TTaskFFFmpegFrameThreadSafeSharedPtr_T>>
	void AddFrame(TTaskFFFmpegFrameThreadSafeSharedPtr_T&& Frame,
	              FFmpegEncoderAddFrameResult& Result, FString& ErrorMessage);

public:
	~FFFmpegEncodeThread();

	// FRunnable interfaces
public:
	virtual uint32 Run() override;
	virtual void   Stop() override;

	// private constants
private:
	static constexpr const TCHAR checkfMesNotOpened_AddFrame[] =
	    TEXT("Before calling this function, Open function must be called.");
	static constexpr const TCHAR checkfMesClosed_AddFrame[] = TEXT(
	    "Once Close function is called, this function can no longer be called.");

	// private fields: no data race
private:
	bool                 bOpened = false;
	bool                 bClosed = false;
	FFFmpegEncoderConfig Config;
	FString              VideoPath;
	int64_t              FrameIndex = 0;
	FRunnableThread*     Thread     = nullptr;

	// private fields: beware of data race
private:
	// single-producer, single-consumer
	TQueue<TTask_Frame, EQueueMode::Spsc> FrameTasks;
	std::atomic_bool                      bRunning = true;
	std::mutex                            FrameTasks_mutex;
	std::condition_variable               EncodeThread_cv;
};

#pragma region definition of template functions
template <typename FTextureRHIRef_T>
  requires std::is_same_v<FTextureRHIRef, std::remove_cvref_t<FTextureRHIRef_T>>
void FFFmpegEncodeThread::AddFrame(FTextureRHIRef_T&&           TextureRHI,
                                   FFmpegEncoderAddFrameResult& Result,
                                   FString&                     ErrorMessage) {
	// Open function must be called
	checkf(bOpened, checkfMesNotOpened_AddFrame);

	// and Close function must not be called.
	checkf(!bClosed, checkfMesClosed_AddFrame);

	// launch task to create image
	auto ImageTask =
	    CreateImageFromTextureRHIAsync(Forward<FTextureRHIRef_T>(TextureRHI));

	return AddFrame(MoveTemp(ImageTask), Result, ErrorMessage);
}

template <typename TTaskFFFmpegFrameThreadSafeSharedPtr_T>
  requires std::is_same_v<
      FFFmpegEncodeThread::TTask_Frame,
      std::remove_cvref_t<TTaskFFFmpegFrameThreadSafeSharedPtr_T>>
void FFFmpegEncodeThread::AddFrame(
    TTaskFFFmpegFrameThreadSafeSharedPtr_T&& Frame,
    FFmpegEncoderAddFrameResult& Result, FString& ErrorMessage) {
	// Open function must be called
	checkf(bOpened, checkfMesNotOpened_AddFrame);

	// and Close function must not be called.
	checkf(!bClosed, checkfMesClosed_AddFrame);

	// helper function to finish with success
	const auto& Success = [&]() {
		Result = FFmpegEncoderAddFrameResult::Success;
	};

	// helper function to finish with failure
	const auto& Failure = [&](const FString& Message) {
		ErrorMessage = Message;
		UE_LOG(LogFFmpegEncoder, Error, TEXT("%s"), *ErrorMessage);
		Result = FFmpegEncoderAddFrameResult::Failure;
	};

	// enqueue frame
	const auto& SuccessToEnqueue = FrameTasks.Enqueue(
	    Forward<TTaskFFFmpegFrameThreadSafeSharedPtr_T>(Frame));

	// if failed to enqueue
	if (!SuccessToEnqueue) {
		return Failure("Failed to enqueue the frame.");
	}

	// increment FrameIndex
	++FrameIndex;

	// notify that a task has been enqueued to FrameTasks
	{
		std::lock_guard lk(FrameTasks_mutex);
		EncodeThread_cv.notify_one();
	}

	return Success();
}
#pragma endregion
