#include "EditorNodeUtil.h"
#include "GlCommon.h"

#include "imnodes.h"
#include "imnodes_internal.h"

namespace EditorNodeUtil
{
	EditorPinType GLTypeToPinType(GLenum type)
	{
		switch (type)
		{
			case GL_BOOL:
			case GL_INT:
			case GL_UNSIGNED_INT:
				return EditorPinType::INT;
			case GL_BOOL_VEC2:
			case GL_INT_VEC2:
			case GL_UNSIGNED_INT_VEC2:
				return EditorPinType::INT2;
			case GL_BOOL_VEC3:
			case GL_INT_VEC3:
			case GL_UNSIGNED_INT_VEC3:
				return EditorPinType::INT3;
			case GL_BOOL_VEC4:
			case GL_INT_VEC4:
			case GL_UNSIGNED_INT_VEC4:
				return EditorPinType::INT4;

			case GL_FLOAT:
				return EditorPinType::FLOAT;
			case GL_FLOAT_VEC2:
				return EditorPinType::FLOAT2;
			case GL_FLOAT_VEC3:
				return EditorPinType::FLOAT3;
			case GL_FLOAT_VEC4:
				return EditorPinType::FLOAT4;

			case GL_SAMPLER_2D:
				return EditorPinType::TEXTURE;
			case GL_IMAGE_2D:
				return EditorPinType::IMAGE;

			default:
				return EditorPinType::FLOAT;
		}
	}

	int PinTypeSize(EditorPinType type)
	{
		switch (type)
		{
			case EditorPinType::FLOAT:
				return sizeof(float);
			case EditorPinType::FLOAT2:
				return sizeof(float) * 2;
			case EditorPinType::FLOAT3:
				return sizeof(float) * 3;
			case EditorPinType::FLOAT4:
				return sizeof(float) * 4;
			case EditorPinType::INT:
				return sizeof(int);
			case EditorPinType::INT2:
				return sizeof(int) * 2;
			case EditorPinType::INT3:
				return sizeof(int) * 3;
			case EditorPinType::INT4:
				return sizeof(int) * 4;
			default:
				return 0;
		}
	}

	int GLDrawModeToIndex(GLenum drawMode)
	{
		switch (drawMode)
		{
			case GL_POINTS:
				return 0;
			case GL_LINE_STRIP:
				return 1;
			case GL_LINE_LOOP:
				return 2;
			case GL_LINES:
				return 3;
			case GL_LINE_STRIP_ADJACENCY:
				return 4;
			case GL_LINES_ADJACENCY:
				return 5;
			case GL_TRIANGLE_STRIP:
				return 6;
			case GL_TRIANGLE_FAN:
				return 7;
			case GL_TRIANGLES:
				return 8;
			case GL_TRIANGLE_STRIP_ADJACENCY:
				return 9;
			default:
				return 0;
		}
	}

	GLenum IndexToGLDrawMode(int index)
	{
		switch (index)
		{
			case 0:
				return GL_POINTS;
			case 1:
				return GL_LINE_STRIP;
			case 2:
				return GL_LINE_LOOP;
			case 3:
				return GL_LINES;
			case 4:
				return GL_LINE_STRIP_ADJACENCY;
			case 5:
				return GL_LINES_ADJACENCY;
			case 6:
				return GL_TRIANGLE_STRIP;
			case 7:
				return GL_TRIANGLE_FAN;
			case 8:
				return GL_TRIANGLES;
			case 9:
				return GL_TRIANGLE_STRIP_ADJACENCY;
			default:
				return GL_POINTS;
		}
	}

	ImVec2 MousePosToGridSpace()
	{
		ImVec2 canvasOrigin= ImNodes::GetCurrentContext()->CanvasOriginScreenSpace;
		ImVec2 canvasPan= ImNodes::EditorContextGetPanning();
		ImVec2 mousePos = ImGui::GetMousePos();

		return mousePos - canvasOrigin - canvasPan;
	}
};
