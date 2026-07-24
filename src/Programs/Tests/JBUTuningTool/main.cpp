// JBU Tuning Tool (ticket D5) - loads a PFM depth map + PNG guide image, runs
// MikanARKitVideo's Joint Bilateral Upsampling kernel (src/Plugins/MikanARKitVideo/
// Private/Cuda/JBUKernel.h), and writes the result(s) as grayscale PNGs for visual
// inspection. Not a pass/fail test - a dev tool for tuning JBUParams (radius/
// sigmaSpatial/sigmaColor/confWeight*) against real depth data, the same way the
// D5 tuning pass itself was done (see JBUKernel.h's JBUParams comment for that
// pass's findings and chosen defaults).
//
// Usage:
//   JBUTuningTool --depth <path.pfm> --guide <path.png> --out <dir>
//                 [--radius N] [--sigma-spatial F] [--sigma-color F]
//                 [--conf-weight-low F] [--conf-weight-medium F]
//                 [--sweep]
//
// A single run (no --sweep) uses JBUParams' current defaults unless overridden by
// the flags above, and writes one <out>/result.png plus <out>/baseline_nearest.png
// (a plain nearest-neighbor upsample, useful as a "no smoothing at all" reference
// point). --sweep instead ignores the individual --radius/--sigma-* flags and
// runs a small preset sweep of radius/sigma combinations, writing one PNG per
// combination, named after its parameters.
//
// depth_low.pfm/rgb_full.png from D:\Github\git-BrendanWalker\CudaDepthUpsample
// (the CUDA JBU prototype this kernel was ported from) is real 256x192 depth +
// 1920x1080 RGB test data at exactly ARKit's real upsample ratio - a good default
// choice of --depth/--guide if you don't have real ARKit-captured data on hand.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <cuda.h>

#if defined(_WIN32)
#include <direct.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "JBUKernel.h"
#include "Logger.h"

namespace
{
struct Options
{
	std::string depthPath;
	std::string guidePath;
	std::string outDir= "jbu_tuning_output";
	JBUParams params; // defaults from JBUKernel.h unless overridden below
	bool sweep= false;
};

void printUsage()
{
	printf("JBUTuningTool - visualize JBU depth upsampling at different parameters\n\n"
		   "Usage:\n"
		   "  JBUTuningTool --depth <path.pfm> --guide <path.png> --out <dir>\n"
		   "                [--radius N] [--sigma-spatial F] [--sigma-color F]\n"
		   "                [--conf-weight-low F] [--conf-weight-medium F]\n"
		   "                [--sweep]\n\n"
		   "  --depth   Path to a grayscale PFM depth map (e.g. CudaDepthUpsample's\n"
		   "            data\\depth_low.pfm)\n"
		   "  --guide   Path to an RGB PNG guide image, any resolution (this is the\n"
		   "            upsample target size)\n"
		   "  --out     Output directory for result PNGs (created if missing)\n"
		   "  --sweep   Ignore --radius/--sigma-*, run a small preset parameter sweep\n"
		   "            instead of a single run, writing one PNG per combination\n");
}

bool parseArgs(int argc, char* argv[], Options& outOptions)
{
	for (int i= 1; i < argc; ++i)
	{
		const std::string arg= argv[i];
		auto nextArg= [&]() -> const char* { return (i + 1 < argc) ? argv[++i] : nullptr; };

		if (arg == "--depth")
		{
			const char* v= nextArg();
			if (v == nullptr)
				return false;
			outOptions.depthPath= v;
		}
		else if (arg == "--guide")
		{
			const char* v= nextArg();
			if (v == nullptr)
				return false;
			outOptions.guidePath= v;
		}
		else if (arg == "--out")
		{
			const char* v= nextArg();
			if (v == nullptr)
				return false;
			outOptions.outDir= v;
		}
		else if (arg == "--radius")
		{
			const char* v= nextArg();
			if (v == nullptr)
				return false;
			outOptions.params.radius= std::atoi(v);
		}
		else if (arg == "--sigma-spatial")
		{
			const char* v= nextArg();
			if (v == nullptr)
				return false;
			outOptions.params.sigmaSpatial= static_cast<float>(std::atof(v));
		}
		else if (arg == "--sigma-color")
		{
			const char* v= nextArg();
			if (v == nullptr)
				return false;
			outOptions.params.sigmaColor= static_cast<float>(std::atof(v));
		}
		else if (arg == "--conf-weight-low")
		{
			const char* v= nextArg();
			if (v == nullptr)
				return false;
			outOptions.params.confWeightLow= static_cast<float>(std::atof(v));
		}
		else if (arg == "--conf-weight-medium")
		{
			const char* v= nextArg();
			if (v == nullptr)
				return false;
			outOptions.params.confWeightMedium= static_cast<float>(std::atof(v));
		}
		else if (arg == "--sweep")
		{
			outOptions.sweep= true;
		}
		else if (arg == "--help" || arg == "-h")
		{
			return false;
		}
		else
		{
			fprintf(stderr, "Unrecognized argument: %s\n", arg.c_str());
			return false;
		}
	}

	return !outOptions.depthPath.empty() && !outOptions.guidePath.empty();
}

// Grayscale (Pf) PFM loader. Rows are stored bottom-to-top in the file; flipped
// here to top-to-bottom to match everything else (including stb_image's PNG
// loading convention).
bool loadPFM(const std::string& path, int& outW, int& outH, std::vector<float>& outData)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
		return false;

