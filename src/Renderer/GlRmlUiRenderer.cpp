// https://github.com/mikke89/RmlUi/blob/master/Backends/RmlUi_Renderer_GL3.cpp
/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "App.h"
#include "AppStage.h"
#include "GlCommon.h"
#include "GlShaderCache.h"
#include "GlProgram.h"
#include "GlStateStack.h"
#include "GlVertexDefinition.h"
#include "Logger.h"
#include "PathUtils.h"
#include "Renderer.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Vertex.h>
#include <string>

#include "GlRmlUiRenderer.h"

#if defined(_WIN32)
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

#include <glm/gtc/type_ptr.hpp>

#define SCREEN_POSITION_UNIFORM_NAME	"_translate"
#define TRANSFORM_UNIFORM_NAME			"_transform"
#define TEXTURE_UNIFORM_NAME			"_tex"

namespace RmlGfx {
	static const char* shader_main_vertex =
		R""""(
			#version 330 core
			uniform vec2 _translate;
			uniform mat4 _transform;
			layout (location = 0) in vec2 inPosition;
			layout (location = 1) in vec4 inColor0;
			layout (location = 2) in vec2 inTexCoord0;
			out vec2 fragTexCoord;
			out vec4 fragColor;
			void main() {
				fragTexCoord = inTexCoord0;
				fragColor = inColor0;
				vec2 translatedPos = inPosition + _translate.xy;
				vec4 outPos = _transform * vec4(translatedPos, 0, 1);
				gl_Position = outPos;
			}
		)"""";

	static const char* shader_main_fragment_texture =
		R""""(
			#version 330 core
			uniform sampler2D _tex;
			in vec2 fragTexCoord;
			in vec4 fragColor;
			out vec4 finalColor;
			void main() {
				vec4 texColor = texture(_tex, fragTexCoord);
				finalColor = fragColor * texColor;
			}
		)"""";

	static const char* shader_main_fragment_color =
		R""""(
			#version 330 core
			in vec2 fragTexCoord;
			in vec4 fragColor;
			out vec4 finalColor;
			void main() {
				finalColor = fragColor;
			}
		)"""";

	static GlProgramCode color_program_code = GlProgramCode(
		"Rml UI Color Program",
		// vertex shader
		shader_main_vertex,
		//fragment shader
		shader_main_fragment_color)
		.addUniform(SCREEN_POSITION_UNIFORM_NAME, eUniformSemantic::screenPosition)
		.addUniform(TRANSFORM_UNIFORM_NAME, eUniformSemantic::transformMatrix);

	static GlProgramCode texture_program_code = GlProgramCode(
		"Rml UI Texture Program",
		// vertex shader`
		shader_main_vertex,
		//fragment shader
		shader_main_fragment_texture)
		.addUniform(SCREEN_POSITION_UNIFORM_NAME, eUniformSemantic::screenPosition)
		.addUniform(TRANSFORM_UNIFORM_NAME, eUniformSemantic::transformMatrix)
		.addUniform(TEXTURE_UNIFORM_NAME, eUniformSemantic::texture0);

	struct CompiledGeometryData {
		GLuint texture;
		GLuint vao;
		GLuint vbo;
		GLuint ibo;
		GLsizei draw_count;
	};

	struct ShadersData {
		GlProgram* program_color;
		GlProgram* program_texture;
	};

	const GlVertexDefinition* GetVertexDefinition()
	{
		static GlVertexDefinition x_vertexDefinition;

		if (x_vertexDefinition.attributes.size() == 0)
		{
			const int32_t vertexSize = (int32_t)sizeof(Rml::Vertex);
			std::vector<GlVertexAttribute>& attribs = x_vertexDefinition.attributes;

			attribs.push_back(GlVertexAttribute(0, eVertexSemantic::position2f, false, vertexSize, offsetof(Rml::Vertex, position)));
			attribs.push_back(GlVertexAttribute(1, eVertexSemantic::color4b, true, vertexSize, offsetof(Rml::Vertex, colour)));
			attribs.push_back(GlVertexAttribute(2, eVertexSemantic::texel2f, false, vertexSize, offsetof(Rml::Vertex, tex_coord)));

			x_vertexDefinition.vertexSize = vertexSize;
		}

		return &x_vertexDefinition;
	}

