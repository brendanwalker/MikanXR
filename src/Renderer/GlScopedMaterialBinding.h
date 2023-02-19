#pragma once

class GlScopedMaterialBinding
{
public:
	GlScopedMaterialBinding() : m_boundMaterial(nullptr) {}
	GlScopedMaterialBinding(const class GlMaterial* material) : m_boundMaterial(material) {}
	virtual ~GlScopedMaterialBinding();

	inline const GlMaterial* getBoundMaterial() const { return m_boundMaterial; }
	inline operator bool() const { return m_boundMaterial != nullptr; }

private:
	const class GlMaterial* m_boundMaterial = nullptr;
};
