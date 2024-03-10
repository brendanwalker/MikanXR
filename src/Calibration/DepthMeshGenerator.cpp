#include "CalibrationRenderHelpers.h"
#include "CalibrationPatternFinder.h"
#include "Graphs/CompositorNodeGraph.h"
#include "CameraMath.h"
#include "Colors.h"
#include "GlCommon.h"
#include "GlLineRenderer.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlScene.h"
#include "GlTexture.h"
#include "GlTriangulatedMesh.h"
#include "IGlWindow.h"
#include "Logger.h"
#include "DepthMeshGenerator.h"
#include "MathTypeConversion.h"
#include "MathOpenCV.h"
#include "MathUtility.h"
#include "ModelStencilComponent.h"
#include "MikanClientTypes.h"
#include "PathUtils.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"
#include "SyntheticDepthEstimator.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"
#include "VRDeviceView.h"

#include <algorithm>
#include <atomic>
#include <thread>

struct DepthMeshCaptureState
{
	IGlWindow* ownerWindow;

	// Static Input
	MikanMonoIntrinsics inputCameraIntrinsics;
	OpenCVCalibrationGeometry inputCVObjectGeometry;
	OpenGLCalibrationGeometry inputGLObjectGeometry;
	ProfileConfigConstPtr profileConfig;

	// Rendering state
	glm::dmat4 cameraToPatternXform;
	std::vector<glm::dvec3> openGLCameraRelativePatternPoints;

	// Generated mesh
	double disparityBias;
	double disparityScale;
	GlRenderModelResourcePtr depthMeshResource;

	void init(
		IGlWindow* owner,
		ProfileConfigConstPtr config, 
		VideoSourceViewPtr videoSourceView)
	{
		ownerWindow= owner;
		profileConfig= config;

		// Get the current camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);
		assert(cameraIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);

		inputCameraIntrinsics = cameraIntrinsics.intrinsics.mono;

		resetCalibration();
	}

	void resetCalibration()
	{
		disparityBias= 0.0;
		disparityScale= 1.0;
		depthMeshResource= nullptr;
	}

	void computeCameraRelativePatternPoints(std::vector<glm::dvec3>& outCameraRelativePoints)
	{
		// Compute the camera relative points of the calibration pattern
		// using the cameraToPatternXform computed by OpenCV
		outCameraRelativePoints.clear();
		for (int pointIndex= 0; pointIndex < inputGLObjectGeometry.points.size(); pointIndex++)
		{
			// Compute the 3D point in the pattern's local space
			const glm::dvec4 localPoint(inputGLObjectGeometry.points[pointIndex], 1.0);

			// Transform the pattern point into the camera's space
			const glm::dvec3 cameraPoint = cameraToPatternXform * localPoint;

			// Store the camera relative point
			outCameraRelativePoints.push_back(cameraPoint);
		}
	}

	void sampleSyntheticDisparityMap(
		CalibrationPatternFinderPtr patternFinder,
		const t_opencv_point2d_list& patternPoints,
		const cv::Mat& syntheticDisparityDnnBuffer,
		std::vector<float>& outSyntheticDepths)
	{

		const float patternFrameWidth = patternFinder->getFrameWidth();
		const float patternFrameHeight = patternFinder->getFrameHeight();
		const float depthFrameWidth = (float)syntheticDisparityDnnBuffer.cols;
		const float depthFrameHeight = (float)syntheticDisparityDnnBuffer.rows;

		// Sample the synthetic disparity map from the camera at the 2d pattern locations
		for (const cv::Point2f& patternPoint : patternPoints)
		{
			// Convert the pattern point to the depth map space
			const int depthX = int((patternPoint.x / patternFrameWidth) * depthFrameWidth);
			const int depthY = int((patternPoint.y / patternFrameHeight) * depthFrameHeight);

			// Sample the synthetic disparity at the pattern point
			const float syntheticDisparity = syntheticDisparityDnnBuffer.at<float>(depthY, depthX);
			outSyntheticDepths.push_back(syntheticDisparity);
		}
	}