	std::string header;
	file >> header;
	if (header != "Pf")
	{
		fprintf(stderr, "PFM file is not grayscale (\"Pf\") - color PFM isn't supported\n");
		return false;
	}

	file >> outW >> outH;
	float scale= 0.0f;
	file >> scale;
	file.get(); // consume the single whitespace char terminating the scale line

	outData.resize(static_cast<size_t>(outW) * outH);
	file.read(reinterpret_cast<char*>(outData.data()), outData.size() * sizeof(float));
	if (!file)
		return false;

	std::vector<float> flipped(outData.size());
	for (int y= 0; y < outH; ++y)
	{
		std::copy(outData.begin() + static_cast<size_t>(outH - 1 - y) * outW,
				  outData.begin() + static_cast<size_t>(outH - y) * outW,
				  flipped.begin() + static_cast<size_t>(y) * outW);
	}
	outData= std::move(flipped);
	return true;
}

// PFM depth is arbitrary-unit float (often normalized disparity, not physical
// distance) with NaN marking invalid/occluded pixels - remaps into a plausible
// uint16 "mm" range for the kernel (0 = invalid, matching ARKit's real wire
// convention - see ARKitDepthReceiver.h). The exact output unit doesn't matter for
// visual/spatial tuning purposes, only relative variation and invalid-pixel
// locations do.
std::vector<uint16_t> normalizeToU16(const std::vector<float>& depthRaw)
{
	float minV= 1e9f, maxV= -1e9f;
	for (float v : depthRaw)
	{
		if (std::isfinite(v))
		{
			minV= std::min(minV, v);
			maxV= std::max(maxV, v);
		}
	}
	const float range= (maxV > minV) ? (maxV - minV) : 1.0f;

	std::vector<uint16_t> out(depthRaw.size());
	for (size_t i= 0; i < depthRaw.size(); ++i)
	{
		const float v= depthRaw[i];
		out[i]= std::isfinite(v) ? static_cast<uint16_t>(500.0f + 3500.0f * (v - minV) / range) : 0;
	}
	return out;
}

void writeDepthPNG(const std::string& path, const std::vector<float>& depth, int w, int h)
{
	float minV= 1e9f, maxV= -1e9f;
	for (float v : depth)
	{
		if (v > 0.0f && std::isfinite(v))
		{
			minV= std::min(minV, v);
			maxV= std::max(maxV, v);
		}
	}
	const float range= (maxV > minV) ? (maxV - minV) : 1.0f;

	std::vector<uint8_t> pixels(static_cast<size_t>(w) * h);
	for (size_t i= 0; i < depth.size(); ++i)
	{
		const float v= depth[i];
		pixels[i]= (v > 0.0f && std::isfinite(v)) ? static_cast<uint8_t>(255.0f * (v - minV) / range) : 0;
	}
	stbi_write_png(path.c_str(), w, h, 1, pixels.data(), w);
}

void writeNearestNeighborBaseline(const std::string& outDir, const std::vector<uint16_t>& depthU16, int lowW, int lowH,
								  int guideW, int guideH)
{
	std::vector<float> nearest(static_cast<size_t>(guideW) * guideH);
	for (int y= 0; y < guideH; ++y)
	{
		const int ly= std::min(lowH - 1, y * lowH / guideH);
		for (int x= 0; x < guideW; ++x)
		{
			const int lx= std::min(lowW - 1, x * lowW / guideW);
			nearest[static_cast<size_t>(y) * guideW + x]=
				static_cast<float>(depthU16[static_cast<size_t>(ly) * lowW + lx]);
		}
	}
	writeDepthPNG(outDir + "/baseline_nearest.png", nearest, guideW, guideH);
}

bool ensureCudaContext(CUcontext& outContext)
{
	if (cuInit(0) != CUDA_SUCCESS)
	{
		fprintf(stderr, "cuInit failed\n");
		return false;
	}

	int deviceCount= 0;
	if (cuDeviceGetCount(&deviceCount) != CUDA_SUCCESS || deviceCount <= 0)
	{
		fprintf(stderr, "No CUDA-capable device found\n");
		return false;
	}

	CUdevice device;
	cuDeviceGet(&device, 0);
	if (cuCtxCreate(&outContext, nullptr, 0, device) != CUDA_SUCCESS)
	{
		fprintf(stderr, "cuCtxCreate failed\n");
		return false;
	}
	return true;
}

