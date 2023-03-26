#include "App.h"
#include "MathGLM.h"
#include "MathFastener.h"
#include "MikanClientTypes.h"
#include "ProfileConfig.h"

#include <glm/gtx/quaternion.hpp>

bool align_stencil_fastener_to_anchor_fastener(
	MikanSpatialFastenerID sourceId, 
	MikanSpatialFastenerID targetId,
	glm::mat4& outNewStencilXform,
	glm::vec3 outNewStencilPoints[3]) 
{
	const ProfileConfig* profile= App::getInstance()->getProfileConfig();

	MikanSpatialFastenerInfo sourceFastenerInfo;
	MikanSpatialFastenerInfo targetFastenerInfo;
	if (profile->getSpatialFastenerInfo(sourceId, sourceFastenerInfo) &&
		profile->getSpatialFastenerInfo(targetId, targetFastenerInfo))
	{
		if (sourceFastenerInfo.parent_object_type == MikanFastenerParentType_Stencil &&
			targetFastenerInfo.parent_object_type == MikanFastenerParentType_SpatialAnchor)
		{
			const MikanSpatialAnchorID sourceAnchorId =
				profile->getStencilParentAnchorId(sourceFastenerInfo.parent_object_id);
			const MikanSpatialAnchorID targetAnchorId = targetFastenerInfo.parent_object_id;

			glm::vec3 stencilPoints[3];
			glm::vec3 anchorPoints[3];
			profile->getFastenerLocalPoints(&sourceFastenerInfo, stencilPoints);
			profile->getFastenerLocalPoints(&targetFastenerInfo, anchorPoints);

			// Anchor edge properties remain constant, so we can compute these up front
			const glm::vec3 anchorEdge0 = anchorPoints[1] - anchorPoints[0];
			const glm::vec3 anchorEdge1 = anchorPoints[2] - anchorPoints[0];
			const glm::vec3 anchorNormal = glm::normalize(glm::cross(anchorEdge0, anchorEdge1));
			const float anchorEdge0Len = glm::length(anchorEdge0);
			const float anchorEdge1Len = glm::length(anchorEdge1);

			// Compute transform to align stencil points to anchor points
			// -------

			// Move stencil points so outStencilPoints is at the origin
			const glm::mat4 translateStencilToOrigin = glm::translate(glm::mat4(1.f), -stencilPoints[0]);
			glm_xform_points(translateStencilToOrigin, stencilPoints, 3);

			// Stretch stencil edge1 to match the length anchor edge 1
			// NOTE: if stencil edge0 isn't perpendicular to edge1 then 
			// this scaling will effect length of edge 0 as well
			glm::mat4 scaleAlongStencilEdge1= glm::mat4(1.f);
			{
				const glm::vec3 stencilEdge1 = stencilPoints[2] - stencilPoints[0];
				const float stencilEdge1Len = glm::length(stencilEdge1);
				const float scaleFactor = anchorEdge1Len / stencilEdge1Len;

				scaleAlongStencilEdge1 = glm_scale_along_axis(stencilEdge1, scaleFactor);
				glm_xform_points(scaleAlongStencilEdge1, stencilPoints, 3);
			}

			// Stretch stencil edge0 to match the length anchor edge 0
			// NOTE: if stencil edge1 isn't perpendicular to edge0 then 
			// this scaling will effect length of edge 1 as well
			glm::mat4 scaleAlongStencilEdge0= glm::mat4(1.f);
			{
				const glm::vec3 stencilEdge0 = stencilPoints[1] - stencilPoints[0];
				const float stencilEdge0Len = glm::length(stencilEdge0);
				const float scaleFactor = anchorEdge0Len / stencilEdge0Len;

				scaleAlongStencilEdge0 = glm_scale_along_axis(stencilEdge0, scaleFactor);
				glm_xform_points(scaleAlongStencilEdge0, stencilPoints, 3);
			}

			// Rotate the stencil normal to align with anchor normal
			glm::mat4 rotateStencilToAnchor= glm::mat4(1.f);
			{
				const glm::vec3 stencilEdge0 = stencilPoints[1] - stencilPoints[0];
				const glm::vec3 stencilEdge1 = stencilPoints[2] - stencilPoints[0];
				const glm::vec3 stencilNormal = glm::normalize(glm::cross(stencilEdge0, stencilEdge1));

				// Rotate the plane stencil normal to align with the anchor normals
				rotateStencilToAnchor = glm::mat4_cast(glm::rotation(stencilNormal, anchorNormal));
				glm_xform_points(rotateStencilToAnchor, stencilPoints, 3);
			}

			// Translate the stencil back to the anchor
			const glm::mat4 translateStencilToAnchor = glm::translate(glm::mat4(1.f), anchorPoints[0]);
			glm_xform_points(translateStencilToAnchor, stencilPoints, 3);

			// If the source and target fasteners are children of different anchors
			// append the relative transform from source to target anchor
			glm::mat4 anchorRelativeXform = glm::mat4(1.f);
			if (sourceAnchorId != targetAnchorId)
			{
				glm::mat4 sourceAnchorXform = glm::mat4(1.f);
				profile->getSpatialAnchorWorldTransform(sourceAnchorId, sourceAnchorXform);

				glm::mat4 targetAnchorXform = glm::mat4(1.f);
				profile->getSpatialAnchorWorldTransform(targetAnchorId, targetAnchorXform);

				// Apply the offset transform to target anchor
				anchorRelativeXform = glm::inverse(sourceAnchorXform) * targetAnchorXform;
				glm_xform_points(anchorRelativeXform, stencilPoints, 3);
			}

			// Composite the transforms together to make a net transform
			// that will align the stencil points with the anchor points
			outNewStencilXform = translateStencilToOrigin;
			outNewStencilXform = glm_composite_xform(outNewStencilXform, scaleAlongStencilEdge1);
			outNewStencilXform = glm_composite_xform(outNewStencilXform, scaleAlongStencilEdge0);
			outNewStencilXform = glm_composite_xform(outNewStencilXform, rotateStencilToAnchor);
			outNewStencilXform = glm_composite_xform(outNewStencilXform, translateStencilToAnchor);
			if (sourceAnchorId != targetAnchorId)
			{
				outNewStencilXform = glm_composite_xform(outNewStencilXform, anchorRelativeXform);
			}

			// Also return the new fastener points on the stencil
			memcpy(outNewStencilPoints, &stencilPoints, 3*sizeof(glm::vec3));

			return true;
		}
	}

	return false;
}