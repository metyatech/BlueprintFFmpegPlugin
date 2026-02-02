
#include "FFmpegEncoder.h"

void UFFmpegEncoder::Open(const FFFmpegEncoderConfig& FFmpegEncoderConfig,
                          const FString&              OutputFilePath,
                          FFmpegEncoderOpenResult&    Result,
                          FString&                    ErrorMessage) {
	// open thread
	return FFmpegEncodeThread.Open(FFmpegEncoderConfig, OutputFilePath, Result,
	                               ErrorMessage);
}

void UFFmpegEncoder::Close() {
	// close thread
	return FFmpegEncodeThread.Close();
}

void UFFmpegEncoder::AddFrameFromRenderTarget(
    const UTextureRenderTarget2D* TextureRenderTarget,
    FFmpegEncoderAddFrameResult& Result, FString& ErrorMessage) {
	return FFmpegEncodeThread.AddFrame(TextureRenderTarget, Result, ErrorMessage);
}

void UFFmpegEncoder::AddFrameFromImagePath(const FString& ImagePath,
                                           FFmpegEncoderAddFrameResult& Result,
                                           FString& ErrorMessage) {
	return FFmpegEncodeThread.AddFrame(ImagePath, Result, ErrorMessage);
}

void UFFmpegEncoder::AddFrame(const TTask_Image&           ImageTask,
                              FFmpegEncoderAddFrameResult& Result,
                              FString&                     ErrorMessage) {
	return FFmpegEncodeThread.AddFrame(ImageTask, Result, ErrorMessage);
}