	static bool CreateShaders(ShadersData& out_shaders)
	{
		out_shaders = {};

		out_shaders.program_color = GlShaderCache::getInstance()->fetchCompiledGlProgram(&color_program_code);
		if (out_shaders.program_color == nullptr)
		{
			MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile RmlUI color shader";
			return false;
		}

		out_shaders.program_texture = GlShaderCache::getInstance()->fetchCompiledGlProgram(&texture_program_code);
		if (out_shaders.program_texture == nullptr)
		{
			MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile RmlUI color shader";
			return false;
		}

		return true;
	}

} // namespace RmlGfx

namespace RmlInput
{
	Rml::Input::KeyIdentifier ConvertKey(int sdlkey)
	{
		// clang-format off
		switch (sdlkey)
		{
		case SDLK_UNKNOWN:      return Rml::Input::KI_UNKNOWN;
		case SDLK_ESCAPE:       return Rml::Input::KI_ESCAPE;
		case SDLK_SPACE:        return Rml::Input::KI_SPACE;
		case SDLK_0:            return Rml::Input::KI_0;
		case SDLK_1:            return Rml::Input::KI_1;
		case SDLK_2:            return Rml::Input::KI_2;
		case SDLK_3:            return Rml::Input::KI_3;
		case SDLK_4:            return Rml::Input::KI_4;
		case SDLK_5:            return Rml::Input::KI_5;
		case SDLK_6:            return Rml::Input::KI_6;
		case SDLK_7:            return Rml::Input::KI_7;
		case SDLK_8:            return Rml::Input::KI_8;
		case SDLK_9:            return Rml::Input::KI_9;
		case SDLK_a:            return Rml::Input::KI_A;
		case SDLK_b:            return Rml::Input::KI_B;
		case SDLK_c:            return Rml::Input::KI_C;
		case SDLK_d:            return Rml::Input::KI_D;
		case SDLK_e:            return Rml::Input::KI_E;
		case SDLK_f:            return Rml::Input::KI_F;
		case SDLK_g:            return Rml::Input::KI_G;
		case SDLK_h:            return Rml::Input::KI_H;
		case SDLK_i:            return Rml::Input::KI_I;
		case SDLK_j:            return Rml::Input::KI_J;
		case SDLK_k:            return Rml::Input::KI_K;
		case SDLK_l:            return Rml::Input::KI_L;
		case SDLK_m:            return Rml::Input::KI_M;
		case SDLK_n:            return Rml::Input::KI_N;
		case SDLK_o:            return Rml::Input::KI_O;
		case SDLK_p:            return Rml::Input::KI_P;
		case SDLK_q:            return Rml::Input::KI_Q;
		case SDLK_r:            return Rml::Input::KI_R;
		case SDLK_s:            return Rml::Input::KI_S;
		case SDLK_t:            return Rml::Input::KI_T;
		case SDLK_u:            return Rml::Input::KI_U;
		case SDLK_v:            return Rml::Input::KI_V;
		case SDLK_w:            return Rml::Input::KI_W;
		case SDLK_x:            return Rml::Input::KI_X;
		case SDLK_y:            return Rml::Input::KI_Y;
		case SDLK_z:            return Rml::Input::KI_Z;
		case SDLK_SEMICOLON:    return Rml::Input::KI_OEM_1;
		case SDLK_PLUS:         return Rml::Input::KI_OEM_PLUS;
		case SDLK_COMMA:        return Rml::Input::KI_OEM_COMMA;
		case SDLK_MINUS:        return Rml::Input::KI_OEM_MINUS;
		case SDLK_PERIOD:       return Rml::Input::KI_OEM_PERIOD;
		case SDLK_SLASH:        return Rml::Input::KI_OEM_2;
		case SDLK_BACKQUOTE:    return Rml::Input::KI_OEM_3;
		case SDLK_LEFTBRACKET:  return Rml::Input::KI_OEM_4;
		case SDLK_BACKSLASH:    return Rml::Input::KI_OEM_5;
		case SDLK_RIGHTBRACKET: return Rml::Input::KI_OEM_6;
		case SDLK_QUOTEDBL:     return Rml::Input::KI_OEM_7;
		case SDLK_KP_0:         return Rml::Input::KI_NUMPAD0;
		case SDLK_KP_1:         return Rml::Input::KI_NUMPAD1;
		case SDLK_KP_2:         return Rml::Input::KI_NUMPAD2;
		case SDLK_KP_3:         return Rml::Input::KI_NUMPAD3;
		case SDLK_KP_4:         return Rml::Input::KI_NUMPAD4;
		case SDLK_KP_5:         return Rml::Input::KI_NUMPAD5;
		case SDLK_KP_6:         return Rml::Input::KI_NUMPAD6;
		case SDLK_KP_7:         return Rml::Input::KI_NUMPAD7;
		case SDLK_KP_8:         return Rml::Input::KI_NUMPAD8;
		case SDLK_KP_9:         return Rml::Input::KI_NUMPAD9;
		case SDLK_KP_ENTER:     return Rml::Input::KI_NUMPADENTER;
		case SDLK_KP_MULTIPLY:  return Rml::Input::KI_MULTIPLY;
		case SDLK_KP_PLUS:      return Rml::Input::KI_ADD;
		case SDLK_KP_MINUS:     return Rml::Input::KI_SUBTRACT;
		case SDLK_KP_PERIOD:    return Rml::Input::KI_DECIMAL;
		case SDLK_KP_DIVIDE:    return Rml::Input::KI_DIVIDE;
		case SDLK_KP_EQUALS:    return Rml::Input::KI_OEM_NEC_EQUAL;
		case SDLK_BACKSPACE:    return Rml::Input::KI_BACK;
		case SDLK_TAB:          return Rml::Input::KI_TAB;
		case SDLK_CLEAR:        return Rml::Input::KI_CLEAR;
		case SDLK_RETURN:       return Rml::Input::KI_RETURN;
		case SDLK_PAUSE:        return Rml::Input::KI_PAUSE;
		case SDLK_CAPSLOCK:     return Rml::Input::KI_CAPITAL;
		case SDLK_PAGEUP:       return Rml::Input::KI_PRIOR;
		case SDLK_PAGEDOWN:     return Rml::Input::KI_NEXT;
		case SDLK_END:          return Rml::Input::KI_END;
		case SDLK_HOME:         return Rml::Input::KI_HOME;
		case SDLK_LEFT:         return Rml::Input::KI_LEFT;
		case SDLK_UP:           return Rml::Input::KI_UP;
		case SDLK_RIGHT:        return Rml::Input::KI_RIGHT;
		case SDLK_DOWN:         return Rml::Input::KI_DOWN;
		case SDLK_INSERT:       return Rml::Input::KI_INSERT;
		case SDLK_DELETE:       return Rml::Input::KI_DELETE;
		case SDLK_HELP:         return Rml::Input::KI_HELP;
		case SDLK_F1:           return Rml::Input::KI_F1;
		case SDLK_F2:           return Rml::Input::KI_F2;
		case SDLK_F3:           return Rml::Input::KI_F3;
		case SDLK_F4:           return Rml::Input::KI_F4;
		case SDLK_F5:           return Rml::Input::KI_F5;
		case SDLK_F6:           return Rml::Input::KI_F6;
		case SDLK_F7:           return Rml::Input::KI_F7;
		case SDLK_F8:           return Rml::Input::KI_F8;
		case SDLK_F9:           return Rml::Input::KI_F9;
		case SDLK_F10:          return Rml::Input::KI_F10;
		case SDLK_F11:          return Rml::Input::KI_F11;
		case SDLK_F12:          return Rml::Input::KI_F12;
		case SDLK_F13:          return Rml::Input::KI_F13;
		case SDLK_F14:          return Rml::Input::KI_F14;
		case SDLK_F15:          return Rml::Input::KI_F15;
		case SDLK_NUMLOCKCLEAR: return Rml::Input::KI_NUMLOCK;
		case SDLK_SCROLLLOCK:   return Rml::Input::KI_SCROLL;
		case SDLK_LSHIFT:       return Rml::Input::KI_LSHIFT;
		case SDLK_RSHIFT:       return Rml::Input::KI_RSHIFT;
		case SDLK_LCTRL:        return Rml::Input::KI_LCONTROL;
		case SDLK_RCTRL:        return Rml::Input::KI_RCONTROL;
		case SDLK_LALT:         return Rml::Input::KI_LMENU;
		case SDLK_RALT:         return Rml::Input::KI_RMENU;
		case SDLK_LGUI:         return Rml::Input::KI_LMETA;
		case SDLK_RGUI:         return Rml::Input::KI_RMETA;
			/*
			case SDLK_LSUPER:       return Rml::Input::KI_LWIN;
			case SDLK_RSUPER:       return Rml::Input::KI_RWIN;
			*/
		default: break;
		}
		// clang-format on

		return Rml::Input::KI_UNKNOWN;
	}

