#include "ModelPointCloudAligner.h"
#include "Colors.h"
#include "MathGLM.h"
#include "MeshColliderComponent.h"
#include "MikanLineRenderer.h"
#include "ModelStencilComponent.h"
#include "StaticMeshKdTree.h"
#include "TransformComponent.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <functional>
#include <map>
#include <numeric>

#include "opencv2/opencv.hpp"

// -- internal types / helpers -----
namespace
{
// A collider plus its fixed offset within the stencil (colliderWorld = stencilWorld * localOffset).
struct ColliderRef
{
	MeshColliderComponentPtr collider;
	glm::mat4 localOffset= glm::mat4(1.f);
	glm::mat4 invLocalOffset= glm::mat4(1.f);
};

// Oriented bounding box, axes sorted by descending half-extent.
struct GlmObb
{
	glm::vec3 center= glm::vec3(0.f);
	glm::vec3 axis[3]= {glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)};
	float halfExtent[3]= {0.f, 0.f, 0.f};
};

int64_t voxelKey(int x, int y, int z)
{
	// Pack three 21-bit signed cell indices into one key (sufficient range for real scenes)
	const int64_t bias= 1 << 20;
	return ((int64_t)(x + bias) << 42) | ((int64_t)(y + bias) << 21) | (int64_t)(z + bias);
}

std::vector<glm::vec3> voxelDownsample(const std::vector<glm::vec3>& points, float voxelSize)
{
	if (voxelSize <= 0.f)
		return points;

	std::map<int64_t, std::pair<glm::vec3, int>> cells;
	for (const glm::vec3& p : points)
	{
		const int cx= (int)std::floor(p.x / voxelSize);
		const int cy= (int)std::floor(p.y / voxelSize);
		const int cz= (int)std::floor(p.z / voxelSize);
		auto& entry= cells[voxelKey(cx, cy, cz)];
		entry.first+= p;
		entry.second++;
	}

	std::vector<glm::vec3> result;
	result.reserve(cells.size());
	for (const auto& kv : cells)
		result.push_back(kv.second.first / (float)kv.second.second);

	return result;
}

std::vector<glm::vec3> statisticalOutlierRemoval(const std::vector<glm::vec3>& points, int neighborCount,
												 float stddevMultiplier)
{
	const int n= (int)points.size();
	if (n <= neighborCount + 1)
		return points;

	std::vector<float> meanDistances(n, 0.f);
	for (int i= 0; i < n; ++i)
	{
		std::vector<float> dists;
		dists.reserve(n - 1);
		for (int j= 0; j < n; ++j)
		{
			if (i != j)
				dists.push_back(glm::distance(points[i], points[j]));
		}
		std::partial_sort(dists.begin(), dists.begin() + neighborCount, dists.end());
		float sum= 0.f;
		for (int k= 0; k < neighborCount; ++k)
			sum+= dists[k];
		meanDistances[i]= sum / (float)neighborCount;
	}

	const float globalMean= std::accumulate(meanDistances.begin(), meanDistances.end(), 0.f) / (float)n;
	float variance= 0.f;
	for (float d : meanDistances)
		variance+= (d - globalMean) * (d - globalMean);
	const float stddev= std::sqrt(variance / (float)n);
	const float threshold= globalMean + stddevMultiplier * stddev;

	std::vector<glm::vec3> result;
	result.reserve(n);
	for (int i= 0; i < n; ++i)
	{
		if (meanDistances[i] <= threshold)
			result.push_back(points[i]);
	}

	return result;
}

