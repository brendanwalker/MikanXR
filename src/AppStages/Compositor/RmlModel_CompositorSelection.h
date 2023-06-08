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
	ePropertyDataType field_type;
	ePropertySemantic semantic_type;
	Rml::String field_name;
	Rml::String semantic;
	Rml::Variant valueContainer;

	bool getBoolean();
	void setBoolean(bool value);

	int getInt();
	void setInt(int value);

	Rml::String getString();
	void setString(Rml::String value);

	float getFloat();
	void setFloat(float value);

	float getVec2X();
	void setVector2X(float value);
	float getVec2Y();
	void setVector2Y(float value);

	float getVec3X();
	void setVector3X(float value);
	float getVec3Y();
	void setVector3Y(float value);
	float getVec3Z();
	void setVector3Z(float value);

	float getVec4X();
	void setVector4X(float value);
	float getVec4Y();
	void setVector4Y(float value);
	float getVec4Z();
	void setVector4Z(float value);
	float getVec4W();
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
	bool m_bIgnoreFieldsUpdate= false;

	static bool s_bHasRegisteredTypes;
};
