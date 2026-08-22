#include "DepthMeshGeneratorTests.h"
#include "DepthMeshGenerator.h"
#include "MoGeInference.h"
#include "unit_test.h"

#include <opencv2/opencv.hpp>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>

// ---- Helpers ----

/// Build a synthetic MoGe-2 result from a depth map: points unprojected
/// through the given FOV in Mikan camera convention, flat normals facing the
/// camera, full validity mask. Mirrors what MoGeInference::run produces.
static MoGeInference::Result make_synthetic_geometry(const cv::Mat& depth, float fovXDegrees)
{
	const int height= depth.rows;
	const int width= depth.cols;
	const float aspectRatio= (float)width / (float)height;
	const float focal=
		aspectRatio / std::sqrt(1.f + aspectRatio * aspectRatio) / std::tan(fovXDegrees * 3.14159265f / 360.f);
	const float fx= focal / 2.f * std::sqrt(1.f + aspectRatio * aspectRatio) / aspectRatio;
	const float fy= focal / 2.f * std::sqrt(1.f + aspectRatio * aspectRatio);

	MoGeInference::Result geometry;
	geometry.depth= depth.clone();
	geometry.points.create(height, width, CV_32FC3);
	geometry.normals.create(height, width, CV_32FC3);
	geometry.mask= cv::Mat::ones(height, width, CV_8UC1);

	for (int y= 0; y < height; ++y)
	{
		for (int x= 0; x < width; ++x)
		{
			const float d= depth.at<float>(y, x);
			const float u= ((float)x + 0.5f) / (float)width;
			const float v= ((float)y + 0.5f) / (float)height;
			geometry.points.at<cv::Vec3f>(y, x)= cv::Vec3f((u - 0.5f) / fx * d, -(v - 0.5f) / fy * d, -d);
			geometry.normals.at<cv::Vec3f>(y, x)= cv::Vec3f(0.f, 0.f, 1.f);
		}
	}

	return geometry;
}

// ---- Test cases ----

// The shift solver is the one nontrivial piece of the MoGe-2 postprocessing
// port: build an OpenCV-convention point map whose Z was offset by a known
// amount, and require the solver to recover that offset against the focal it
// was built with.
bool depth_mesh_test_shift_solver_recovers_known_shift()
{
	UNIT_TEST_BEGIN("shift solver recovers a known Z offset")

	const int width= 128, height= 96;
	const float fovXDegrees= 60.f;
	const float aspectRatio= (float)width / (float)height;
	const float focal=
		aspectRatio / std::sqrt(1.f + aspectRatio * aspectRatio) / std::tan(fovXDegrees * 3.14159265f / 360.f);
	const float spanX= aspectRatio / std::sqrt(1.f + aspectRatio * aspectRatio);
	const float spanY= 1.f / std::sqrt(1.f + aspectRatio * aspectRatio);

	for (float trueShift : {0.35f, -0.2f, 1.5f})
	{
		// Point map in the RAW model convention (+Y down, +Z forward): a tilted
		// plane so the projection actually constrains the shift, with xy built
		// from the same normalized-view-plane UVs the solver uses.
		cv::Mat points(height, width, CV_32FC3);
		for (int y= 0; y < height; ++y)
		{
			const float v= spanY * (2.f * (float)y / (float)(height - 1) - 1.f) * (float)(height - 1) / (float)height;
			for (int x= 0; x < width; ++x)
			{
				const float u= spanX * (2.f * (float)x / (float)(width - 1) - 1.f) * (float)(width - 1) / (float)width;
				const float depth= 2.f + 0.8f * u + 0.4f * v; // tilted plane, all > 0
				points.at<cv::Vec3f>(y, x)= cv::Vec3f(u * depth / focal, v * depth / focal, depth - trueShift);
			}
		}
		const cv::Mat mask= cv::Mat::ones(height, width, CV_8UC1);

		const float recovered= MoGeInference::solveDepthShift(points, mask, focal);
		if (std::fabs(recovered - trueShift) > 1e-3f)
		{
			fprintf(stdout, "    expected shift %.4f, recovered %.4f\n", trueShift, recovered);
			success= false;
		}
	}

	UNIT_TEST_COMPLETE()
}

// A foreground square floating over a background plane: no triangle may
// connect the two surfaces, and both surfaces must still be fully meshed.
bool depth_mesh_test_discontinuity_is_cut()
{
	UNIT_TEST_BEGIN("triangles never span a depth discontinuity")

	const int size= 32;
	cv::Mat depth(size, size, CV_32FC1, cv::Scalar(3.f));
	cv::Rect foreground(8, 8, 12, 12);
	depth(foreground)= 1.f;

	MoGeInference::Result geometry= make_synthetic_geometry(depth, 60.f);

	DepthMeshGenerator::Config config;
	config.vertexStride= 1;
	config.discontinuityRatio= 1.15f;

	DepthMeshGenerator::Mesh mesh;
	DepthMeshGenerator::Stats stats;
	if (!DepthMeshGenerator::generateMesh(geometry, config, mesh, stats))
	{
		fprintf(stdout, "    generateMesh failed\n");
		success= false;
	}
	else
	{
		// Every vertex is either near (1m) or far (3m); a triangle mixing the
		// two is a skirt the culling must have removed.
		int skirtTriangles= 0;
		for (size_t i= 0; i + 2 < mesh.indices.size(); i+= 3)
		{
			const float za= -mesh.vertices[mesh.indices[i]].z;
			const float zb= -mesh.vertices[mesh.indices[i + 1]].z;
			const float zc= -mesh.vertices[mesh.indices[i + 2]].z;
			const float nearDepth= std::min(za, std::min(zb, zc));
			const float farDepth= std::max(za, std::max(zb, zc));
			if (farDepth > nearDepth * 1.5f)
				skirtTriangles++;
		}
		if (skirtTriangles > 0)
		{
			fprintf(stdout, "    %d skirt triangles crossed the discontinuity\n", skirtTriangles);
			success= false;
		}
		if (stats.culledDiscontinuityEdges == 0)
		{
			fprintf(stdout, "    expected some cells to be culled at the silhouette\n");
			success= false;
		}
		// Both surfaces should survive: the full grid has (size-1)^2 * 2
		// triangles and only the silhouette ring should be missing.
		const size_t fullGridTriangles= (size_t)(size - 1) * (size - 1) * 2;
		if (mesh.getTriangleCount() < fullGridTriangles * 8 / 10)
		{
			fprintf(stdout, "    only %zu of %zu possible triangles were emitted\n", mesh.getTriangleCount(),
					fullGridTriangles);
			success= false;
		}
	}

	UNIT_TEST_COMPLETE()
}