// Keep only points in the largest connected cluster of occupied voxel cells.
std::vector<glm::vec3> largestCluster(const std::vector<glm::vec3>& points, float voxelSize, float cellRadius)
{
	if (points.size() < 2 || voxelSize <= 0.f)
		return points;

	// Assign points to cells
	std::map<int64_t, std::vector<int>> cellToPoints;
	std::map<int64_t, glm::ivec3> cellCoords;
	for (int i= 0; i < (int)points.size(); ++i)
	{
		const int cx= (int)std::floor(points[i].x / voxelSize);
		const int cy= (int)std::floor(points[i].y / voxelSize);
		const int cz= (int)std::floor(points[i].z / voxelSize);
		const int64_t key= voxelKey(cx, cy, cz);
		cellToPoints[key].push_back(i);
		cellCoords[key]= glm::ivec3(cx, cy, cz);
	}

	// Union-find over occupied cells connected within cellRadius
	std::map<int64_t, int64_t> parent;
	for (const auto& kv : cellToPoints)
		parent[kv.first]= kv.first;

	std::function<int64_t(int64_t)> findRoot= [&](int64_t x)
	{
		while (parent[x] != x)
		{
			parent[x]= parent[parent[x]];
			x= parent[x];
		}
		return x;
	};
	auto unite= [&](int64_t a, int64_t b)
	{
		const int64_t ra= findRoot(a);
		const int64_t rb= findRoot(b);
		if (ra != rb)
			parent[ra]= rb;
	};

	const int radius= std::max(1, (int)std::ceil(cellRadius));
	for (const auto& kv : cellCoords)
	{
		const glm::ivec3 c= kv.second;
		for (int dx= -radius; dx <= radius; ++dx)
			for (int dy= -radius; dy <= radius; ++dy)
				for (int dz= -radius; dz <= radius; ++dz)
				{
					if (dx == 0 && dy == 0 && dz == 0)
						continue;
					const int64_t neighborKey= voxelKey(c.x + dx, c.y + dy, c.z + dz);
					if (cellToPoints.count(neighborKey))
						unite(kv.first, neighborKey);
				}
	}

	// Tally cluster sizes (by point count) and find the largest
	std::map<int64_t, int> clusterSize;
	for (const auto& kv : cellToPoints)
		clusterSize[findRoot(kv.first)]+= (int)kv.second.size();

	int64_t bestRoot= 0;
	int bestSize= -1;
	for (const auto& kv : clusterSize)
	{
		if (kv.second > bestSize)
		{
			bestSize= kv.second;
			bestRoot= kv.first;
		}
	}

	std::vector<glm::vec3> result;
	for (const auto& kv : cellToPoints)
	{
		if (findRoot(kv.first) == bestRoot)
		{
			for (int idx : kv.second)
				result.push_back(points[idx]);
		}
	}

	return result;
}

// PCA-based oriented bounding box: eigenvectors of the covariance give axes; extents from min/max projection.
bool computeCloudObb(const std::vector<glm::vec3>& points, GlmObb& outObb)
{
	const int n= (int)points.size();
	if (n < 3)
		return false;

	glm::vec3 centroid(0.f);
	for (const glm::vec3& p : points)
		centroid+= p;
	centroid/= (float)n;

	// 3x3 covariance
	cv::Mat covariance= cv::Mat::zeros(3, 3, CV_64F);
	for (const glm::vec3& p : points)
	{
		const glm::vec3 d= p - centroid;
		const double dd[3]= {d.x, d.y, d.z};
		for (int r= 0; r < 3; ++r)
			for (int c= 0; c < 3; ++c)
				covariance.at<double>(r, c)+= dd[r] * dd[c];
	}
	covariance*= 1.0 / (double)n;

	cv::Mat eigenValues, eigenVectors;
	if (!cv::eigen(covariance, eigenValues, eigenVectors))
		return false;

	// eigenVectors rows are the eigenvectors
	glm::vec3 axes[3];
	for (int i= 0; i < 3; ++i)
	{
		axes[i]= glm::vec3((float)eigenVectors.at<double>(i, 0), (float)eigenVectors.at<double>(i, 1),
						   (float)eigenVectors.at<double>(i, 2));
		if (glm::length(axes[i]) > 1e-6f)
			axes[i]= glm::normalize(axes[i]);
	}

	// Compute actual half-extents from projected min/max along each axis
	float halfExtents[3];
	for (int a= 0; a < 3; ++a)
	{
		float minProj= FLT_MAX, maxProj= -FLT_MAX;
		for (const glm::vec3& p : points)
		{
			const float proj= glm::dot(p - centroid, axes[a]);
			minProj= std::min(minProj, proj);
			maxProj= std::max(maxProj, proj);
		}
		halfExtents[a]= 0.5f * (maxProj - minProj);
		// Recenter along this axis to the midpoint of the projection
		centroid+= axes[a] * (0.5f * (maxProj + minProj));
	}

	// Sort axes by descending half-extent
	int order[3]= {0, 1, 2};
	std::sort(order, order + 3, [&](int a, int b) { return halfExtents[a] > halfExtents[b]; });

	outObb.center= centroid;
	for (int k= 0; k < 3; ++k)
	{
		outObb.axis[k]= axes[order[k]];
		outObb.halfExtent[k]= halfExtents[order[k]];
	}

	return true;
}

