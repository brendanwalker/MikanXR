#pragma once

//-- includes -----
#include "MikanMathExport.h"

#include <glm/glm.hpp>

//-- constants -----
/// Order-2 real spherical harmonics: 9 coefficients per color channel.
/// Ramamoorthi & Hanrahan showed these capture ~99% of the diffuse irradiance
/// of a Lambertian surface, which is what makes recovering scene lighting from
/// per-pixel normals plus per-pixel shading a small least-squares problem.
static constexpr int k_shCoefficientCount= 9;

//-- types -----
/// A radiance environment stored as order-2 SH, one set of 9 coefficients per
/// RGB channel. This is the *radiance* of the environment (the Lambertian
/// convolution factors live in the solve), so it can be evaluated directly to
/// produce an environment map.
struct MIKAN_MATH_CLASS SHLightingEnvironment
{
	glm::vec3 coefficients[k_shCoefficientCount];

	SHLightingEnvironment();

	/// Radiance arriving from a direction. May be NEGATIVE: order-2 SH cannot
	/// represent a sharp light, so any directional source rings into negative
	/// lobes (fitting a clamped cosine leaves ~47% of the sphere negative, and
	/// no amount of regularization removes that). Use evalRadianceClamped when
	/// producing anything that will be treated as light.
	glm::vec3 evalRadiance(const glm::vec3& direction) const;

	/// Radiance clamped to non-negative. This is what feeds an environment map
	/// or Unreal, which cannot emit negative light in any case.
	glm::vec3 evalRadianceClamped(const glm::vec3& direction) const;

	/// Lambertian irradiance received by a surface with this normal.
	glm::vec3 evalIrradiance(const glm::vec3& normal) const;

	/// Direction of the light "centroid", from the l=1 band. Only meaningful
	/// when getDirectionality() is high - see the note there.
	glm::vec3 getDominantDirection() const;

	/// Ratio of l=1 band energy to l=0. A confidence signal, NOT a quality
	/// score: near 0 means the recovered environment is essentially uniform
	/// ambient, and the dominant direction is then arbitrary and has been
	/// observed to flip sign under small changes to the solve. Surface this so
	/// a flat estimate is not mistaken for a confident directional one.
	float getDirectionality() const;

	/// Rotate the environment into another frame. Normals come out of the
	/// inference model in camera space, so the recovered environment must be
	/// rotated by the tracked camera pose before it means anything in Mikan
	/// world space.
	SHLightingEnvironment rotated(const glm::mat3& rotation) const;

	/// Fraction of the sphere (solid-angle weighted) where the reconstructed
	/// radiance is negative. Non-zero is non-physical - see
	/// SHLightingSolver's band ridge.
	float computeNegativeSolidAngleFraction() const;
};

/// Accumulates (normal, shading) observations into 9x9 normal equations and
/// solves for the environment that best explains them.
///
/// Memory is O(1) in the number of samples: only A^T*A (9x9 symmetric) and
/// A^T*b (9x3) are retained, so a multi-megapixel frame costs nothing to
/// accumulate. Accumulation is in double precision because hundreds of
/// thousands of samples are summed.
class MIKAN_MATH_CLASS SHLightingSolver
{
public:
	SHLightingSolver();

	void reset();

	/// normal must be unit length and in a consistent frame; shading is the
	/// linear-space diffuse shading observed at that normal.
	void addSample(const glm::vec3& normal, const glm::vec3& shading, float weight= 1.f);

	int getSampleCount() const { return m_sampleCount; }

	/// Solve for the environment. Returns false if too few samples or the
	/// system is singular.
	///
	/// bandRidge penalizes the l=2 band. Real shading violates the
	/// single-unoccluded-environment model through cast shadows and spatially
	/// varying light, and the l=2 band tends to absorb that error rather than
	/// any real signal: on a real office plate an unpenalized fit came out with
	/// more energy in l=2 (0.56) than in l=0 (0.25) and produced radiance that
	/// was negative over 31% of the sphere. Penalizing l=2 took that to 0% for
	/// only 0.025 of R^2.
	///
	/// What this does NOT do is remove ringing in general. Order-2 SH cannot
	/// represent a sharp light at all, so a genuinely directional scene stays
	/// negative over a large solid angle whatever the ridge is set to. Clamp at
	/// evaluation. See docs/reference/scene-lighting.md.
	bool solve(SHLightingEnvironment& outEnvironment, float bandRidge= 0.1f) const;

private:
	// Upper triangle of the symmetric 9x9 normal matrix, row major.
	double m_ata[k_shCoefficientCount][k_shCoefficientCount];
	double m_atb[k_shCoefficientCount][3];
	int m_sampleCount;
};

//-- functions -----
/// Evaluate the 9 order-2 real SH basis functions for a unit direction.
MIKAN_MATH_FUNC(void) sh_eval_basis(const glm::vec3& direction, float outBasis[k_shCoefficientCount]);

/// Per-band Lambertian convolution factor A-hat = [pi, 2pi/3, pi/4] expanded
/// to the 9 coefficient slots.
MIKAN_MATH_FUNC(float) sh_lambertian_band_factor(int coefficientIndex);