	int ConvertMouseButton(int button)
	{
		switch (button)
		{
		case SDL_BUTTON_LEFT:
			return 0;
		case SDL_BUTTON_RIGHT:
			return 1;
		case SDL_BUTTON_MIDDLE:
			return 2;
		default:
			return 3;
		}
	}

	int GetKeyModifierState()
	{
		SDL_Keymod sdlMods = SDL_GetModState();

		int retval = 0;

		if (sdlMods & KMOD_CTRL)
			retval |= Rml::Input::KM_CTRL;

		if (sdlMods & KMOD_SHIFT)
			retval |= Rml::Input::KM_SHIFT;

		if (sdlMods & KMOD_ALT)
			retval |= Rml::Input::KM_ALT;

		return retval;
	}
} // namespace RmlSDL


GlRmlUiRender::GlRmlUiRender()
	: shaders(Rml::MakeUnique<RmlGfx::ShadersData>())
{
}

GlRmlUiRender::~GlRmlUiRender()
{
}

bool GlRmlUiRender::startup()
{
	if (!RmlGfx::CreateShaders(*shaders))
		return false;

	Renderer* renderer= Renderer::getInstance();
	viewport_width = renderer->getSDLWindowWidth();
	viewport_height = renderer->getSDLWindowHeight();

	Rml::SetRenderInterface(this);

	return true;
}