// Umeyama rigid(+scale) fit mapping source -> target. Returns the 4x4 transform and the fitted scale.
glm::mat4 computeUmeyama(const std::vector<glm::vec3>& source, const std::vector<glm::vec3>& target, bool withScale,
						 float minScale, float maxScale, float& outScale)
{
	outScale= 1.f;
	const int n= (int)source.size();
	if (n < 3 || (int)target.size() != n)
		return glm::mat4(1.f);

	glm::vec3 muSrc(0.f), muTgt(0.f);
	for (int i= 0; i < n; ++i)
	{
		muSrc+= source[i];
		muTgt+= target[i];
	}
	muSrc/= (float)n;
	muTgt/= (float)n;

	// Cross-covariance Sigma = (1/n) sum (tgt_c) (src_c)^T  (maps src -> tgt)
	cv::Mat sigma= cv::Mat::zeros(3, 3, CV_64F);
	double srcVariance= 0.0;
	for (int i= 0; i < n; ++i)
	{
		const glm::vec3 s= source[i] - muSrc;
		const glm::vec3 t= target[i] - muTgt;
		const double sd[3]= {s.x, s.y, s.z};
		const double td[3]= {t.x, t.y, t.z};
		for (int r= 0; r < 3; ++r)
			for (int c= 0; c < 3; ++c)
				sigma.at<double>(r, c)+= td[r] * sd[c];
		srcVariance+= (double)glm::dot(s, s);
	}
	sigma*= 1.0 / (double)n;
	srcVariance/= (double)n;

	cv::Mat W, U, Vt;
	cv::SVD::compute(sigma, W, U, Vt, cv::SVD::FULL_UV);

	// R = U * S * Vt, with S correcting for reflection
	cv::Mat S= cv::Mat::eye(3, 3, CV_64F);
	const double detUVt= cv::determinant(U) * cv::determinant(Vt);
	if (detUVt < 0.0)
		S.at<double>(2, 2)= -1.0;

	cv::Mat R= U * S * Vt;

	double scale= 1.0;
	if (withScale && srcVariance > 1e-12)
	{
		const double traceDS= W.at<double>(0) * S.at<double>(0, 0) + W.at<double>(1) * S.at<double>(1, 1)
							  + W.at<double>(2) * S.at<double>(2, 2);
		scale= traceDS / srcVariance;
		scale= std::max((double)minScale, std::min((double)maxScale, scale));
	}
	outScale= (float)scale;

	// t = muTgt - scale * R * muSrc
	const glm::vec3 rotatedScaledMuSrc(
		scale * (R.at<double>(0, 0) * muSrc.x + R.at<double>(0, 1) * muSrc.y + R.at<double>(0, 2) * muSrc.z),
		scale * (R.at<double>(1, 0) * muSrc.x + R.at<double>(1, 1) * muSrc.y + R.at<double>(1, 2) * muSrc.z),
		scale * (R.at<double>(2, 0) * muSrc.x + R.at<double>(2, 1) * muSrc.y + R.at<double>(2, 2) * muSrc.z));
	const glm::vec3 translation= muTgt - rotatedScaledMuSrc;

	// Assemble 4x4 (column-major glm): columns are scale*R basis vectors, last column translation
	glm::mat4 xform(1.f);
	for (int col= 0; col < 3; ++col)
	{
		for (int row= 0; row < 3; ++row)
			xform[col][row]= (float)(scale * R.at<double>(row, col));
	}
	xform[3]= glm::vec4(translation, 1.f);

	return xform;
}
} // namespace

// -- aligner state -----
struct ModelAlignerState
{
	std::vector<ColliderRef> colliders;

	// Model oriented bounds in stencil-local space (axis-aligned there, so axes are identity basis)
	glm::vec3 modelLocalCenter= glm::vec3(0.f);
	float modelLocalHalfExtent[3]= {0.f, 0.f, 0.f};