	void computeDisparityBiasAndScale(
		SyntheticDepthEstimatorPtr depthEstimator,
		CalibrationPatternFinderPtr patternFinder,
		const t_opencv_point2d_list& imagePoints)
	{
		VideoFrameDistortionView* distortionView = patternFinder->getDistortionView();
		const cv::Mat floatDepthDnnBuffer = depthEstimator->getFloatDepthDnnBufferAccessor();

		// True depth is computed using the formula: Z = 1.0 / (A + (B * synthetic_disparity))
		// where A and B are bias and scale constants needed to map disparity values from the MiDaS DNN model.
		// Given the true depth from the calibration pattern, we can derive A and B using a linear regression
		// of the form y = slope*x + intercept, where y = 1.0 / Z, x = synthetic_disparity, slope = B, intercept = A.

		// Compute camera relative chessboard points using the cameraToPatternXform computed by OpenCV
		computeCameraRelativePatternPoints(openGLCameraRelativePatternPoints);

		// Sample the synthetic disparity map from the camera at the 2d pattern locations
		std::vector<float> dnnSamples;
		sampleSyntheticDisparityMap(patternFinder, imagePoints, floatDepthDnnBuffer, dnnSamples);

		// Merge the real depth and synthetic disparity pairs into a single array
		std::vector<cv::Vec2d> depthDisparityPairs; // (disparity, 1/true_z)
		for (int i = 0; i < imagePoints.size(); i++)
		{
			const glm::dvec3 openGLCameraRelativePoint = openGLCameraRelativePatternPoints[i];

			// Convert the camera relative point to OpenCV coordinate system
			// World units in OpenCV are in mm, not meters
			cv::Vec3d opencvCameraPoint(
				openGLCameraRelativePoint.x * k_meters_to_millimeters,
				-openGLCameraRelativePoint.y * k_meters_to_millimeters,
				-openGLCameraRelativePoint.z * k_meters_to_millimeters);

			const double trueZ = opencvCameraPoint[2];
			const double invTrueZ = 1.0f / trueZ;
			const double syntheticDisparity = dnnSamples[i];

			depthDisparityPairs.push_back(cv::Vec2f(syntheticDisparity, invTrueZ));
		}
		
		// Run a linear regression on the pairs to compute the disparity bias and scale
		cv::Vec4d lineParams;
		cv::fitLine(depthDisparityPairs, lineParams, cv::DIST_L2, 0, 0.01, 0.01);
		const double vx= lineParams[0];
		const double vy= lineParams[1];
		const double x0 = lineParams[2];
		const double y0 = lineParams[3];

		disparityScale = vy / vx;
		disparityBias = y0 - (disparityScale*x0);

		// Compute the RootMeanSquare Error of the linear regression
		double rmsError = 0.0f;
		for (const cv::Vec2d& pair : depthDisparityPairs)
		{
			const double disparity = pair[0];
			const double invTrueZ = pair[1];
			const double trueZ = 1.0f / invTrueZ;

			const double estZ = 1.0f / (disparityBias + (disparityScale * disparity));
			const double error = estZ - trueZ;

			rmsError += error * error;
		}
		rmsError = sqrt(rmsError / (double)imagePoints.size());
	}

	bool createRenderModelResource(
		VideoFrameDistortionViewPtr distortionView,
		SyntheticDepthEstimatorPtr depthEstimator)
	{
		GlModelResourceManager* modelResourceManager = ownerWindow->getModelResourceManager();
		GlShaderCache* shaderCache= ownerWindow->getShaderCache();

		// Create a texture from the undistorted video frame
		GlTexturePtr texture = createDepthMeshTexture(distortionView);
		if (!texture)
		{
			return false;
		}

		// Use the internal basic textured material to render the mesh
		GlMaterialConstPtr stencilMaterial = shaderCache->getMaterialByName(INTERNAL_MATERIAL_PT_TEXTURED);
		GlMaterialInstancePtr materialInstance = std::make_shared<GlMaterialInstance>(stencilMaterial);
		materialInstance->setTextureBySemantic(eUniformSemantic::diffuseTexture, texture);

		// Create a triangulated mesh from the synthetic depth map
		GlTriangulatedMeshPtr triMesh = 
			createTriangulatedDepthMesh(
				distortionView, depthEstimator, materialInstance);
		if (!triMesh)
		{
			return false;
		}

		// Create a render model resource from the mesh and material
		depthMeshResource = std::make_shared<GlRenderModelResource>(ownerWindow);
		depthMeshResource->addTriangulatedMesh(triMesh);

		return true;
	}

	bool loadDepthMesh(ModelStencilDefinitionPtr modelStencilDefinition)
	{
		if (modelStencilDefinition == nullptr)
			return false;

		// Use the internal basic textured material to render the mesh
		GlModelResourceManager* modelResourceManager = ownerWindow->getModelResourceManager();
		GlShaderCache* shaderCache = ownerWindow->getShaderCache();
		GlMaterialConstPtr stencilMaterial = shaderCache->getMaterialByName(INTERNAL_MATERIAL_PT_TEXTURED);

		depthMeshResource= 
			modelResourceManager->fetchRenderModel(
				modelStencilDefinition->getModelPath(), stencilMaterial);

		return depthMeshResource != nullptr;
	}

