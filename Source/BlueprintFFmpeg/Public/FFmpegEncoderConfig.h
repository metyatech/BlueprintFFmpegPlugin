
#pragma once

#include "CoreMinimal.h"

#include "FFmpegEncoderConfig.generated.h"

/**
 * Structure for FFmpegEncoder settings
 */
USTRUCT(BlueprintType)
struct BLUEPRINTFFMPEG_API FFFmpegEncoderConfig {
	GENERATED_BODY()

	/**
	 * Width of output media
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Width = 1920;

	/**
	 * Height of output media
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Height = 1080;

	/**
	 * FrameRate of output media
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrameRate = 30.0f;

	/**
	 * BitRate of output media
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BitRate = 5000000;
};
