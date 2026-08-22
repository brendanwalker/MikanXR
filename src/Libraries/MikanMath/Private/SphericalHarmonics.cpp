//-- includes -----
#include "SphericalHarmonics.h"

#include <cmath>
#include <cstring>

//-- constants -----
// Real SH basis normalization constants, order 2.
static const float k_shBasisConstants[k_shCoefficientCount]= {0.282095f, 0.488603f, 0.488603f, 0.488603f, 1.092548f,
															  1.092548f, 0.315392f, 1.092548f, 0.546274f};

// Lambertian convolution per band: A0 = pi, A1 = 2pi/3, A2 = pi/4.
static const float k_shLambertianFactors[k_shCoefficientCount]= {3.14159265f, 2.09439510f, 2.09439510f,
																 2.09439510f, 0.78539816f, 0.78539816f,
																 0.78539816f, 0.78539816f, 0.78539816f};

//-- free functions -----
void sh_eval_basis(const glm::vec3& direction, float outBasis[k_shCoefficientCount])
{
	const float x= direction.x;
	const float y= direction.y;
	const float z= direction.z;

	outBasis[0]= k_shBasisConstants[0];
	outBasis[1]= k_shBasisConstants[1] * y;
	outBasis[2]= k_shBasisConstants[2] * z;
	outBasis[3]= k_shBasisConstants[3] * x;
	outBasis[4]= k_shBasisConstants[4] * x * y;
	outBasis[5]= k_shBasisConstants[5] * y * z;
	outBasis[6]= k_shBasisConstants[6] * (3.f * z * z - 1.f);
	outBasis[7]= k_shBasisConstants[7] * x * z;
	outBasis[8]= k_shBasisConstants[8] * (x * x - y * y);
}

float sh_lambertian_band_factor(int coefficientIndex)
{
	if (coefficientIndex < 0 || coefficientIndex >= k_shCoefficientCount)
		return 0.f;

	return k_shLambertianFactors[coefficientIndex];
}

//-- SHLightingEnvironment -----
SHLightingEnvironment::SHLightingEnvironment()
{
	for (int i= 0; i < k_shCoefficientCount; ++i)
		coefficients[i]= glm::vec3(0.f);
}

glm::vec3 SHLightingEnvironment::evalRadiance(const glm::vec3& direction) const
{
	float basis[k_shCoefficientCount];
	sh_eval_basis(direction, basis);

	glm::vec3 result(0.f);
	for (int i= 0; i < k_shCoefficientCount; ++i)
		result+= coefficients[i] * basis[i];

	return result;
}

glm::vec3 SHLightingEnvironment::evalRadianceClamped(const glm::vec3& direction) const
{
	const glm::vec3 radiance= evalRadiance(direction);

	return glm::vec3(fmaxf(radiance.r, 0.f), fmaxf(radiance.g, 0.f), fmaxf(radiance.b, 0.f));
}

glm::vec3 SHLightingEnvironment::evalIrradiance(const glm::vec3& normal) const
{
	float basis[k_shCoefficientCount];
	sh_eval_basis(normal, basis);

	glm::vec3 result(0.f);
	for (int i= 0; i < k_shCoefficientCount; ++i)
		result+= coefficients[i] * (basis[i] * k_shLambertianFactors[i]);

	return result;
}

glm::vec3 SHLightingEnvironment::getDominantDirection() const
{
	// The l=1 band is the first moment of the radiance distribution, so its
	// (x, y, z) components point at the light centroid. Basis order is
	// [Y1-1 = y, Y10 = z, Y11 = x], hence the shuffle.
	const glm::vec3 gray((coefficients[3].r + coefficients[3].g + coefficients[3].b) / 3.f,
						 (coefficients[1].r + coefficients[1].g + coefficients[1].b) / 3.f,
						 (coefficients[2].r + coefficients[2].g + coefficients[2].b) / 3.f);

	const float length= glm::length(gray);
	if (length < 1e-9f)
		return glm::vec3(0.f, 0.f, 1.f);

	return gray / length;
}