	bool saveDepthMesh(ModelStencilDefinitionPtr modelStencilDefinition, const glm::mat4& cameraXform)
	{
		if (modelStencilDefinition != nullptr && depthMeshResource != nullptr)
		{
			auto* modelResourceManager= ownerWindow->getModelResourceManager();

			MikanStencilID stencilId= modelStencilDefinition->getStencilId();
			std::string depthMeshResourceName = StringUtils::stringify("depth_mesh_", stencilId);
			auto depthMeshPath= PathUtils::getResourceDirectory() / "models" / (depthMeshResourceName + ".obj");

			depthMeshResource->setName(depthMeshResourceName);
			depthMeshResource->setModelFilePath(depthMeshPath);
			if (modelResourceManager->exportModelToFile(depthMeshResource, depthMeshPath))
			{
				modelStencilDefinition->setIsDepthMesh(true);
				
				// Flush any existing model resource from the cache
				// so that any requests to reload the model will get new data from disk
				modelResourceManager->flushModelByFilePathFromCache(depthMeshPath);

				// This will dirty the stencil definition
				// and cause any associated stencil components to reload their mesh
				modelStencilDefinition->setModelPath(depthMeshPath, true);

				// Snap the stencil component to the camera transform
				auto stencilSystem= StencilObjectSystem::getSystem();
				ModelStencilComponentPtr stencilComponent= stencilSystem->getModelStencilById(stencilId);
				if (stencilComponent != nullptr)
				{
					stencilComponent->setWorldTransform(cameraXform);
				}

				return true;
			}
		}

		return false;
	}

private:
	GlTriangulatedMeshPtr createTriangulatedDepthMesh(
		VideoFrameDistortionViewPtr distortionView,
		SyntheticDepthEstimatorPtr depthEstimator,
		GlMaterialInstancePtr materialInstance)
	{
		// Make sure the material vertex definition has the needed attributes
		GlMaterialConstPtr material = materialInstance->getMaterial();
		const GlVertexDefinition& vertexDefinition = material->getProgram()->getVertexDefinition();
		const size_t vertexSize = (size_t)vertexDefinition.getVertexSize();
		const GlVertexAttribute* posAttrib= vertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::position);
		const GlVertexAttribute* texelAttrib= vertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::texCoord);
		if (posAttrib == nullptr || texelAttrib == nullptr)
		{
			MIKAN_LOG_ERROR("DepthMeshCaptureState::createTriangulatedDepthMesh()") 
				<< "Material vertex definition missing needed attributes";
			return GlTriangulatedMeshPtr();
		}

		// Fetch the camera intrinsics to project pixel coordinates to 3D space
		cv::Matx33d intrinsicMatrix = MikanMatrix3d_to_cv_mat33d(inputCameraIntrinsics.camera_matrix);
		cv::Matx33d invIntrinsicMatrix = intrinsicMatrix.inv();

		// Fetch the synthetic disparity map and the frame dimensions
		cv::Mat syntheticDisparityDnnBuffer = depthEstimator->getFloatDepthDnnBufferAccessor();
		const float frameWidth = (float)distortionView->getFrameWidth();
		const float frameHeight = (float)distortionView->getFrameHeight();
		const int depthFrameWidth = syntheticDisparityDnnBuffer.cols;
		const int depthFrameHeight = syntheticDisparityDnnBuffer.rows;

		// Allocate the mesh vertices based on the material vertex definition
		const uint32_t meshVertexCount = depthFrameWidth * depthFrameHeight;
		uint8_t* meshVertices = new uint8_t[vertexSize * meshVertexCount];

		// Generate the mesh vertices
		{
			const float frameUStep = frameWidth / depthFrameWidth;
			const float frameVStep = frameHeight / depthFrameHeight;

			float frameU = 0.0f;
			float frameV = 0.0f;

			uint8_t* posWritePtr = meshVertices + posAttrib->getOffset();
			uint8_t* texelWritePtr = meshVertices + texelAttrib->getOffset();

			for (int depthV = 0; depthV < depthFrameHeight; depthV++)
			{
				for (int depthU = 0; depthU < depthFrameWidth; depthU++)
				{
					// Fetch depth from the synthetic disparity map
					const float disparity = syntheticDisparityDnnBuffer.at<float>(depthV, depthU);
					const float depth = 1.0f / (disparityBias + (disparityScale * disparity));

					// Compute the 3D point in the camera space
					const cv::Vec3d cameraPoint = invIntrinsicMatrix * cv::Vec3d(frameU, frameV, 1.0);
					const float openCV_x = cameraPoint[0] * depth;
					const float openCV_y = cameraPoint[1] * depth;
					const float openCV_z = depth;

					// Store the 3D point in the vertex array
					// OpenCV -> OpenGL coordinate system transform
					// Rendering world units in meters, not mm
					*((glm::vec3*)posWritePtr) = glm::vec3(
						openCV_x * k_millimeters_to_meters,
						-openCV_y * k_millimeters_to_meters,
						-openCV_z * k_millimeters_to_meters);
					posWritePtr += vertexSize;

					// Store the texture coordinate in the vertex array
					*((glm::vec2*)texelWritePtr) = glm::vec2(
						depthU / (float)depthFrameWidth, 
						1.f - (depthV / (float)depthFrameHeight));
					texelWritePtr+= vertexSize;

					// Advance horizontally the proportional amount in video frame width
					frameU += frameUStep;
				}

				// Advance vertically the proportional amount in the video frame height
				frameU = 0;
				frameV += frameVStep;
			}
		}

		// Allocate the mesh indices
		const uint32_t triangleCount = (depthFrameWidth - 1) * (depthFrameHeight - 1) * 2;
		uint32_t* meshIndices = new uint32_t[triangleCount * 3];

		// Generate the mesh indices
		{
			uint32_t* indexWritePtr = meshIndices;

			for (int depthV = 0; depthV < depthFrameHeight - 1; depthV++)
			{
				for (int depthU = 0; depthU < depthFrameWidth - 1; depthU++)
				{
					// Compute the indices of the quad
					const uint32_t index0 = depthV * depthFrameWidth + depthU;
					const uint32_t index1 = index0 + 1;
					const uint32_t index2 = index0 + depthFrameWidth;
					const uint32_t index3 = index2 + 1;

					// Create two triangles from the quad
					indexWritePtr[0]= index0;
					indexWritePtr[1]= index1;
					indexWritePtr[2]= index2;
					indexWritePtr[3]= index1;
					indexWritePtr[4]= index3;
					indexWritePtr[5]= index2;
					indexWritePtr+= 6;
				}
			}
		}

		// Get the vertex definition associated with the material
		auto depthMeshPtr = std::make_shared<GlTriangulatedMesh>(
			ownerWindow,
			"depth_mesh",
			(const uint8_t*)meshVertices,
			vertexSize,
			meshVertexCount,
			(const uint8_t*)meshIndices,
			sizeof(uint32_t), // 4 bytes per index
			triangleCount,
			true); // mesh owns the vertex data

		// Assign the material instance
		depthMeshPtr->setMaterialInstance(materialInstance);

		if (!depthMeshPtr->createResources())
		{
			MIKAN_LOG_ERROR("DepthMeshCaptureState::createTriangulatedDepthMesh()") << "Failed to create depth mesh";
			return GlTriangulatedMeshPtr();
		}

		return depthMeshPtr;
	}

	GlTexturePtr createDepthMeshTexture(VideoFrameDistortionViewPtr distortionView)
	{
		// Create a texture for the mesh
		cv::Mat* bgrUndistortBuffer = distortionView->getBGRUndistortBuffer();
		auto depthMeshTexture = std::make_shared<GlTexture>(
			bgrUndistortBuffer->cols,
			bgrUndistortBuffer->rows,
			bgrUndistortBuffer->data,
			GL_RGB, // texture format
			GL_BGR); // buffer format
		if (depthMeshTexture->createTexture())
		{
			return depthMeshTexture;
		}

		return GlTexturePtr();
	}
};

