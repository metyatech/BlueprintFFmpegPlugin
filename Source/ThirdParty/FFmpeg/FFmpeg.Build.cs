
using System.IO;
using UnrealBuildTool;

public class FFmpeg : ModuleRules
{
    public FFmpeg(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        // get FFmpeg directory path
        var FFmpegDirectoryPath = Path.Combine(ModuleDirectory, "FFmpeg");

        // get FFmpeg libavcodec directory path
        var FFmpegIncludeDirectoryPath = Path.Combine(FFmpegDirectoryPath, "bin", "include");

        // get FFmpeg lib directory path
        var FFmpegLibDirectoryPath = Path.Combine(FFmpegDirectoryPath, "bin");

        // get FFmpeg bin directory path
        var FFmpegBinDirectoryPath = Path.Combine(FFmpegDirectoryPath, "bin");

        PublicSystemIncludePaths.Add(FFmpegIncludeDirectoryPath);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            var FFmpegDllFilePath = Path.Combine(FFmpegBinDirectoryPath, "*.dll");

            // Add the import library
            PublicAdditionalLibraries.Add(Path.Combine(FFmpegLibDirectoryPath, "avcodec.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(FFmpegLibDirectoryPath, "avformat.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(FFmpegLibDirectoryPath, "swscale.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(FFmpegLibDirectoryPath, "avutil.lib"));

            // Delay-load the DLL, so we can load it from the right place first
            PublicDelayLoadDLLs.Add(Path.Combine(FFmpegDllFilePath));

            // Ensure that the DLLs are staged along with the executable
            RuntimeDependencies.Add("$(BinaryOutputDir)", FFmpegDllFilePath);
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            var FFmpegDylibFilePath = Path.Combine(FFmpegBinDirectoryPath, "*.dylib");

            // Delay-load the DLL, so we can load it from the right place first
            PublicDelayLoadDLLs.Add(Path.Combine(FFmpegDylibFilePath));

            // Ensure that the DLL is staged along with the executable
            RuntimeDependencies.Add("$(BinaryOutputDir)", FFmpegDylibFilePath);
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            // Add the import library
            PublicAdditionalLibraries.Add(Path.Combine(FFmpegBinDirectoryPath, "*.so"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string LibFFmpegSoPath = Path.Combine(FFmpegBinDirectoryPath, "*.so");

            PublicAdditionalLibraries.Add(LibFFmpegSoPath);
            PublicDelayLoadDLLs.Add(LibFFmpegSoPath);
            RuntimeDependencies.Add(LibFFmpegSoPath);
        }
    }
}