float SHLightingEnvironment::getDirectionality() const
{
	const float l0= glm::length(coefficients[0]);
	if (l0 < 1e-9f)
		return 0.f;

	const float l1= std::sqrt(glm::dot(coefficients[1], coefficients[1]) + glm::dot(coefficients[2], coefficients[2])
							  + glm::dot(coefficients[3], coefficients[3]));

	return l1 / l0;
}

SHLightingEnvironment SHLightingEnvironment::rotated(const glm::mat3& rotation) const
{
	// Rather than build the analytic 9x9 SH rotation matrix, resample: evaluate
	// the source environment along rotated directions and re-project onto the
	// basis by Monte Carlo integration over a deterministic direction set. For
	// order-2 SH this is exact to well under the accuracy the estimate itself
	// carries, and it avoids a large amount of fiddly Wigner-D code.
	const int k_thetaSteps= 64;
	const int k_phiSteps= 128;

	SHLightingEnvironment result;
	double accum[k_shCoefficientCount][3]= {};
	double totalWeight= 0.0;

	for (int ti= 0; ti < k_thetaSteps; ++ti)
	{
		const double theta= (ti + 0.5) * 3.14159265358979 / k_thetaSteps;
		const double sinTheta= std::sin(theta);
		const double cosTheta= std::cos(theta);

		for (int pi= 0; pi < k_phiSteps; ++pi)
		{
			const double phi= (pi + 0.5) * 2.0 * 3.14159265358979 / k_phiSteps;
			const glm::vec3 dir((float)(sinTheta * std::cos(phi)), (float)cosTheta, (float)(sinTheta * std::sin(phi)));

			// Sample the source environment in its own frame.
			const glm::vec3 radiance= evalRadiance(glm::transpose(rotation) * dir);

			float basis[k_shCoefficientCount];
			sh_eval_basis(dir, basis);

			for (int i= 0; i < k_shCoefficientCount; ++i)
			{
				accum[i][0]+= radiance.r * basis[i] * sinTheta;
				accum[i][1]+= radiance.g * basis[i] * sinTheta;
				accum[i][2]+= radiance.b * basis[i] * sinTheta;
			}
			totalWeight+= sinTheta;
		}
	}

	// Normalize by the sampled solid angle (4*pi over the sphere).
	const double norm= (totalWeight > 0.0) ? (4.0 * 3.14159265358979 / totalWeight) : 0.0;
	for (int i= 0; i < k_shCoefficientCount; ++i)
	{
		result.coefficients[i]=
			glm::vec3((float)(accum[i][0] * norm), (float)(accum[i][1] * norm), (float)(accum[i][2] * norm));
	}

	return result;
}

float SHLightingEnvironment::computeNegativeSolidAngleFraction() const
{
	const int k_thetaSteps= 64;
	const int k_phiSteps= 128;

	double negativeWeight= 0.0;
	double totalWeight= 0.0;

	for (int ti= 0; ti < k_thetaSteps; ++ti)
	{
		const double theta= (ti + 0.5) * 3.14159265358979 / k_thetaSteps;
		const double sinTheta= std::sin(theta);
		const double cosTheta= std::cos(theta);

		for (int pi= 0; pi < k_phiSteps; ++pi)
		{
			const double phi= (pi + 0.5) * 2.0 * 3.14159265358979 / k_phiSteps;
			const glm::vec3 dir((float)(sinTheta * std::cos(phi)), (float)cosTheta, (float)(sinTheta * std::sin(phi)));

			const glm::vec3 radiance= evalRadiance(dir);
			const float gray= (radiance.r + radiance.g + radiance.b) / 3.f;

			if (gray < 0.f)
				negativeWeight+= sinTheta;
			totalWeight+= sinTheta;
		}
	}

	return (totalWeight > 0.0) ? (float)(negativeWeight / totalWeight) : 0.f;
}