void GlRmlUiRender::shutdown()
{
	Rml::SetRenderInterface(nullptr);
}

bool GlRmlUiRender::onSDLEvent(const SDL_Event* event)
{
	bool result = false;

	Rml::Context* context = App::getInstance()->getCurrentAppStage()->getRmlContext();
	if (context != nullptr)
	{
		switch (event->type)
		{
		case SDL_MOUSEMOTION:
			result = context->ProcessMouseMove(event->motion.x, event->motion.y, RmlInput::GetKeyModifierState());
			break;
		case SDL_MOUSEBUTTONDOWN:
			result = context->ProcessMouseButtonDown(
				RmlInput::ConvertMouseButton(event->button.button),
				RmlInput::GetKeyModifierState());
			SDL_CaptureMouse(SDL_TRUE);
			break;
		case SDL_MOUSEBUTTONUP:
			SDL_CaptureMouse(SDL_FALSE);
			result = context->ProcessMouseButtonUp(
				RmlInput::ConvertMouseButton(event->button.button),
				RmlInput::GetKeyModifierState());
			break;
		case SDL_MOUSEWHEEL:
			result = context->ProcessMouseWheel(
				float(-event->wheel.y),
				RmlInput::GetKeyModifierState());
			break;
		case SDL_KEYDOWN:
			result = context->ProcessKeyDown(
				RmlInput::ConvertKey(event->key.keysym.sym),
				RmlInput::GetKeyModifierState());
			if (event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_KP_ENTER)
				result &= context->ProcessTextInput('\n');
			break;
		case SDL_KEYUP:
			result = context->ProcessKeyUp(
				RmlInput::ConvertKey(event->key.keysym.sym),
				RmlInput::GetKeyModifierState());
			break;
		case SDL_TEXTINPUT:
			result = context->ProcessTextInput(Rml::String(&event->text.text[0]));
			break;
		case SDL_WINDOWEVENT:
			{
				switch (event->window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					setViewport(event->window.data1, event->window.data2);
					break;
				case SDL_WINDOWEVENT_LEAVE:
					//context->ProcessMouseLeave();
					break;
				}
			}
		break;
		default:
			break;
		}
	}

	return result;
}

void GlRmlUiRender::setViewport(int width, int height)
{
	Rml::Context* context = App::getInstance()->getCurrentAppStage()->getRmlContext();
	if (context != nullptr)
	{
		context->SetDimensions(Rml::Vector2i(width, height));
	}

	viewport_width = width;
	viewport_height = height;
}

void GlRmlUiRender::beginFrame(Renderer* renderer)
{
	GLState& glState= renderer->getGlStateStack()->pushState();

	RMLUI_ASSERT(viewport_width > 0 && viewport_height > 0);
	glViewport(0, 0, viewport_width, viewport_height);

	glState.disableFlag(eGlStateFlagType::depthTest);

	glClearStencil(0);
	glClearColor(0, 0, 0, 1);
	glState.disableFlag(eGlStateFlagType::cullFace);

	glState.enableFlag(eGlStateFlagType::stencilTest);
	glStencilFunc(GL_ALWAYS, 1, GLuint(-1));
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glState.enableFlag(eGlStateFlagType::blend);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	projection = Rml::Matrix4f::ProjectOrtho(0, (float)viewport_width, (float)viewport_height, 0, -10000, 10000);

	SetTransform(nullptr);
}

