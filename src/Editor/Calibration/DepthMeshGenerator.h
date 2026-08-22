#pragma once

//-- includes -----
#include "MoGeInference.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

//-- types -----
/// Turns a MoGe-2 geometry result into a camera-space proxy mesh suitable for
/// a model stencil (shadow catcher).
///
/// The mesh is a regular grid over the depth map with triangles CUT at depth
/// discontinuities: connecting a silhouette edge to the background would
/// create stretched "skirt" triangles that catch shadows in mid-air. Measured
/// on the reference plates, the model's depth edges land within ~1px of the
/// image silhouettes and commit to near-or-far rather than averaging, so
/// ratio-based culling is enough. See docs/reference/scene-lighting.md.
class DepthMeshGenerator
{
public:
	struct Config
	{
		/// Adjacent-vertex depth ratio above which the surface is treated as
		/// discontinuous and no triangle is emitted across it.
		float discontinuityRatio= 1.15f;

		/// Sample every Nth pixel. At stride 4 a 1080p frame still produces
		/// ~250k triangles, which is plenty for a shadow catcher.
		int vertexStride= 4;

		/// Geometry beyond this many metres is dropped entirely. Distant
		/// background adds triangles but cannot receive a meaningful contact
		/// shadow. 0 disables the cut.
		float maxDepth= 20.f;
	};

	struct Mesh
	{
		/// Mikan camera space (+X right, +Y up, -Z forward), metres. Meant to
		/// be parented under the capturing camera's pose.
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		/// Video-frame UVs (v up, OBJ convention), so the plate or a live feed
		/// can be projected back onto the proxy.
		std::vector<glm::vec2> texCoords;
		/// Triangle list, counter-clockwise as seen from the camera.
		std::vector<uint32_t> indices;

		size_t getTriangleCount() const { return indices.size() / 3; }
	};

	struct Stats
	{
		int validPixelCount= 0;
		int culledDiscontinuityEdges= 0;
		float nearDepth= 0.f;
		float farDepth= 0.f;
	};

	static bool generateMesh(const MoGeInference::Result& geometry, const Config& config, Mesh& outMesh,
							 Stats& outStats);

	/// Writes a standalone .obj (positions, normals, UVs, triangles). No .mtl:
	/// the importer supplies a default material when the file declares none.
	static bool saveObj(const Mesh& mesh, const std::string& path, const std::string& objectName);
};
