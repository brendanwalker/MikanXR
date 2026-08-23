//-- includes -----
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "MathUtility.h"
#include "SphericalHarmonics.h"
#include "unit_test.h"

//-- helpers -----
namespace
{
// Deterministic RNG so a failure is always reproducible. Anything seeded
// from the clock turns a real regression into a flake.
struct Rng
{
	unsigned int state;

	explicit Rng(unsigned int seed)
		: state(seed)
	{
	}

	float uniform()
	{
		state= state * 1664525u + 1013904223u;
		return (float)((state >> 8) & 0xFFFFFF) / (float)0x1000000;
	}

	glm::vec3 unitVector()
	{
		// Uniform on the sphere: z uniform in [-1,1], azimuth uniform.
		const float z= uniform() * 2.f - 1.f;
		const float phi= uniform() * 6.28318531f;
		const float r= sqrtf(fmaxf(0.f, 1.f - z * z));
		return glm::vec3(r * cosf(phi), r * sinf(phi), z);
	}
};

SHLightingEnvironment makeKnownEnvironment()
{
	SHLightingEnvironment env;
	env.coefficients[0]= glm::vec3(1.00f, 0.90f, 0.80f);
	env.coefficients[1]= glm::vec3(0.20f, 0.15f, 0.10f);
	env.coefficients[2]= glm::vec3(-0.30f, -0.25f, -0.20f);
	env.coefficients[3]= glm::vec3(0.40f, 0.35f, 0.30f);
	env.coefficients[4]= glm::vec3(0.05f, 0.04f, 0.03f);
	env.coefficients[5]= glm::vec3(-0.06f, -0.05f, -0.04f);
	env.coefficients[6]= glm::vec3(0.07f, 0.06f, 0.05f);
	env.coefficients[7]= glm::vec3(-0.08f, -0.07f, -0.06f);
	env.coefficients[8]= glm::vec3(0.09f, 0.08f, 0.07f);
	return env;
}

float maxCoefficientError(const SHLightingEnvironment& a, const SHLightingEnvironment& b)
{
	float worst= 0.f;
	for (int i= 0; i < k_shCoefficientCount; ++i)
	{
		const glm::vec3 d= a.coefficients[i] - b.coefficients[i];
		worst= fmaxf(worst, fmaxf(fabsf(d.r), fmaxf(fabsf(d.g), fabsf(d.b))));
	}
	return worst;
}
} // namespace

//-- public interface -----
bool run_sh_lighting_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("sh_lighting")
	UNIT_TEST_MODULE_CALL_TEST(sh_lighting_test_basis_orthonormal);
	UNIT_TEST_MODULE_CALL_TEST(sh_lighting_test_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(sh_lighting_test_round_trip_hemisphere);
	UNIT_TEST_MODULE_CALL_TEST(sh_lighting_test_band_ridge_shrinks_l2);
	UNIT_TEST_MODULE_CALL_TEST(sh_lighting_test_ringing_is_inherent);
	UNIT_TEST_MODULE_CALL_TEST(sh_lighting_test_directionality);
	UNIT_TEST_MODULE_CALL_TEST(sh_lighting_test_rotation);
	UNIT_TEST_MODULE_CALL_TEST(sh_lighting_test_degenerate_input);
	UNIT_TEST_MODULE_END()
}

//-- private functions -----

// The basis must be orthonormal over the sphere, which is what makes the
// projection in rotated() and the least-squares solve well posed.
bool sh_lighting_test_basis_orthonormal()
{
	UNIT_TEST_BEGIN("basis is orthonormal over the sphere")

	double dot[k_shCoefficientCount][k_shCoefficientCount]= {};
	double totalWeight= 0.0;
	const int thetaSteps= 128, phiSteps= 256;

	for (int ti= 0; ti < thetaSteps; ++ti)
	{
		const double theta= (ti + 0.5) * 3.14159265358979 / thetaSteps;
		const double sinTheta= sin(theta);

		for (int pi= 0; pi < phiSteps; ++pi)
		{
			const double phi= (pi + 0.5) * 2.0 * 3.14159265358979 / phiSteps;
			const glm::vec3 dir((float)(sinTheta * cos(phi)), (float)cos(theta), (float)(sinTheta * sin(phi)));

			float basis[k_shCoefficientCount];
			sh_eval_basis(dir, basis);

			for (int i= 0; i < k_shCoefficientCount; ++i)
				for (int j= 0; j < k_shCoefficientCount; ++j)
					dot[i][j]+= basis[i] * basis[j] * sinTheta;

			totalWeight+= sinTheta;
		}
	}

	const double norm= 4.0 * 3.14159265358979 / totalWeight;
	for (int i= 0; i < k_shCoefficientCount && success; ++i)
	{
		for (int j= 0; j < k_shCoefficientCount && success; ++j)
		{
			const double expected= (i == j) ? 1.0 : 0.0;
			const double actual= dot[i][j] * norm;
			success= fabs(actual - expected) < 1e-3;
			assert(success);
		}
	}

	UNIT_TEST_COMPLETE()
}