// Invalid-mask pixels and beyond-max-depth pixels must produce no vertices,
// and the depth stats must reflect only what was meshed.
bool depth_mesh_test_masking_and_depth_cut()
{
	UNIT_TEST_BEGIN("mask and max-depth both drop geometry")

	const int size= 16;
	cv::Mat depth(size, size, CV_32FC1, cv::Scalar(2.f));
	// Top rows far beyond the cut, one masked-out corner.
	for (int x= 0; x < size; ++x)
		depth.at<float>(0, x)= 50.f;

	MoGeInference::Result geometry= make_synthetic_geometry(depth, 60.f);
	geometry.mask(cv::Rect(0, size - 4, 4, 4))= 0;

	DepthMeshGenerator::Config config;
	config.vertexStride= 1;
	config.maxDepth= 20.f;

	DepthMeshGenerator::Mesh mesh;
	DepthMeshGenerator::Stats stats;
	if (!DepthMeshGenerator::generateMesh(geometry, config, mesh, stats))
	{
		fprintf(stdout, "    generateMesh failed\n");
		success= false;
	}
	else
	{
		const int expectedVertices= size * size - size /* far row */ - 16 /* masked block */;
		if (stats.validPixelCount != expectedVertices)
		{
			fprintf(stdout, "    expected %d vertices, got %d\n", expectedVertices, stats.validPixelCount);
			success= false;
		}
		if (stats.farDepth > 20.f)
		{
			fprintf(stdout, "    far depth %.1f exceeds the max-depth cut\n", stats.farDepth);
			success= false;
		}
	}

	UNIT_TEST_COMPLETE()
}

// The OBJ writer must emit parallel v/vt/vn counts and 1-based faces - the
// in-editor importer (fast_obj) silently mis-indexes otherwise.
bool depth_mesh_test_obj_export()
{
	UNIT_TEST_BEGIN("obj export writes consistent counts")

	cv::Mat depth(8, 8, CV_32FC1, cv::Scalar(2.f));
	MoGeInference::Result geometry= make_synthetic_geometry(depth, 60.f);

	DepthMeshGenerator::Config config;
	config.vertexStride= 1;

	DepthMeshGenerator::Mesh mesh;
	DepthMeshGenerator::Stats stats;
	success&= DepthMeshGenerator::generateMesh(geometry, config, mesh, stats);

	const std::string objPath= (std::filesystem::temp_directory_path() / "mikan_depth_mesh_test.obj").string();
	if (success && !DepthMeshGenerator::saveObj(mesh, objPath, "test"))
	{
		fprintf(stdout, "    saveObj failed\n");
		success= false;
	}
	if (success)
	{
		std::ifstream file(objPath);
		size_t vCount= 0, vtCount= 0, vnCount= 0, fCount= 0;
		std::string line;
		while (std::getline(file, line))
		{
			if (line.rfind("v ", 0) == 0)
				vCount++;
			else if (line.rfind("vt ", 0) == 0)
				vtCount++;
			else if (line.rfind("vn ", 0) == 0)
				vnCount++;
			else if (line.rfind("f ", 0) == 0)
				fCount++;
		}
		if (vCount != mesh.vertices.size() || vtCount != vCount || vnCount != vCount
			|| fCount != mesh.getTriangleCount())
		{
			fprintf(stdout, "    counts v=%zu vt=%zu vn=%zu f=%zu do not match mesh (%zu verts, %zu tris)\n", vCount,
					vtCount, vnCount, fCount, mesh.vertices.size(), mesh.getTriangleCount());
			success= false;
		}
		// The stream still holds the file open, and on Windows deleting an open
		// file makes the throwing overload of remove terminate the test runner.
		file.close();
		std::filesystem::remove(objPath);
	}

	UNIT_TEST_COMPLETE()
}

// ---- Test Suite ----

bool run_depth_mesh_generator_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("depth_mesh_generator")
	UNIT_TEST_MODULE_CALL_TEST(depth_mesh_test_shift_solver_recovers_known_shift);
	UNIT_TEST_MODULE_CALL_TEST(depth_mesh_test_discontinuity_is_cut);
	UNIT_TEST_MODULE_CALL_TEST(depth_mesh_test_masking_and_depth_cut);
	UNIT_TEST_MODULE_CALL_TEST(depth_mesh_test_obj_export);
	UNIT_TEST_MODULE_END()
}
