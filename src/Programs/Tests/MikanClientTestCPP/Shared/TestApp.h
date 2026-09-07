#pragma once

#include "MikanMathTypes.h"

#if defined(_WIN32)
#include <SDL.h>
#include <SDL_events.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#endif

#include <memory>
#include <string>

using TestGraphicsContextPtr= std::shared_ptr<class TestGraphicsContext>;
using TestMikanClientPtr= std::shared_ptr<class TestMikanClient>;
using IMikanAPIPtr= std::shared_ptr<class IMikanAPI>;

enum class TestRenderMode : int
{
	Color,
	DepthNormalize,
	PackedDepth,
};

class TestApp
{
public:
	TestApp();
	virtual ~TestApp();

	IMikanAPIPtr getMikanAPI() const;
	inline TestRenderMode getRenderMode() const { return m_renderMode; }
	inline const MikanVector3f& getCubeOffset() const { return m_cubeOffset; }
	inline float getTimeSeconds() const { return m_timeSeconds; }

	int exec(int argc, char** argv);
	inline void requestShutdown() { m_bShutdownRequested= true; }

protected:
	bool startup(int argc, char** argv);
	void shutdown();
	void onSDLEvent(SDL_Event& e);

	void update(float deltaSeconds);
	void render();
	void dumpCameraFrame();

private:
	// `-dump <png>`: write the last rendered camera's color target once, a few seconds in, so a
	// headless drive can check what the client actually rendered rather than what a compositor shows
	std::string m_dumpPath;
	bool m_bDumpPending= false;
	static constexpr float k_dumpDelaySeconds= 5.0f;

	TestGraphicsContextPtr m_graphicsContext;
	TestMikanClientPtr m_mikanClient;

	TestRenderMode m_renderMode= TestRenderMode::Color;
	MikanVector3f m_cubeOffset= {0.f, 0.f, 10.f}; // Default +Z offset of cube from camera

	// Flag requesting that we exit the update loop
	bool m_bShutdownRequested= false;

	// Current time
	float m_timeSeconds= 0.f;
	uint32_t m_lastFrameTimestamp= 0;
};