// The core guarantee: shading generated from a KNOWN environment must solve
// back to that environment. This isolates the solver from model quality, so a
// failure here is a math regression and nothing else.
bool sh_lighting_test_round_trip()
{
	UNIT_TEST_BEGIN("round trip from known environment")

	const SHLightingEnvironment truth= makeKnownEnvironment();

	SHLightingSolver solver;
	Rng rng(12345u);
	for (int i= 0; i < 20000; ++i)
	{
		const glm::vec3 n= rng.unitVector();
		solver.addSample(n, truth.evalIrradiance(n));
	}

	SHLightingEnvironment solved;
	// bandRidge = 0: this data really does come from an order-2 environment, so
	// the l=2 band carries signal here and must not be penalized.
	success= solver.solve(solved, 0.f);
	assert(success);

	const float error= maxCoefficientError(truth, solved);
	printf("      max coefficient error = %.3e\n", error);
	success&= error < 1e-3f;
	assert(success);

	UNIT_TEST_COMPLETE()
}

// A camera only ever sees front-facing normals, so the realistic case is a
// hemisphere rather than a full sphere. The solve must stay conditioned.
bool sh_lighting_test_round_trip_hemisphere()
{
	UNIT_TEST_BEGIN("round trip from hemisphere of normals")

	const SHLightingEnvironment truth= makeKnownEnvironment();

	SHLightingSolver solver;
	Rng rng(999u);
	for (int i= 0; i < 20000; ++i)
	{
		glm::vec3 n= rng.unitVector();
		n.z= fabsf(n.z); // front facing only
		solver.addSample(n, truth.evalIrradiance(n));
	}

	SHLightingEnvironment solved;
	success= solver.solve(solved, 0.f);
	assert(success);

	const float error= maxCoefficientError(truth, solved);
	printf("      max coefficient error = %.3e\n", error);
	success&= error < 1e-2f;
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Guards the band ridge behavior of SHLightingSolver::solve.
//
// What the ridge actually guarantees is that it SHRINKS THE l=2 BAND. That is
// the property asserted here, because it is the only one that holds
// unconditionally.
//
// It is tempting to assert "negative radiance goes to zero", and on the real
// office plate it does (30.7% -> 0%). But that happened because the l=2 band
// there was over-fit to model error and dominated l=0. Where the scene genuinely
// contains high-frequency light, order-2 SH rings no matter what: fitting a
// clamped cosine leaves ~47% of the sphere negative with or without the ridge.
// So the ridge fixes over-fitting, not inherent ringing, and callers must still
// clamp radiance at evaluation time.
bool sh_lighting_test_band_ridge_shrinks_l2()
{
	UNIT_TEST_BEGIN("band ridge shrinks the l=2 band")

	const SHLightingEnvironment truth= makeKnownEnvironment();

	SHLightingSolver solver;
	Rng rng(4242u);
	for (int i= 0; i < 40000; ++i)
	{
		const glm::vec3 n= rng.unitVector();
		// Directional occlusion: surfaces facing -X are heavily shadowed. This
		// correlates with direction, which is what a real cast shadow does and
		// what an unoccluded environment cannot express, so the solve is forced
		// to absorb the error somewhere.
		const float occlusion= (n.x < 0.f) ? (0.15f + 0.25f * rng.uniform()) : 1.f;
		solver.addSample(n, truth.evalIrradiance(n) * occlusion);
	}

	SHLightingEnvironment unpenalized, penalized;
	success= solver.solve(unpenalized, 0.f);
	assert(success);
	success&= solver.solve(penalized, 0.1f);
	assert(success);

	auto bandEnergy= [](const SHLightingEnvironment& env, int first, int last)
	{
		float sum= 0.f;
		for (int i= first; i <= last; ++i)
			sum+= glm::dot(env.coefficients[i], env.coefficients[i]);
		return sqrtf(sum);
	};

	const float l2Unpenalized= bandEnergy(unpenalized, 4, 8);
	const float l2Penalized= bandEnergy(penalized, 4, 8);
	printf("      l2 band energy: unpenalized %.4f -> penalized %.4f\n", l2Unpenalized, l2Penalized);
	printf("      negative solid angle: unpenalized %.1f%% -> penalized %.1f%%\n",
		   unpenalized.computeNegativeSolidAngleFraction() * 100.f,
		   penalized.computeNegativeSolidAngleFraction() * 100.f);

	success&= (l2Penalized < l2Unpenalized);
	assert(success);

	// The l=0 (ambient) term is what dominates the look and must survive the
	// penalty essentially intact.
	const float ambientDelta= fabsf(bandEnergy(penalized, 0, 0) - bandEnergy(unpenalized, 0, 0));
	success&= (ambientDelta < 0.25f * bandEnergy(unpenalized, 0, 0));
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Order-2 SH cannot represent a clamped cosine, so a directional light rings
// into negative radiance regardless of regularization. This pins that reality
// down so nobody "fixes" it by tuning the ridge, and documents why the
// evaluation path clamps.
bool sh_lighting_test_ringing_is_inherent()
{
	UNIT_TEST_BEGIN("order-2 ringing is inherent for sharp lights")

	glm::vec3 lightDir= glm::normalize(glm::vec3(1.f, 0.3f, 0.2f));

	SHLightingSolver solver;
	Rng rng(777u);
	for (int i= 0; i < 40000; ++i)
	{
		const glm::vec3 n= rng.unitVector();
		const float cosine= fmaxf(0.f, glm::dot(n, lightDir));
		solver.addSample(n, glm::vec3(cosine, cosine * 0.9f, cosine * 0.8f));
	}

	SHLightingEnvironment env;
	success= solver.solve(env, 0.1f);
	assert(success);

	const float negative= env.computeNegativeSolidAngleFraction();
	printf("      clamped cosine leaves %.1f%% of the sphere negative (expected, not a bug)\n", negative * 100.f);

	// Direction must still be recovered correctly despite the ringing - that is
	// the quantity the estimator actually needs.
	const glm::vec3 recovered= env.getDominantDirection();
	const float angleDegrees= acosf(fminf(1.f, fmaxf(-1.f, glm::dot(recovered, lightDir)))) * 57.29578f;
	printf("      dominant direction within %.2f degrees of truth\n", angleDegrees);
	success&= (angleDegrees < 5.f);
	assert(success);

	UNIT_TEST_COMPLETE()
}

// getDirectionality is the confidence signal surfaced in the UI, so it has to
// actually separate a flat environment from a directional one.
bool sh_lighting_test_directionality()
{
	UNIT_TEST_BEGIN("directionality separates flat from directional")

	SHLightingEnvironment ambient;
	ambient.coefficients[0]= glm::vec3(1.f);

	SHLightingEnvironment directional;
	directional.coefficients[0]= glm::vec3(1.f);
	directional.coefficients[3]= glm::vec3(1.5f); // strong +X lobe

	const float flatness= ambient.getDirectionality();
	const float directionality= directional.getDirectionality();
	printf("      ambient l1/l0 = %.3f, directional l1/l0 = %.3f\n", flatness, directionality);

	success= flatness < 1e-6f;
	assert(success);
	success&= directionality > 1.f;
	assert(success);

	// The +X lobe must be recovered as +X.
	const glm::vec3 dir= directional.getDominantDirection();
	success&= (dir.x > 0.99f);
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Normals arrive in camera space and must be rotated into world space, so the
// rotation has to preserve the environment rather than distort it.
bool sh_lighting_test_rotation()
{
	UNIT_TEST_BEGIN("rotation moves the dominant direction correctly")

	SHLightingEnvironment env;
	env.coefficients[0]= glm::vec3(1.f);
	env.coefficients[3]= glm::vec3(1.f); // +X lobe

	// Rotate +90 degrees about Y: +X should map to -Z (right handed).
	const float c= 0.f, s= 1.f;
	const glm::mat3 rotY(glm::vec3(c, 0.f, -s), glm::vec3(0.f, 1.f, 0.f), glm::vec3(s, 0.f, c));

	const SHLightingEnvironment rotated= env.rotated(rotY);
	const glm::vec3 before= env.getDominantDirection();
	const glm::vec3 after= rotated.getDominantDirection();
	const glm::vec3 expected= rotY * before;

	printf("      dominant dir %.2f,%.2f,%.2f -> %.2f,%.2f,%.2f (expected %.2f,%.2f,%.2f)\n", before.x, before.y,
		   before.z, after.x, after.y, after.z, expected.x, expected.y, expected.z);

	success= glm::length(after - expected) < 0.05f;
	assert(success);

	// Ambient must be unchanged by rotation.
	success&= fabsf(rotated.coefficients[0].r - env.coefficients[0].r) < 0.05f;
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool sh_lighting_test_degenerate_input()
{
	UNIT_TEST_BEGIN("degenerate input fails cleanly")

	SHLightingEnvironment env;

	// No samples at all.
	SHLightingSolver empty;
	success= !empty.solve(env);
	assert(success);

	// All samples share one normal: nowhere near enough to constrain 9
	// coefficients. The regularized solve may still return a result, but it
	// must not crash or produce NaN.
	SHLightingSolver degenerate;
	for (int i= 0; i < 1000; ++i)
		degenerate.addSample(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.5f));

	if (degenerate.solve(env))
	{
		for (int i= 0; i < k_shCoefficientCount && success; ++i)
		{
			const glm::vec3& c= env.coefficients[i];
			success= !isnan(c.r) && !isnan(c.g) && !isnan(c.b);
			assert(success);
		}
	}

	// Zero-weight samples must be ignored rather than corrupt the accumulator.
	SHLightingSolver weighted;
	weighted.addSample(glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f), 0.f);
	success&= (weighted.getSampleCount() == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}
