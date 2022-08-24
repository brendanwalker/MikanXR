#pragma once

#include <string>
#include "glm/ext/vector_float4.hpp"

class GlMaterial
{
public:
	GlMaterial() = default;
	GlMaterial(const std::string& name, class GlProgram* program);

	const std::string& getName() const { return m_name; }
	class GlProgram* getProgram() const { return m_program; }

	void setTexture(const class GlTexture* texture) { m_texture = texture; }
	const class GlTexture* getTexture() const { return m_texture; }

	bool bindMaterial() const;
	void unbindMaterial() const;

private:
	std::string m_name;
	class GlProgram* m_program = nullptr;

	// optional material properties
	const class GlTexture* m_texture = nullptr;
};
