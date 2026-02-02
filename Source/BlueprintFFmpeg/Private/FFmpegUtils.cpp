
#include "FFmpegUtils.h"

#include "FFmpegEncoder.h"

void UFFmpegUtils::GenerateVideoFromImageFiles(
    const FString& OutputFilePath, const TArray<FString>& InputImagePaths,
    const FFFmpegEncoderConfig& FFmpegEncoderConfig) {
	const auto FFmpegEncoder = NewObject<UFFmpegEncoder>();
	check(nullptr != FFmpegEncoder);

	FFmpegEncoderOpenResult OpenResult;
	FString                 Open_ErrorMessage;
	FFmpegEncoder->Open(FFmpegEncoderConfig, OutputFilePath, OpenResult,
	                    Open_ErrorMessage);
	check(FFmpegEncoderOpenResult::Success == OpenResult);

	for (const auto& ImagePath : InputImagePaths) {
		FFmpegEncoderAddFrameResult AddFrame_Result;
		FString                     AddFrame_ErrorMessage;
		FFmpegEncoder->AddFrameFromImagePath(ImagePath, AddFrame_Result,
		                                     AddFrame_ErrorMessage);
		check(FFmpegEncoderAddFrameResult::Success == AddFrame_Result);
	}

	FFmpegEncoder->Close();
}
