#pragma once

#include "CommonConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "SceneFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

#include <map>
#include <vector>

struct RmlModel_ComponentField
{
	int field_index;
	Rml::String field_name;
	Rml::String semantic;
	Rml::Variant valueContainer;

	bool getBoolean() { return valueContainer.Get<bool>(); }
	void setBoolean(bool value) { valueContainer= value; }

	int getInt() { return valueContainer.Get<int>(); }
	void setInt(int value) { valueContainer= value; }

	Rml::String getString() { return valueContainer.Get<Rml::String>(); }
	void setString(Rml::String value) { valueContainer= value; }

	float getFloat() { return valueContainer.Get<float>(); }
	void setFloat(float value) { valueContainer= value; }

	float getVec2X() { return valueContainer.Get<Rml::Vector2f>().x; }
	float getVec2Y() { return valueContainer.Get<Rml::Vector2f>().y; }

	float getVec3X() { return valueContainer.Get<Rml::Vector3f>().x; }
	float getVec3Y() { return valueContainer.Get<Rml::Vector3f>().y; }
	float getVec3Z() { return valueContainer.Get<Rml::Vector3f>().z; }

	float getVec4X() { return valueContainer.Get<Rml::Vector4f>().x; }
	float getVec4Y() { return valueContainer.Get<Rml::Vector4f>().y; }
	float getVec4Z() { return valueContainer.Get<Rml::Vector4f>().z; }
	float getVec4W() { return valueContainer.Get<Rml::Vector4f>().w; }

	void setVector2X(float value);
	void setVector2Y(float value);

	void setVector3X(float value);
	void setVector3Y(float value);
	void setVector3Z(float value);

	void setVector4X(float value);
	void setVector4Y(float value);
	void setVector4Z(float value);
	void setVector4W(float value);
};

class RmlModel_CompositorSelection : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		AnchorObjectSystemPtr anchorSystemPtr,
		EditorObjectSystemPtr editorSystemPtr,
		StencilObjectSystemPtr stencilSystemPtr);
	virtual void dispose() override;

private:
	void anchorSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void stencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void applyConfigChangesToSelection(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	void updateSelection();
	void rebuildFieldList();
	void rebuildAnchorList();

	RmlModel_ComponentField* getFieldByPropertyName(const std::string& propertyName);
	RmlModel_ComponentField* getFieldBySemantic(const std::string& semantic);

	AnchorObjectSystemPtr m_anchorSystemPtr;
	EditorObjectSystemPtr m_editorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;

	Rml::Vector<int> m_spatialAnchorIds;

	SceneComponentWeakPtr m_selectedComponentWeakPtr;
	Rml::Vector<RmlModel_ComponentField> m_componentFieldModels;
	std::map<std::string, int> m_fieldNameToIndexMap;

	static bool s_bHasRegisteredTypes;
};
