#pragma once

#include "CommonConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "PropertyInterface.h"
#include "SceneFwd.h"
#include "Shared/RmlModel.h"

#include <map>
#include <vector>

struct RmlModel_ComponentField
{
	class RmlModel_CompositorSelection* parent_model;
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

struct RmlModel_ComponentFunction
{
	int function_index;
	Rml::String function_name;
	Rml::String display_name;
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

	inline TransformComponentPtr getSelectedComponent() { return m_selectedComponentWeakPtr.lock(); }

private:
	void anchorSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void stencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void applyConfigChangesToSelection(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	void updateSelection();
	void rebuildFieldList();
	void appendFieldListToFieldMap(Rml::Vector<RmlModel_ComponentField>& fieldList);
	void rebuildFunctionList();
	void rebuildAnchorList();

	RmlModel_ComponentField* getFieldByPropertyName(const std::string& propertyName);

	AnchorObjectSystemPtr m_anchorSystemPtr;
	EditorObjectSystemPtr m_editorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;

	Rml::Vector<int> m_spatialAnchorIds;
	Rml::Vector<int> m_stencilCullModeValues;

	TransformComponentWeakPtr m_selectedComponentWeakPtr;

	Rml::Vector<RmlModel_ComponentField> m_componentNameFieldModels;
	Rml::Vector<RmlModel_ComponentField> m_componentAnchorIdFieldModels;
	Rml::Vector<RmlModel_ComponentField> m_componentPositionFieldModels;
	Rml::Vector<RmlModel_ComponentField> m_componentRotationFieldModels;
	Rml::Vector<RmlModel_ComponentField> m_componentScaleFieldModels;
	Rml::Vector<RmlModel_ComponentField> m_componentSize3dFieldModels;
	Rml::Vector<RmlModel_ComponentField> m_componentSize2dFieldModels;
	Rml::Vector<RmlModel_ComponentField> m_componentSize1dFieldModels;
	Rml::Vector<RmlModel_ComponentField> m_componentCheckboxFieldModels;
	Rml::Vector<RmlModel_ComponentField> m_componentFilenameFieldModels;
	Rml::Vector<RmlModel_ComponentField> m_componentCullModeFieldModels;
	std::map<std::string, RmlModel_ComponentField*> m_fieldNameToFieldMap;
	int m_fieldCount= 0;
	bool m_bIgnoreFieldsUpdate= false;

	Rml::Vector<RmlModel_ComponentFunction> m_componentFunctionModels;

	static bool s_bHasRegisteredTypes;
};
