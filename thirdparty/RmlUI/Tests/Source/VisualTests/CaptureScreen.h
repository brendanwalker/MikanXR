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

#ifndef RMLUI_TESTS_VISUALTESTS_CAPTURESCREEN_H
#define RMLUI_TESTS_VISUALTESTS_CAPTURESCREEN_H

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Vertex.h>

class ShellRenderInterfaceOpenGL;

struct ComparisonResult {
	bool skipped = true;
	bool success = false;
	bool is_equal = false;
	double similarity_score = 0;
	std::size_t absolute_difference_sum = 0;
	Rml::String error_msg;
};

struct TextureGeometry {
	Rml::TextureHandle texture_handle = 0;
	Rml::Vertex vertices[4];
	int indices[6] = {};
};

bool CaptureScreenshot(ShellRenderInterfaceOpenGL* shell_renderer, const Rml::String& filename, int clip_width);

ComparisonResult CompareScreenToPreviousCapture(
	ShellRenderInterfaceOpenGL* shell_renderer, const Rml::String& filename, bool write_diff_image, TextureGeometry* out_geometry);

void RenderTextureGeometry(ShellRenderInterfaceOpenGL* shell_renderer, TextureGeometry& geometry);

void ReleaseTextureGeometry(ShellRenderInterfaceOpenGL* shell_renderer, TextureGeometry& geometry);


#endif