	std::vector<glm::vec3> segmentedCloud;
	IcpResult lastResult;

	// Correspondences from the final iteration (for overlay): pairs of (model surface point, cloud point)
	std::vector<glm::vec3> corrModelPoints;
	std::vector<glm::vec3> corrCloudPoints;
};

// -- ModelPointCloudAligner -----
ModelPointCloudAligner::ModelPointCloudAligner(ModelStencilComponentPtr model)
	: m_model(model)
	, m_state(new ModelAlignerState)
{
}

ModelPointCloudAligner::~ModelPointCloudAligner() { delete m_state; }

bool ModelPointCloudAligner::align(const std::vector<NaturalFeaturePoint>& cloud,
								   const glm::mat4& initialModelWorldGuess, const IcpParams& params,
								   IcpResult& outResult)
{
	if (!m_model)
		return false;

	// --- Gather collider references + fixed local offsets, and the model's stencil-local bounds ---
	m_state->colliders.clear();
	const glm::mat4 stencilWorld= m_model->getWorldTransform();
	const glm::mat4 invStencilWorld= glm::inverse(stencilWorld);

	glm::vec3 modelMin(FLT_MAX), modelMax(-FLT_MAX);
	bool hasBounds= false;
	for (MeshColliderComponentPtr collider : m_model->getColliderComponents())
	{
		if (!collider)
			continue;

		ColliderRef ref;
		ref.collider= collider;
		ref.localOffset= invStencilWorld * collider->getWorldTransform();
		ref.invLocalOffset= glm::inverse(ref.localOffset);
		m_state->colliders.push_back(ref);

		glm::vec3 localMin, localMax;
		if (collider->getLocalAABB(localMin, localMax))
		{
			// Transform the 8 corners by the local offset into stencil-local space
			for (int i= 0; i < 8; ++i)
			{
				const glm::vec3 corner((i & 1) ? localMax.x : localMin.x, (i & 2) ? localMax.y : localMin.y,
									   (i & 4) ? localMax.z : localMin.z);
				const glm::vec3 stencilLocalCorner= ref.localOffset * glm::vec4(corner, 1.f);
				modelMin= glm::min(modelMin, stencilLocalCorner);
				modelMax= glm::max(modelMax, stencilLocalCorner);
				hasBounds= true;
			}
		}
	}

	if (m_state->colliders.empty() || !hasBounds)
		return false;

	m_state->modelLocalCenter= 0.5f * (modelMin + modelMax);
	m_state->modelLocalHalfExtent[0]= 0.5f * (modelMax.x - modelMin.x);
	m_state->modelLocalHalfExtent[1]= 0.5f * (modelMax.y - modelMin.y);
	m_state->modelLocalHalfExtent[2]= 0.5f * (modelMax.z - modelMin.z);

	// --- Segment the object cloud ---
	std::vector<glm::vec3> rawPoints;
	rawPoints.reserve(cloud.size());
	for (const NaturalFeaturePoint& point : cloud)
		rawPoints.push_back(point.worldPosition);

	std::vector<glm::vec3> segmented= voxelDownsample(rawPoints, params.voxelSizeMeters);
	segmented= statisticalOutlierRemoval(segmented, 8, 2.0f);
	segmented= largestCluster(segmented, params.voxelSizeMeters, params.clusterCellRadius);
	if ((int)segmented.size() < 3)
		return false;
	m_state->segmentedCloud= segmented;

	// --- Build global-registration hypotheses from OBB alignment ---
	GlmObb cloudObb;
	if (!computeCloudObb(segmented, cloudObb))
		return false;

	// Model OBB in stencil-local space: axis-aligned, so axes are the identity basis; sort by half-extent
	int modelOrder[3]= {0, 1, 2};
	std::sort(modelOrder, modelOrder + 3,
			  [&](int a, int b) { return m_state->modelLocalHalfExtent[a] > m_state->modelLocalHalfExtent[b]; });
	glm::vec3 modelAxisSorted[3];
	float modelHalfSorted[3];
	for (int k= 0; k < 3; ++k)
	{
		glm::vec3 axis(0.f);
		axis[modelOrder[k]]= 1.f;
		modelAxisSorted[k]= axis;
		modelHalfSorted[k]= m_state->modelLocalHalfExtent[modelOrder[k]];
	}

	// Candidate axis permutations of the cloud OBB (identity, plus swap of the two largest when their
	// extents are similar, to cover near-symmetry)
	std::vector<std::array<int, 3>> permutations;
	permutations.push_back({0, 1, 2});
	const float extentRatio= (cloudObb.halfExtent[0] > 1e-5f) ? (cloudObb.halfExtent[1] / cloudObb.halfExtent[0]) : 0.f;
	if (extentRatio > 0.8f)
		permutations.push_back({1, 0, 2});

	std::vector<glm::mat4> hypotheses;
	// Uniform init scale from mean extent ratio (clamped)
	float scaleAccum= 0.f;
	int scaleCount= 0;
	for (int k= 0; k < 3; ++k)
	{
		if (modelHalfSorted[k] > 1e-4f)
		{
			scaleAccum+= cloudObb.halfExtent[k] / modelHalfSorted[k];
			scaleCount++;
		}
	}
	float initScale= (scaleCount > 0) ? (scaleAccum / (float)scaleCount) : 1.f;
	initScale= std::max(params.minScale, std::min(params.maxScale, initScale));

	for (const std::array<int, 3>& perm : permutations)
	{
		// Cloud axes reordered per this permutation
		glm::vec3 cloudAxisPerm[3];
		for (int k= 0; k < 3; ++k)
			cloudAxisPerm[k]= cloudObb.axis[perm[k]];

		// Enumerate sign flips; keep only proper rotations (det ~ +1)
		for (int signMask= 0; signMask < 8; ++signMask)
		{
			glm::vec3 cloudAxisSigned[3];
			for (int k= 0; k < 3; ++k)
			{
				const float sign= (signMask & (1 << k)) ? -1.f : 1.f;
				cloudAxisSigned[k]= sign * cloudAxisPerm[k];
			}

			// R maps modelAxisSorted[k] -> cloudAxisSigned[k]:  R = cloudMat * modelMat^T
			glm::mat3 cloudMat, modelMat;
			for (int k= 0; k < 3; ++k)
			{
				cloudMat[k]= cloudAxisSigned[k]; // column k
				modelMat[k]= modelAxisSorted[k];
			}
			glm::mat3 R= cloudMat * glm::transpose(modelMat);
			if (glm::determinant(R) < 0.f)
				continue; // improper rotation (reflection)

			// T maps stencil-local p -> world: world = scale*R*(p - modelCenter) + cloudCenter
			glm::mat4 T(1.f);
			const glm::mat3 sR= initScale * R;
			for (int col= 0; col < 3; ++col)
				T[col]= glm::vec4(sR[col], 0.f);
			const glm::vec3 translation= cloudObb.center - sR * m_state->modelLocalCenter;
			T[3]= glm::vec4(translation, 1.f);
			hypotheses.push_back(T);
		}
	}

	// The caller's guess competes as an extra hypothesis
	hypotheses.push_back(initialModelWorldGuess);

	// --- Refine each hypothesis with scaled trimmed ICP; keep the lowest-residual result ---
	IcpResult best;
	best.rmsResidualMeters= FLT_MAX;
	bool anyValid= false;

	for (const glm::mat4& hypothesis : hypotheses)
	{
		glm::mat4 T= hypothesis;
		float lastRms= FLT_MAX;
		int iterationsRun= 0;
		int inlierCount= 0;
		float lastScale= 1.f;
		bool converged= false;

		std::vector<glm::vec3> finalModelPoints;
		std::vector<glm::vec3> finalCloudPoints;

		for (int iter= 0; iter < params.maxIterations; ++iter)
		{
			iterationsRun= iter + 1;

			// Build correspondences: closest model surface point (at current T) for each cloud point
			struct Correspondence
			{
				glm::vec3 modelWorld;
				glm::vec3 cloudWorld;
				float distance;
			};
			std::vector<Correspondence> correspondences;
			correspondences.reserve(segmented.size());

			const glm::mat4 invT= glm::inverse(T);
			for (const glm::vec3& c : segmented)
			{
				float bestDist= FLT_MAX;
				glm::vec3 bestModelWorld(0.f);
				for (const ColliderRef& ref : m_state->colliders)
				{
					// world -> stencil-local -> mesh-local
					const glm::vec3 stencilLocal= invT * glm::vec4(c, 1.f);
					const glm::vec3 meshLocal= ref.invLocalOffset * glm::vec4(stencilLocal, 1.f);

					KdTreeClosestPointResult result;
					if (ref.collider->computeClosestPointLocal(meshLocal, result))
					{
						// mesh-local -> stencil-local -> world
						const glm::vec3 stencilLocalClosest= ref.localOffset * glm::vec4(result.position, 1.f);
						const glm::vec3 worldClosest= T * glm::vec4(stencilLocalClosest, 1.f);
						const float dist= glm::distance(c, worldClosest);
						if (dist < bestDist)
						{
							bestDist= dist;
							bestModelWorld= worldClosest;
						}
					}
				}

				if (bestDist <= params.maxCorrespondenceDistMeters)
					correspondences.push_back({bestModelWorld, c, bestDist});
			}

			if ((int)correspondences.size() < 3)
				break;

			// Trimmed rejection: keep the best fraction by distance
			std::sort(correspondences.begin(), correspondences.end(),
					  [](const Correspondence& a, const Correspondence& b) { return a.distance < b.distance; });
			const int keepCount= std::max(3, (int)(correspondences.size() * params.trimFraction));
			correspondences.resize(std::min((int)correspondences.size(), keepCount));

			std::vector<glm::vec3> modelPoints, cloudPoints;
			modelPoints.reserve(correspondences.size());
			cloudPoints.reserve(correspondences.size());
			double sumSq= 0.0;
			for (const Correspondence& corr : correspondences)
			{
				modelPoints.push_back(corr.modelWorld);
				cloudPoints.push_back(corr.cloudWorld);
				sumSq+= (double)corr.distance * corr.distance;
			}
			const float rms= (float)std::sqrt(sumSq / (double)correspondences.size());

			// Solve incremental transform mapping model surface points -> cloud points
			float stepScale= 1.f;
			const glm::mat4 deltaXform= computeUmeyama(modelPoints, cloudPoints, params.estimateUniformScale,
													   params.minScale, params.maxScale, stepScale);

			T= deltaXform * T;
			lastRms= rms;
			inlierCount= (int)correspondences.size();
			lastScale= stepScale;
			finalModelPoints= modelPoints;
			finalCloudPoints= cloudPoints;

			// Convergence: incremental translation + rotation both tiny
			const glm::vec3 deltaTranslation= glm::vec3(deltaXform[3]);
			const float translationDelta= glm::length(deltaTranslation);
			const glm::mat3 deltaRot(deltaXform);
			const float rotTrace= glm::clamp((deltaRot[0][0] + deltaRot[1][1] + deltaRot[2][2]) / stepScale, -1.f, 3.f);
			const float rotAngle= std::acos(glm::clamp((rotTrace - 1.f) * 0.5f, -1.f, 1.f));
			if (translationDelta < params.convergenceDeltaMeters && rotAngle < 1e-3f)
			{
				converged= true;
				break;
			}
		}

		if (lastRms < best.rmsResidualMeters)
		{
			anyValid= true;
			best.converged= converged;
			best.modelWorldTransform= T;
			best.rmsResidualMeters= lastRms;
			best.iterationsRun= iterationsRun;
			best.inlierCount= inlierCount;
			best.scale= lastScale;
			m_state->corrModelPoints= finalModelPoints;
			m_state->corrCloudPoints= finalCloudPoints;
		}
	}

	if (!anyValid)
		return false;

	m_state->lastResult= best;
	outResult= best;

	return true;
}

void ModelPointCloudAligner::renderSegmentedCloud(IMkGraphicsContext* graphicsContext)
{
	for (const glm::vec3& p : m_state->segmentedCloud)
		drawPoint(graphicsContext, glm::mat4(1.f), p, Colors::Cyan, 3.f);
}

void ModelPointCloudAligner::renderResult(IMkGraphicsContext* graphicsContext)
{
	const size_t count= std::min(m_state->corrModelPoints.size(), m_state->corrCloudPoints.size());
	for (size_t i= 0; i < count; ++i)
	{
		drawSegment(graphicsContext, glm::mat4(1.f), m_state->corrCloudPoints[i], m_state->corrModelPoints[i],
					Colors::Yellow, Colors::Red);
	}
}
