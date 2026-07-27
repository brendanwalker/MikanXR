#pragma once

#include "ComponentFwd.h"
#include "NaturalFeatureCloudBuilder.h" // NaturalFeaturePoint

#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

struct IcpParams
{
	int maxIterations= 40;
	float convergenceDeltaMeters= 1e-4f; // stop when the per-iteration transform delta is below this
	float trimFraction= 0.7f;            // keep the best fraction of correspondences each iteration
	float maxCorrespondenceDistMeters= 0.05f;
	bool estimateUniformScale= true; // fit a single uniform scale in addition to rigid pose
	float minScale= 0.9f;
	float maxScale= 1.1f;

	// Segmentation
	bool removeDominantPlane= false; // off by default: a desk is largely planar, so plane removal is risky
	float voxelSizeMeters= 0.01f;
	float clusterCellRadius= 1.5f; // connectivity radius in voxel cells
};

struct IcpResult
{
	bool converged= false;
	glm::mat4 modelWorldTransform= glm::mat4(1.f);
	float rmsResidualMeters= 0.f;
	int iterationsRun= 0;
	int inlierCount= 0;
	float scale= 1.f;
};

// Registers a ModelStencilComponent's CAD mesh to a sparse natural-feature point cloud.
// Fully automatic: segments the object cloud, seeds multiple OBB-aligned pose hypotheses to
// resolve near-symmetry without a manual seed, then refines each with scaled trimmed ICP and
// keeps the lowest-residual result. All math is glm world space, meters.
class ModelPointCloudAligner
{
public:
	ModelPointCloudAligner(ModelStencilComponentPtr model);
	virtual ~ModelPointCloudAligner();

	// initialModelWorldGuess is added as an extra hypothesis (e.g. the stencil's current placement);
	// registration does not require it to be good.
	bool align(const std::vector<NaturalFeaturePoint>& cloud, const glm::mat4& initialModelWorldGuess,
			   const IcpParams& params, IcpResult& outResult);

	// Overlays (world space)
	void renderSegmentedCloud(class IMkGraphicsContext* graphicsContext);
	void renderResult(class IMkGraphicsContext* graphicsContext);

protected:
	ModelStencilComponentPtr m_model;
	struct ModelAlignerState* m_state;
};