void GlRmlUiRender::endFrame(Renderer* renderer) 
{
	glViewport(0, 0, (int)viewport_width, (int)viewport_height);
	
	renderer->getGlStateStack()->popState();
}

void GlRmlUiRender::clear()
{
	glClearStencil(0);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GlRmlUiRender::RenderGeometry(
	Rml::Vertex* vertices, int num_vertices, 
	int* indices, int num_indices, 
	const Rml::TextureHandle texture,
	const Rml::Vector2f& translation)
{
	Rml::CompiledGeometryHandle geometry = CompileGeometry(vertices, num_vertices, indices, num_indices, texture);

	if (geometry)
	{
		RenderCompiledGeometry(geometry, translation);
		ReleaseCompiledGeometry(geometry);
	}
}

Rml::CompiledGeometryHandle GlRmlUiRender::CompileGeometry(
	Rml::Vertex* vertices, int num_vertices, 
	int* indices, int num_indices,
	Rml::TextureHandle texture)
{
	constexpr GLenum draw_usage = GL_STATIC_DRAW;

	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ibo = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Rml::Vertex) * num_vertices, (const void*)vertices, draw_usage);

	RmlGfx::GetVertexDefinition()->applyVertexDefintion();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * num_indices, (const void*)indices, draw_usage);
	glBindVertexArray(0);

	checkGLError(__FILE__, __LINE__);

	RmlGfx::CompiledGeometryData* geometry = new RmlGfx::CompiledGeometryData;
	geometry->texture = (GLuint)texture;
	geometry->vao = vao;
	geometry->vbo = vbo;
	geometry->ibo = ibo;
	geometry->draw_count = num_indices;

	return (Rml::CompiledGeometryHandle)geometry;
}

void GlRmlUiRender::RenderCompiledGeometry(Rml::CompiledGeometryHandle handle, const Rml::Vector2f& translation)
{
	RmlGfx::CompiledGeometryData* geometry = (RmlGfx::CompiledGeometryData*)handle;
	GlProgram* program= nullptr;

	if (geometry->texture)
	{
		program= shaders->program_texture;
		program->bindProgram();

		glBindTexture(GL_TEXTURE_2D, geometry->texture);
		SubmitTransformUniform(ProgramId::Texture);

		program->setVector2Uniform(SCREEN_POSITION_UNIFORM_NAME, glm::vec2(translation.x, translation.y));
	}
	else
	{
		program= shaders->program_color;
		program->bindProgram();

		glBindTexture(GL_TEXTURE_2D, 0);
		SubmitTransformUniform(ProgramId::Color);

		program->setVector2Uniform(SCREEN_POSITION_UNIFORM_NAME, glm::vec2(translation.x, translation.y));
	}

	glBindVertexArray(geometry->vao);
	glDrawElements(GL_TRIANGLES, geometry->draw_count, GL_UNSIGNED_INT, (const GLvoid*)0);

	checkGLError(__FILE__, __LINE__);

	assert(program != nullptr);
	program->unbindProgram();
}

void GlRmlUiRender::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle handle)
{
	RmlGfx::CompiledGeometryData* geometry = (RmlGfx::CompiledGeometryData*)handle;

	glDeleteVertexArrays(1, &geometry->vao);
	glDeleteBuffers(1, &geometry->vbo);
	glDeleteBuffers(1, &geometry->ibo);

	delete geometry;
}

void GlRmlUiRender::EnableScissorRegion(bool enable)
{
	ScissoringState new_state = ScissoringState::Disable;

	if (enable)
		new_state = (transform_active ? ScissoringState::Stencil : ScissoringState::Scissor);

	if (new_state != scissoring_state)
	{
		// Disable old
		if (scissoring_state == ScissoringState::Scissor)
			glDisable(GL_SCISSOR_TEST);
		else if (scissoring_state == ScissoringState::Stencil)
			glStencilFunc(GL_ALWAYS, 1, GLuint(-1));

		// Enable new
		if (new_state == ScissoringState::Scissor)
			glEnable(GL_SCISSOR_TEST);
		else if (new_state == ScissoringState::Stencil)
			glStencilFunc(GL_EQUAL, 1, GLuint(-1));

		scissoring_state = new_state;
	}
}

