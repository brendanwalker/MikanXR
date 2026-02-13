#pragma once

class TestLogUtils
{
public:
	static void logComponent(const struct MikanComponentValues& componentInfo);
	static void logComponent(const struct MikanTransformComponentValues& transformInfo);
	static void logComponent(const struct MikanAnchorComponentValues& anchorInfo);
	static void logComponent(const struct MikanStencilComponentValues& stencilInfo);
	static void logComponent(const struct MikanQuadStencilComponentValues& quadInfo);
	static void logComponent(const struct MikanBoxStencilComponentValues& boxInfo);
	static void logComponent(const struct MikanModelStencilComponentValues& modelInfo);
	static void logComponent(const struct MikanVRDeviceComponentValues& vrDeviceInfo);
	static void logModelStencilGeometry(const struct MikanStencilModelRenderGeometry& geometry);
	static void logModelTriMesh(const struct MikanTriagulatedMesh& triMesh);
	static void logTransform(const struct MikanTransform& xform);
};