//-- MonoDistortionCalibrator ----
DepthMeshGenerator::DepthMeshGenerator(
	IGlWindow* ownerWindow,
	ProfileConfigConstPtr profileConfig,
	VideoFrameDistortionViewPtr distortionView,
	SyntheticDepthEstimatorPtr depthEstimator)
	: m_calibrationState(new DepthMeshCaptureState)
	, m_distortionView(distortionView)
	, m_patternFinder(CalibrationPatternFinder::allocatePatternFinderSharedPtr(profileConfig, distortionView.get()))
	, m_depthEstimator(depthEstimator)
{
	frameWidth = distortionView->getFrameWidth();
	frameHeight = distortionView->getFrameHeight();

	// Private calibration state
	m_calibrationState->init(ownerWindow, profileConfig, distortionView->getVideoSourceView());

	// Cache the 3d geometry of the calibration pattern in the calibration state
	m_patternFinder->getOpenCVSolvePnPGeometry(&m_calibrationState->inputCVObjectGeometry);
	m_patternFinder->getOpenGLSolvePnPGeometry(&m_calibrationState->inputGLObjectGeometry);
}

DepthMeshGenerator::~DepthMeshGenerator()
{
	m_patternFinder= nullptr;
	delete m_calibrationState;
}

bool DepthMeshGenerator::loadMeshFromStencilDefinition(ModelStencilDefinitionPtr stencilDefinition)
{
	if (stencilDefinition->hasValidDepthMesh())
	{
		return m_calibrationState->loadDepthMesh(stencilDefinition);
	}

	return false;
}

