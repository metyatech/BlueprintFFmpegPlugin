
#pragma once

#include "CoreMinimal.h"

/**
 * @param TextureRHI   Source TextureRHI from which the image is created.
 * @return   task to create image
 */
template <typename FTextureRHIRef_T>
  requires std::is_same_v<FTextureRHIRef, std::remove_cvref_t<FTextureRHIRef_T>>
UE::Tasks::TTask<FImage>
    CreateImageFromTextureRHIAsync(FTextureRHIRef_T&& TextureRHI);

#pragma region definition of template functions
template <typename FTextureRHIRef_T>
  requires std::is_same_v<FTextureRHIRef, std::remove_cvref_t<FTextureRHIRef_T>>
UE::Tasks::TTask<FImage>
    CreateImageFromTextureRHIAsync(FTextureRHIRef_T&& TextureRHI) {
	namespace Tasks = UE::Tasks;

	// wait to ReadSurfaceData in GameThread
#if false
	// get description of source texture RHI
	const auto& Desc = TextureRHI->GetDesc();

	// get Width
	const auto& Width = Desc.Extent.X;

	// get Height
	const auto& Height = Desc.Extent.Y;

	// get PixelFormat
	const auto& PixelFormat = Desc.Format;

	// pre allocate array of Color
	TArray<FColor> ColorArray_Pre;

	// ColorArray.Num() becomes Width * Height
	ColorArray_Pre.Reserve(Width * Height);

	// make promise to store Color
	TPromise<TArray<FColor>> Color_Promise;
	auto                     Color_Future = Color_Promise.GetFuture();

	// On Render Thread
	ENQUEUE_RENDER_COMMAND(ReadTexture)
	([&](FRHICommandListImmediate& RHICmdList) mutable {
		// create settings
		FReadSurfaceDataFlags ReadSurfaceDataFlags;

		// assume color space of TextureRHI is gamma space
		ReadSurfaceDataFlags.SetLinearToGamma(false);

		// read texture color data to ColorArray_Pre
		RHICmdList.ReadSurfaceData(MoveTemp(TextureRHI),
		                           FIntRect(0, 0, Width, Height), ColorArray_Pre,
		                           ReadSurfaceDataFlags);

		// Store Color and delivers on promise
		Color_Promise.EmplaceValue(MoveTemp(ColorArray_Pre));
	});

	// wait for completion
	FlushRenderingCommands();

	// launch a task that convert TArray<FColor> to FImage
	return Tasks::Launch(
	    UE_SOURCE_LOCATION,
	    [Color_Future = MoveTemp(Color_Future), Width, Height]() mutable {
		    // get array of Color
		    auto&& ColorArray = Color_Future.Consume();

		    // ColorArray should be packed with all the pixel information without
		    // wasting a single byte.
		    check(Width * Height == ColorArray.Num());

		    // initialize OutImage
		    FImage OutImage(Width, Height, ERawImageFormat::BGRA8);

		    // copy ColorArray to OutImage
		    FMemory::Memcpy(OutImage.RawData.GetData(), ColorArray.GetData(),
		                    ColorArray.Num() * sizeof(FColor));

		    return OutImage;
	    },
	    LowLevelTasks::ETaskPriority::BackgroundNormal);
	// not wait to ReadSurfaceData in GameThread (wait in WorkerThread)
#elif true
	return Tasks::Launch(
	    UE_SOURCE_LOCATION,
	    [TextureRHI = Forward<FTextureRHIRef_T>(TextureRHI)]() mutable {
		    // get description of source texture RHI
		    const auto& Desc = TextureRHI->GetDesc();

		    // get Width
		    const auto& Width = Desc.Extent.X;

		    // get Height
		    const auto& Height = Desc.Extent.Y;

		    // get PixelFormat
		    const auto& PixelFormat = Desc.Format;

		    // pre allocate array of Color
		    TArray<FColor> ColorArray_Pre;

		    // ColorArray.Num() becomes Width * Height
		    ColorArray_Pre.Reserve(Width * Height);

		    // make promise to store Color
		    TPromise<TArray<FColor>> Color_Promise;
		    auto                     Color_Future = Color_Promise.GetFuture();

		    // On Render Thread
		    ENQUEUE_RENDER_COMMAND(ReadTexture)
		    ([&](FRHICommandListImmediate& RHICmdList) mutable {
			    // create settings
			    FReadSurfaceDataFlags ReadSurfaceDataFlags;

			    // assume color space of TextureRHI is gamma space
			    ReadSurfaceDataFlags.SetLinearToGamma(false);

			    // read texture color data to ColorArray_Pre
			    RHICmdList.ReadSurfaceData(MoveTemp(TextureRHI),
			                               FIntRect(0, 0, Width, Height),
			                               ColorArray_Pre, ReadSurfaceDataFlags);

			    // Store Color and delivers on promise
			    Color_Promise.EmplaceValue(MoveTemp(ColorArray_Pre));
		    });

		    // get array of Color
		    auto&& ColorArray = Color_Future.Consume();

		    // ColorArray should be packed with all the pixel information without
		    // wasting a single byte.
		    check(Width * Height == ColorArray.Num());

		    // initialize OutImage
		    FImage OutImage(Width, Height, ERawImageFormat::BGRA8);

		    // copy ColorArray to OutImage
		    FMemory::Memcpy(OutImage.RawData.GetData(), ColorArray.GetData(),
		                    ColorArray.Num() * sizeof(FColor));

		    return OutImage;
	    },
	    LowLevelTasks::ETaskPriority::BackgroundNormal,
	    Tasks::EExtendedTaskPriority::None,
	    Tasks::ETaskFlags::DoNotRunInsideBusyWait);
#endif
}
#pragma endregion