//-- SHLightingSolver -----
SHLightingSolver::SHLightingSolver() { reset(); }

void SHLightingSolver::reset()
{
	std::memset(m_ata, 0, sizeof(m_ata));
	std::memset(m_atb, 0, sizeof(m_atb));
	m_sampleCount= 0;
}

void SHLightingSolver::addSample(const glm::vec3& normal, const glm::vec3& shading, float weight)
{
	if (weight <= 0.f)
		return;

	float basis[k_shCoefficientCount];
	sh_eval_basis(normal, basis);

	// Fold the Lambertian convolution into the design row so the solved
	// coefficients come out as radiance rather than irradiance.
	double row[k_shCoefficientCount];
	for (int i= 0; i < k_shCoefficientCount; ++i)
		row[i]= (double)basis[i] * (double)k_shLambertianFactors[i];

	const double w= (double)weight;
	for (int i= 0; i < k_shCoefficientCount; ++i)
	{
		const double wr= w * row[i];
		for (int j= i; j < k_shCoefficientCount; ++j)
			m_ata[i][j]+= wr * row[j];

		m_atb[i][0]+= wr * (double)shading.r;
		m_atb[i][1]+= wr * (double)shading.g;
		m_atb[i][2]+= wr * (double)shading.b;
	}

	m_sampleCount++;
}

bool SHLightingSolver::solve(SHLightingEnvironment& outEnvironment, float bandRidge) const
{
	if (m_sampleCount < k_shCoefficientCount)
		return false;

	// Mirror the upper triangle into a full working matrix.
	double a[k_shCoefficientCount][k_shCoefficientCount];
	double trace= 0.0;
	for (int i= 0; i < k_shCoefficientCount; ++i)
	{
		for (int j= i; j < k_shCoefficientCount; ++j)
		{
			a[i][j]= m_ata[i][j];
			a[j][i]= m_ata[i][j];
		}
		trace+= m_ata[i][i];
	}

	// Penalize the l=2 band (coefficients 4..8), plus a tiny uniform ridge for
	// conditioning. Scaled by the trace so the penalty is invariant to how many
	// samples were accumulated and how bright the scene is.
	const double bandPenalty= (double)bandRidge * trace / (double)k_shCoefficientCount;
	for (int i= 0; i < k_shCoefficientCount; ++i)
	{
		a[i][i]+= 1e-9 * trace;
		if (i >= 4)
			a[i][i]+= bandPenalty;
	}

	// Cholesky decompose (the normal matrix is symmetric positive definite once
	// regularized), then solve the three channels by forward/back substitution.
	double l[k_shCoefficientCount][k_shCoefficientCount]= {};
	for (int i= 0; i < k_shCoefficientCount; ++i)
	{
		for (int j= 0; j <= i; ++j)
		{
			double sum= a[i][j];
			for (int k= 0; k < j; ++k)
				sum-= l[i][k] * l[j][k];

			if (i == j)
			{
				if (sum <= 0.0)
					return false; // not positive definite - degenerate sample set

				l[i][i]= std::sqrt(sum);
			}
			else
			{
				l[i][j]= sum / l[j][j];
			}
		}
	}

	for (int channel= 0; channel < 3; ++channel)
	{
		double y[k_shCoefficientCount];
		for (int i= 0; i < k_shCoefficientCount; ++i)
		{
			double sum= m_atb[i][channel];
			for (int k= 0; k < i; ++k)
				sum-= l[i][k] * y[k];
			y[i]= sum / l[i][i];
		}

		double x[k_shCoefficientCount];
		for (int i= k_shCoefficientCount - 1; i >= 0; --i)
		{
			double sum= y[i];
			for (int k= i + 1; k < k_shCoefficientCount; ++k)
				sum-= l[k][i] * x[k];
			x[i]= sum / l[i][i];
		}

		for (int i= 0; i < k_shCoefficientCount; ++i)
			outEnvironment.coefficients[i][channel]= (float)x[i];
	}

	return true;
}