bool DepthMeshGenerator::saveMeshToStencilDefinition(
	ModelStencilDefinitionPtr stencilDefinition,
	const glm::mat4& cameraXform)
{
	return m_calibrationState->saveDepthMesh(stencilDefinition, cameraXform);
}

bool DepthMeshGenerator::hasFinishedSampling() const
{
	return m_calibrationState->depthMeshResource != nullptr;
}

void DepthMeshGenerator::resetCalibrationState()
{
	m_calibrationState->resetCalibration();
}

bool DepthMeshGenerator::captureMesh()
{
	// Look for the calibration pattern in the latest video frame
	if (!m_patternFinder->findNewCalibrationPattern())
	{
		return false;
	}

	// Fetch the last found calibration pattern and its bounding quad
	cv::Point2f boundingQuad[4];
	t_opencv_point2d_list imagePoints;
	m_patternFinder->fetchLastFoundCalibrationPattern(imagePoints, boundingQuad);

	// Given an object model and the image points samples we could be able to compute 
	// a position and orientation of the calibration pattern relative to the camera
	cv::Quatd cv_cameraToPatternRot;
	cv::Vec3d cv_cameraToPatternVecMM; // Millimeters
	if (!computeOpenCVCameraRelativePatternTransform(
			m_calibrationState->inputCameraIntrinsics,
			imagePoints,
			m_calibrationState->inputCVObjectGeometry.points,
			cv_cameraToPatternRot,
			cv_cameraToPatternVecMM))
	{
		return false;
	}

	// Convert the OpenCV camera relative pose to a GLM matrix
	convertOpenCVCameraRelativePoseToGLMMat(
		cv_cameraToPatternRot, cv_cameraToPatternVecMM, 
		m_calibrationState->cameraToPatternXform);

	// Compute the synthetic disparity map using the current undistorted video frame
	m_depthEstimator->computeSyntheticDepth(m_distortionView->getBGRUndistortBuffer());

	// Compute the disparity map bias and scale using the calibration pattern
	m_calibrationState->computeDisparityBiasAndScale(
		m_depthEstimator,
		m_patternFinder, 
		imagePoints);

	// Convert the true depth map to a textured mesh
	// Store data in a GlStaticMeshInstancePtr for temp rendering
	m_calibrationState->createRenderModelResource(m_distortionView, m_depthEstimator);

	return true;
}

GlRenderModelResourcePtr DepthMeshGenerator::getCapturedDepthMeshResource() const
{
	return m_calibrationState->depthMeshResource;
}

void DepthMeshGenerator::renderCameraSpaceCalibrationState()
{
	// Draw the most recently capture chessboard in camera space
	m_patternFinder->renderCalibrationPattern2D();

	// Draw the camera relative transforms of the pattern (computed from solvePnP)
	// and the mat puck location offset from the pattern origin
	if (m_calibrationState->depthMeshResource != nullptr)
	{
		const glm::mat4 patternXform = glm::mat4(m_calibrationState->cameraToPatternXform);

		// Compute the mat puck location relative to the mat transform we computed
		ProfileConfigConstPtr config = m_calibrationState->profileConfig;
		const float xOffset = -config->puckHorizontalOffsetMM * k_millimeters_to_meters;
		const float yOffset = config->puckDepthOffsetMM * k_millimeters_to_meters;
		const float zOffset = config->puckVerticalOffsetMM * k_millimeters_to_meters;
		const glm::mat4 matPuckOffsetXform =
			glm::translate(
				glm::mat4(1.0),
				glm::vec3(xOffset, yOffset, zOffset));
		const glm::mat4 matPuckXForm = patternXform * matPuckOffsetXform;

		drawTransformedAxes(patternXform, 0.1f);
		drawTransformedAxes(matPuckXForm, 0.1f);
	}
}