void makeDir(const std::string& path)
{
#if defined(_WIN32)
	_mkdir(path.c_str());
#endif
}
} // namespace

int main(int argc, char* argv[])
{
	Options options;
	if (!parseArgs(argc, argv, options))
	{
		printUsage();
		return 1;
	}

	LoggerSettings logSettings{};
	logSettings.min_log_level= LogSeverityLevel::info;
	logSettings.enable_console= false; // already a console app
	log_init(logSettings);

	int lowW= 0, lowH= 0;
	std::vector<float> depthRaw;
	if (!loadPFM(options.depthPath, lowW, lowH, depthRaw))
	{
		fprintf(stderr, "Failed to load depth PFM: %s\n", options.depthPath.c_str());
		return 1;
	}
	printf("Loaded depth: %dx%d\n", lowW, lowH);

	int guideW= 0, guideH= 0, guideChannels= 0;
	uint8_t* guidePixels= stbi_load(options.guidePath.c_str(), &guideW, &guideH, &guideChannels, 3);
	if (guidePixels == nullptr)
	{
		fprintf(stderr, "Failed to load guide PNG: %s\n", options.guidePath.c_str());
		return 1;
	}
	printf("Loaded guide: %dx%d (%.2fx / %.2fx upsample ratio)\n", guideW, guideH, static_cast<double>(guideW) / lowW,
		   static_cast<double>(guideH) / lowH);

	const std::vector<uint16_t> depthU16= normalizeToU16(depthRaw);

	makeDir(options.outDir);
	writeNearestNeighborBaseline(options.outDir, depthU16, lowW, lowH, guideW, guideH);

	CUcontext context= nullptr;
	if (!ensureCudaContext(context))
	{
		stbi_image_free(guidePixels);
		return 1;
	}

	JBUKernel kernel;
	if (!kernel.init(JBU_KERNEL_PTX_PATH))
	{
		fprintf(stderr, "JBUKernel::init failed\n");
		stbi_image_free(guidePixels);
		cuCtxDestroy(context);
		return 1;
	}

	CUdeviceptr d_depth= 0, d_guide= 0, d_out= 0;
	cuMemAlloc(&d_depth, depthU16.size() * sizeof(uint16_t));
	cuMemAlloc(&d_guide, static_cast<size_t>(guideW) * guideH * 3);
	cuMemAlloc(&d_out, static_cast<size_t>(guideW) * guideH * sizeof(float));
	cuMemcpyHtoD(d_depth, depthU16.data(), depthU16.size() * sizeof(uint16_t));
	cuMemcpyHtoD(d_guide, guidePixels, static_cast<size_t>(guideW) * guideH * 3);

	auto runOne= [&](const JBUParams& params, const std::string& outName)
	{
		kernel.upsample(d_depth, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), 0, 0, d_guide, guideW, guideH,
						guideW * 3, d_out, guideW, guideH, guideW * static_cast<int>(sizeof(float)), params);
		kernel.synchronize();

		std::vector<float> out(static_cast<size_t>(guideW) * guideH);
		cuMemcpyDtoH(out.data(), d_out, out.size() * sizeof(float));

		printf("  %-45s radius=%-3d sigmaSpatial=%-6.1f sigmaColor=%-6.1f  %.2f ms\n", outName.c_str(), params.radius,
			   params.sigmaSpatial, params.sigmaColor, kernel.getLastKernelMilliseconds());

		writeDepthPNG(options.outDir + "/" + outName + ".png", out, guideW, guideH);
	};

	if (options.sweep)
	{
		struct SweepCase
		{
			const char* name;
			int radius;
			float sigmaSpatial;
			float sigmaColor;
		};
		const SweepCase cases[]= {
			{"radius6_spatial7_color25", 6, 7.0f, 25.0f},     {"radius16_spatial7_color25", 16, 7.0f, 25.0f},
			{"radius24_spatial12_color15", 24, 12.0f, 15.0f}, {"radius32_spatial16_color15", 32, 16.0f, 15.0f},
			{"radius32_spatial16_color30", 32, 16.0f, 30.0f}, {"radius48_spatial20_color15", 48, 20.0f, 15.0f},
		};

		printf("Running sweep:\n");
		for (const SweepCase& c : cases)
		{
			JBUParams params;
			params.radius= c.radius;
			params.sigmaSpatial= c.sigmaSpatial;
			params.sigmaColor= c.sigmaColor;
			runOne(params, c.name);
		}
	}
	else
	{
		runOne(options.params, "result");
	}

	stbi_image_free(guidePixels);
	cuMemFree(d_depth);
	cuMemFree(d_guide);
	cuMemFree(d_out);
	kernel.shutdown();
	cuCtxDestroy(context);

	printf("Done - outputs in %s\n", options.outDir.c_str());
	return 0;
}