void GlRmlUiRender::SetScissorRegion(int x, int y, int width, int height)
{
	if (transform_active)
	{
		const float left = float(x);
		const float right = float(x + width);
		const float top = float(y);
		const float bottom = float(y + height);

		Rml::Vertex vertices[4];
		vertices[0].position = { left, top };
		vertices[1].position = { right, top };
		vertices[2].position = { right, bottom };
		vertices[3].position = { left, bottom };

		int indices[6] = { 0, 2, 1, 0, 3, 2 };

		glClear(GL_STENCIL_BUFFER_BIT);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glStencilFunc(GL_ALWAYS, 1, GLuint(-1));
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		RenderGeometry(vertices, 4, indices, 6, 0, Rml::Vector2f(0, 0));

		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilFunc(GL_EQUAL, 1, GLuint(-1));
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	else
	{
		glScissor(x, viewport_height - (y + height), width, height);
	}
}

bool GlRmlUiRender::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
	bool success= true;

	std::string absSourcePath= PathUtils::makeAbsoluteResourceFilePath(source);

	SDL_Surface* sdlSurface = IMG_Load(absSourcePath.c_str());
	if (sdlSurface == nullptr)
	{
		MIKAN_LOG_ERROR("GlRmlUiRender::LoadTexture") << "Failed to load texture: " << source;
		success= false;
	}

	if (success && sdlSurface->format->format != SDL_PIXELFORMAT_RGBA32)
	{
		SDL_Surface* sdlRGBASurface = SDL_ConvertSurfaceFormat(sdlSurface, SDL_PIXELFORMAT_RGBA32, 0);

		if (sdlRGBASurface != nullptr)
		{
			SDL_FreeSurface(sdlSurface);
			sdlSurface= sdlRGBASurface;
		}
		else
		{
			MIKAN_LOG_ERROR("GlRmlUiRender::LoadTexture") << "Failed to convert texture to RGBA32 format: " << source;
			success= false;
		}
	}

	if (success)
	{
		texture_dimensions.x = sdlSurface->w;
		texture_dimensions.y = sdlSurface->h;

		success = GenerateTexture(texture_handle, (const Rml::byte*)sdlSurface->pixels, texture_dimensions);
		if (!success)
		{
			MIKAN_LOG_ERROR("GlRmlUiRender::LoadTexture") << "Failed to generate texture: " << source;
		}
	}

	if (sdlSurface != nullptr)
	{
		SDL_FreeSurface(sdlSurface);
	}

	return success;
}

bool GlRmlUiRender::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions)
{
	GLuint texture_id = 0;
	glGenTextures(1, &texture_id);
	if (texture_id == 0)
	{
		return false;
	}

	glBindTexture(GL_TEXTURE_2D, texture_id);

	GLint internal_format = GL_RGBA8;
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, source_dimensions.x, source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, source);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	texture_handle = (Rml::TextureHandle)texture_id;

	return true;
}

void GlRmlUiRender::ReleaseTexture(Rml::TextureHandle texture_handle)
{
	glDeleteTextures(1, (GLuint*)&texture_handle);
}

void GlRmlUiRender::SetTransform(const Rml::Matrix4f* new_transform)
{
	transform_active = (new_transform != nullptr);
	transform = projection * (new_transform ? *new_transform : Rml::Matrix4f::Identity());
	transform_dirty_state = ProgramId::All;
}

void GlRmlUiRender::SubmitTransformUniform(ProgramId program_id)
{
	if ((int)program_id & (int)transform_dirty_state)
	{
		GlProgram* program= nullptr;

		switch (program_id)
		{
		case ProgramId::Color:
			program= shaders->program_color;
			break;
		case ProgramId::Texture:
			program = shaders->program_texture;
			break;
		}

		if (program != nullptr)
		{
		#ifdef RMLUI_MATRIX_ROW_MAJOR
			#error We are assuming that the Rml is using colomn major matrices, like glm
		#endif
			program->setMatrix4x4Uniform(TRANSFORM_UNIFORM_NAME, glm::make_mat4(transform.data()));
		}

		transform_dirty_state = ProgramId((int)transform_dirty_state & ~(int)program_id);
	}
}