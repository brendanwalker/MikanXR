#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "FileBrowser/ModalDialog_FileBrowser.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "SelectionComponent.h"
#include "StencilObjectSystem.h"
#include "SceneComponent.h"
#include "RmlModel_CompositorSelection.h"
#include "ProfileConfig.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorSelection::s_bHasRegisteredTypes = false;

// -- RmlModel_ComponentField -----
bool RmlModel_ComponentField::getBoolean() 
{ 
	return field_type == ePropertyDataType::datatype_bool ? valueContainer.Get<bool>() : false; 
}
void RmlModel_ComponentField::setBoolean(bool value) 
{
	if (field_type == ePropertyDataType::datatype_bool)
		valueContainer = value; 
}

int RmlModel_ComponentField::getInt() 
{
	return field_type == ePropertyDataType::datatype_int ? valueContainer.Get<int>() : 0; 
}
void RmlModel_ComponentField::setInt(int value) 
{ 
	if (field_type == ePropertyDataType::datatype_int)
		valueContainer = value; 
}

Rml::String RmlModel_ComponentField::getString() 
{ 
	return field_type == ePropertyDataType::datatype_string ? valueContainer.Get<Rml::String>() : "";
}
void RmlModel_ComponentField::setString(Rml::String value)
{ 
	if (field_type == ePropertyDataType::datatype_string)
		valueContainer = value; 
}

float RmlModel_ComponentField::getFloat() 
{ 
	return field_type == ePropertyDataType::datatype_float ? valueContainer.Get<float>() : 0.f; 
}
void RmlModel_ComponentField::setFloat(float value) 
{ 
	if (field_type == ePropertyDataType::datatype_float)
		valueContainer = value; 
}

float RmlModel_ComponentField::getVec2X() 
{ 
	return field_type == ePropertyDataType::datatype_float2 ? valueContainer.Get<Rml::Vector2f>().x : 0.f; 
}
void RmlModel_ComponentField::setVector2X(float value)
{
	if (field_type == ePropertyDataType::datatype_float2)
	{
		Rml::Vector2f vec = valueContainer.Get<Rml::Vector2f>();
		vec.x = value;
		valueContainer = vec;
	}
}
float RmlModel_ComponentField::getVec2Y() 
{ 
	return (field_type == ePropertyDataType::datatype_float2) ? valueContainer.Get<Rml::Vector2f>().y : 0.f; 
}
void RmlModel_ComponentField::setVector2Y(float value)
{
	if (field_type == ePropertyDataType::datatype_float2)
	{
		Rml::Vector2f vec = valueContainer.Get<Rml::Vector2f>();
		vec.y = value;
		valueContainer = vec;
	}
}

float RmlModel_ComponentField::getVec3X() 
{ 
	return (field_type == ePropertyDataType::datatype_float3) ? valueContainer.Get<Rml::Vector3f>().x : 0.f; 
}
void RmlModel_ComponentField::setVector3X(float value)
{
	if (field_type == ePropertyDataType::datatype_float3)
	{
		Rml::Vector3f vec = valueContainer.Get<Rml::Vector3f>();
		vec.x = value;
		valueContainer = vec;
	}
}
float RmlModel_ComponentField::getVec3Y() 
{ 
	return (field_type == ePropertyDataType::datatype_float3) ? valueContainer.Get<Rml::Vector3f>().y : 0.f; 
}
void RmlModel_ComponentField::setVector3Y(float value)
{
	if (field_type == ePropertyDataType::datatype_float3)
	{
		Rml::Vector3f vec = valueContainer.Get<Rml::Vector3f>();
		vec.y = value;
		valueContainer = vec;
	}
}
float RmlModel_ComponentField::getVec3Z() 
{ 
	return (field_type == ePropertyDataType::datatype_float3) ? valueContainer.Get<Rml::Vector3f>().z : 0.f; 
}
void RmlModel_ComponentField::setVector3Z(float value)
{
	if (field_type == ePropertyDataType::datatype_float3)
	{
		Rml::Vector3f vec = valueContainer.Get<Rml::Vector3f>();
		vec.z = value;
		valueContainer = vec;
	}
}

float RmlModel_ComponentField::getVec4X() 
{ 
	return (field_type == ePropertyDataType::datatype_float4) ? valueContainer.Get<Rml::Vector4f>().x : 0.f; 
}
void RmlModel_ComponentField::setVector4X(float value)
{
	if (field_type == ePropertyDataType::datatype_float4)
	{
		Rml::Vector4f vec = valueContainer.Get<Rml::Vector4f>();
		vec.x = value;
		valueContainer = vec;
	}
}
float RmlModel_ComponentField::getVec4Y() 
{ 
	return (field_type == ePropertyDataType::datatype_float4) ? valueContainer.Get<Rml::Vector4f>().y : 0.f; 
}
void RmlModel_ComponentField::setVector4Y(float value)
{
	if (field_type == ePropertyDataType::datatype_float4)
	{
		Rml::Vector4f vec = valueContainer.Get<Rml::Vector4f>();
		vec.y = value;
		valueContainer = vec;
	}
}
float RmlModel_ComponentField::getVec4Z() 
{ 
	return (field_type == ePropertyDataType::datatype_float4) ? valueContainer.Get<Rml::Vector4f>().z : 0.f; 
}
void RmlModel_ComponentField::setVector4Z(float value)
{
	if (field_type == ePropertyDataType::datatype_float4)
	{
		Rml::Vector4f vec = valueContainer.Get<Rml::Vector4f>();
		vec.z = value;
		valueContainer = vec;
	}
}
float RmlModel_ComponentField::getVec4W() 
{ 
	return (field_type == ePropertyDataType::datatype_float4) ? valueContainer.Get<Rml::Vector4f>().w : 0.f; 
}
void RmlModel_ComponentField::setVector4W(float value)
{
	if (field_type == ePropertyDataType::datatype_float4)
	{
		Rml::Vector4f vec = valueContainer.Get<Rml::Vector4f>();
		vec.z = value;
		valueContainer = vec;
	}
}

// -- RmlModel_CompositorSelection ------
bool RmlModel_CompositorSelection::init(
	Rml::Context* rmlContext,
	AnchorObjectSystemPtr anchorSystemPtr,
	EditorObjectSystemPtr editorSystemPtr,
	StencilObjectSystemPtr stencilSystemPtr)
{
	m_anchorSystemPtr = anchorSystemPtr;
	m_editorSystemPtr = editorSystemPtr;
	m_stencilSystemPtr = stencilSystemPtr;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_selection");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for component field struct.
		if (auto field_model_handle = constructor.RegisterStruct<RmlModel_ComponentField>())
		{
			field_model_handle.RegisterMember("field_index", &RmlModel_ComponentField::field_index);
			field_model_handle.RegisterMember("field_name", &RmlModel_ComponentField::field_name);
			field_model_handle.RegisterMember("semantic", &RmlModel_ComponentField::semantic);
			field_model_handle.RegisterMember("boolean", &RmlModel_ComponentField::getBoolean, &RmlModel_ComponentField::setBoolean);
			field_model_handle.RegisterMember("int", &RmlModel_ComponentField::getInt, &RmlModel_ComponentField::setInt);
			field_model_handle.RegisterMember("float", &RmlModel_ComponentField::getFloat, &RmlModel_ComponentField::setFloat);
			field_model_handle.RegisterMember("string", &RmlModel_ComponentField::getString, &RmlModel_ComponentField::setString);

			field_model_handle.RegisterMember("vec2_x", &RmlModel_ComponentField::getVec2X, &RmlModel_ComponentField::setVector2X);
			field_model_handle.RegisterMember("vec2_y", &RmlModel_ComponentField::getVec2Y, &RmlModel_ComponentField::setVector2Y);

			field_model_handle.RegisterMember("vec3_x", &RmlModel_ComponentField::getVec3X, &RmlModel_ComponentField::setVector3X);
			field_model_handle.RegisterMember("vec3_y", &RmlModel_ComponentField::getVec3Y, &RmlModel_ComponentField::setVector3Y);
			field_model_handle.RegisterMember("vec3_z", &RmlModel_ComponentField::getVec3Z, &RmlModel_ComponentField::setVector3Z);

			field_model_handle.RegisterMember("vec4_x", &RmlModel_ComponentField::getVec4X, &RmlModel_ComponentField::setVector4X);
			field_model_handle.RegisterMember("vec4_y", &RmlModel_ComponentField::getVec4Y, &RmlModel_ComponentField::setVector4Y);
			field_model_handle.RegisterMember("vec4_z", &RmlModel_ComponentField::getVec4Z, &RmlModel_ComponentField::setVector4Z);
			field_model_handle.RegisterMember("vec4_w", &RmlModel_ComponentField::getVec4W, &RmlModel_ComponentField::setVector4W);
		}

		// One time registration for component function struct
		if (auto function_model_handle = constructor.RegisterStruct<RmlModel_ComponentFunction>())
		{
			function_model_handle.RegisterMember("function_index", &RmlModel_ComponentFunction::function_index);
			function_model_handle.RegisterMember("function_name", &RmlModel_ComponentFunction::function_name);
			function_model_handle.RegisterMember("display_name", &RmlModel_ComponentFunction::display_name);
		}

		// One time registration for an arrays
		constructor.RegisterArray< Rml::Vector<RmlModel_ComponentField> >();
		constructor.RegisterArray< Rml::Vector<RmlModel_ComponentFunction> >();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("field_count", &m_fieldCount);
	constructor.Bind("name_fields", &m_componentNameFieldModels);
	constructor.Bind("anchor_id_fields", &m_componentAnchorIdFieldModels);
	constructor.Bind("position_fields", &m_componentPositionFieldModels);
	constructor.Bind("rotation_fields", &m_componentRotationFieldModels);
	constructor.Bind("scale_fields", &m_componentScaleFieldModels);
	constructor.Bind("size3d_fields", &m_componentSize3dFieldModels);
	constructor.Bind("size2d_fields", &m_componentSize2dFieldModels);
	constructor.Bind("side1d_fields", &m_componentSize1dFieldModels);
	constructor.Bind("checkbox_fields", &m_componentCheckboxFieldModels);
	constructor.Bind("filename_fields", &m_componentFilenameFieldModels);
	
	constructor.Bind("component_functions", &m_componentFunctionModels);
	constructor.Bind("spatial_anchor_ids", &m_spatialAnchorIds);	

	// Bind data model callbacks
	constructor.BindEventCallback(
		"modify_field",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (m_bIgnoreFieldsUpdate)
				return;

			const Rml::String fieldName = (arguments.size() == 1 ? arguments[0].Get<Rml::String>() : "");
			RmlModel_ComponentField* rmlFieldModel= getFieldByPropertyName(fieldName);
			if (rmlFieldModel != nullptr)
			{
				SceneComponentPtr selectedComponent = m_selectedComponentWeakPtr.lock();
				if (selectedComponent)
				{
					if (rmlFieldModel->semantic_type == ePropertySemantic::anchor_id)
					{
						const int new_anchor_id = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

						if (new_anchor_id != INVALID_MIKAN_ID)
						{
							Rml::Variant value;
							value= new_anchor_id;

							selectedComponent->setPropertyValue(fieldName, value);
						}
					}
					if (rmlFieldModel->semantic_type == ePropertySemantic::name)
					{
						const bool isLineBreak = ev.GetParameter("linebreak", false);

						if (isLineBreak)
						{
							Rml::Variant value;
							value= ev.GetParameter("value", Rml::String());

							selectedComponent->setPropertyValue(fieldName, value);
						}
					}
					else
					{
						selectedComponent->setPropertyValue(fieldName, rmlFieldModel->valueContainer);
					}
				}
			}
		});
	constructor.BindEventCallback(
		"select_filepath",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int field_index = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (field_index >= 0)
			{
				const RmlModel_ComponentField& rmlFieldModel = m_componentFilenameFieldModels[field_index];
				const std::string propertyName = rmlFieldModel.field_name;

				SceneComponentPtr selectedComponent = m_selectedComponentWeakPtr.lock();
				if (selectedComponent)
				{					
					std::filesystem::path current_file= rmlFieldModel.valueContainer.Get<Rml::String>();
					std::filesystem::path current_dir= current_file.remove_filename();

					Rml::Variant titleValue;
					titleValue= "Select File";
					selectedComponent->getPropertyAttribute(
						propertyName, 
						*k_PropertyAttributeFileBrowseTitle, 
						titleValue);

					Rml::Variant filterValue;
					filterValue = "*.*";
					selectedComponent->getPropertyAttribute(
						propertyName, 
						*k_PropertyAttributeFileBrowseFilter, 
						filterValue);

					ModalDialog_FileBrowser::browseFile(
						titleValue.Get<Rml::String>(),
						current_dir,
						current_file,
						{filterValue.Get<Rml::String>()},
						[this, selectedComponent, propertyName](const std::filesystem::path& filepath) {
							Rml::Variant pathValue;
							pathValue= filepath.string();
							selectedComponent->setPropertyValue(propertyName, pathValue);
					});

				}
			}
		});
	constructor.BindEventCallback(
		"invoke_function",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int function_index = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (function_index >= 0)
			{			
				SceneComponentPtr selectedComponent = m_selectedComponentWeakPtr.lock();
				if (selectedComponent)
				{
					const RmlModel_ComponentFunction& rmlFunctionModel = m_componentFunctionModels[function_index];

					selectedComponent->invokeFunction(rmlFunctionModel.function_name);
				}
			}
		});

	// Listen for anchor config changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorSelection::anchorSystemConfigMarkedDirty);

	// Listen for selection changes
	m_editorSystemPtr->OnSelectionChanged +=
		MakeDelegate(this, &RmlModel_CompositorSelection::updateSelection);

	// Listen for stencil config changes
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorSelection::stencilSystemConfigMarkedDirty);

	// Rebuild the list of anchors
	rebuildAnchorList();

	// Rebuild the fields for the current selection, if any
	updateSelection();

	return true;
}

void RmlModel_CompositorSelection::dispose()
{
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorSelection::stencilSystemConfigMarkedDirty);

	m_editorSystemPtr->OnSelectionChanged -=
		MakeDelegate(this, &RmlModel_CompositorSelection::updateSelection);

	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorSelection::anchorSystemConfigMarkedDirty);

	RmlModel::dispose();
}

void RmlModel_CompositorSelection::anchorSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	AnchorComponentPtr selectedAnchor= 
		std::dynamic_pointer_cast<AnchorComponent>(m_selectedComponentWeakPtr.lock());

	if (changedPropertySet.hasPropertyName(AnchorObjectSystemConfig::k_anchorListPropertyId))
	{
		rebuildAnchorList();
	}

	applyConfigChangesToSelection(configPtr, changedPropertySet);
}

void RmlModel_CompositorSelection::stencilSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	applyConfigChangesToSelection(configPtr, changedPropertySet);
}

void RmlModel_CompositorSelection::applyConfigChangesToSelection(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	SceneComponentPtr selectedComponent = m_selectedComponentWeakPtr.lock();

	if (configPtr && selectedComponent && selectedComponent->getDefinition() == configPtr)
	{
		bool bUpdatedField= false;

		for (const std::string& propertyName : changedPropertySet.getSet())
		{
			Rml::Variant value;
			if (selectedComponent->getPropertyValue(propertyName, value))
			{
				RmlModel_ComponentField* rmlFieldModel = getFieldByPropertyName(propertyName);
				if (rmlFieldModel != nullptr)
				{
					rmlFieldModel->valueContainer= value;
					
					switch (rmlFieldModel->semantic_type)
					{
					case ePropertySemantic::name:
						m_modelHandle.DirtyVariable("name_fields");
						break;
					case ePropertySemantic::anchor_id:
						m_modelHandle.DirtyVariable("anchor_id_fields");
						break;
					case ePropertySemantic::position:
						m_modelHandle.DirtyVariable("position_fields");
						break;
					case ePropertySemantic::rotation:
						m_modelHandle.DirtyVariable("rotation_fields");
						break;
					case ePropertySemantic::scale:
						m_modelHandle.DirtyVariable("scale_fields");
						break;
					case ePropertySemantic::size3d:
						m_modelHandle.DirtyVariable("size3d_fields");
						break;
					case ePropertySemantic::size2d:
						m_modelHandle.DirtyVariable("size2d_fields");
						break;
					case ePropertySemantic::size1d:
						m_modelHandle.DirtyVariable("side1d_fields");
						break;
					case ePropertySemantic::checkbox:
						m_modelHandle.DirtyVariable("checkbox_fields");
						break;
					case ePropertySemantic::filename:
						m_modelHandle.DirtyVariable("filename_fields");
						break;
					}
				}
			}
		}
	}
}

RmlModel_ComponentField* RmlModel_CompositorSelection::getFieldByPropertyName(const std::string& propertyName)
{
	auto it= m_fieldNameToFieldMap.find(propertyName);
	if (it != m_fieldNameToFieldMap.end())
	{
		return it->second;
	}

	return nullptr;
}

void RmlModel_CompositorSelection::updateSelection()
{
	SceneComponentPtr oldSelection = m_selectedComponentWeakPtr.lock();
	SceneComponentPtr newSelection= oldSelection;

	SelectionComponentPtr selectionComponent= m_editorSystemPtr->getSelection();
	if (selectionComponent)
	{
		// Ignore selecting the gizmo (preserve current selection)
		MikanObjectPtr selectionObjectPtr= selectionComponent->getOwnerObject();
		if (selectionObjectPtr != m_editorSystemPtr->getGizmoObject())
		{
			newSelection = selectionComponent->getOwnerObject()->getRootComponent();
		}
	}
	else
	{
		newSelection.reset();
	}

	if (newSelection != oldSelection)
	{
		m_selectedComponentWeakPtr= newSelection;
		rebuildFieldList();
		rebuildFunctionList();
	}
}

void RmlModel_CompositorSelection::rebuildFieldList()
{
	m_componentNameFieldModels.clear();
	m_componentAnchorIdFieldModels.clear();
	m_componentPositionFieldModels.clear();
	m_componentRotationFieldModels.clear();
	m_componentScaleFieldModels.clear();
	m_componentSize3dFieldModels.clear();
	m_componentSize2dFieldModels.clear();
	m_componentSize1dFieldModels.clear();
	m_componentCheckboxFieldModels.clear();
	m_componentFilenameFieldModels.clear();

	SceneComponentPtr currentSelection = m_selectedComponentWeakPtr.lock();
	if (currentSelection)
	{
		std::vector<std::string> propertyNames;
		currentSelection->getPropertyNames(propertyNames);

		for (const std::string& propertyName : propertyNames)
		{
			PropertyDescriptor propertyDesc;
			Rml::Variant propertyValue;
			if (currentSelection->getPropertyDescriptor(propertyName, propertyDesc) &&
				currentSelection->getPropertyValue(propertyName, propertyValue))
			{
				RmlModel_ComponentField field;
				field.field_index= -1;
				field.field_type= propertyDesc.dataType;
				field.semantic_type= propertyDesc.semantic;
				field.field_name= propertyName;
				field.semantic= k_PropertySemanticNames[(int)propertyDesc.semantic];
				field.valueContainer= propertyValue;

				switch (propertyDesc.semantic)
				{
					case ePropertySemantic::checkbox:
						field.field_index= (int)m_componentCheckboxFieldModels.size();
						m_componentCheckboxFieldModels.push_back(field);
						break;
					case ePropertySemantic::position:
						field.field_index = (int)m_componentPositionFieldModels.size();
						m_componentPositionFieldModels.push_back(field);
						break;
					case ePropertySemantic::rotation:
						field.field_index = (int)m_componentRotationFieldModels.size();
						m_componentRotationFieldModels.push_back(field);
						break;
					case ePropertySemantic::scale:
						field.field_index = (int)m_componentScaleFieldModels.size();
						m_componentScaleFieldModels.push_back(field);
						break;
					case ePropertySemantic::size3d:
						field.field_index = (int)m_componentSize3dFieldModels.size();
						m_componentSize3dFieldModels.push_back(field);
						break;
					case ePropertySemantic::size2d:
						field.field_index = (int)m_componentSize2dFieldModels.size();
						m_componentSize2dFieldModels.push_back(field);
						break;
					case ePropertySemantic::size1d:
						field.field_index = (int)m_componentSize1dFieldModels.size();
						m_componentSize1dFieldModels.push_back(field);
						break;
					case ePropertySemantic::filename:
						field.field_index = (int)m_componentFilenameFieldModels.size();
						m_componentFilenameFieldModels.push_back(field);
						break;
					case ePropertySemantic::name:
						field.field_index = (int)m_componentNameFieldModels.size();
						m_componentNameFieldModels.push_back(field);
						break;
					case ePropertySemantic::anchor_id:
						field.field_index = (int)m_componentAnchorIdFieldModels.size();
						m_componentAnchorIdFieldModels.push_back(field);
						break;
					default:
						break;
				}
			}
		}
	}

	m_fieldCount = 0;
	appendFieldListToFieldMap(m_componentNameFieldModels);
	appendFieldListToFieldMap(m_componentAnchorIdFieldModels);
	appendFieldListToFieldMap(m_componentPositionFieldModels);
	appendFieldListToFieldMap(m_componentRotationFieldModels);
	appendFieldListToFieldMap(m_componentScaleFieldModels);
	appendFieldListToFieldMap(m_componentSize3dFieldModels);
	appendFieldListToFieldMap(m_componentSize2dFieldModels);
	appendFieldListToFieldMap(m_componentSize1dFieldModels);
	appendFieldListToFieldMap(m_componentCheckboxFieldModels);
	appendFieldListToFieldMap(m_componentFilenameFieldModels);

	m_modelHandle.DirtyVariable("field_count");
	if (m_selectedComponentWeakPtr.lock())
	{
		m_bIgnoreFieldsUpdate = true;
		m_modelHandle.DirtyVariable("name_fields");
		m_modelHandle.DirtyVariable("anchor_id_fields");
		m_modelHandle.DirtyVariable("position_fields");
		m_modelHandle.DirtyVariable("rotation_fields");
		m_modelHandle.DirtyVariable("scale_fields");
		m_modelHandle.DirtyVariable("size3d_fields");
		m_modelHandle.DirtyVariable("size2d_fields");
		m_modelHandle.DirtyVariable("side1d_fields");
		m_modelHandle.DirtyVariable("checkbox_fields");
		m_modelHandle.DirtyVariable("filename_fields");
		getContext()->Update();
		m_bIgnoreFieldsUpdate = false;
	}

	//if (OnComponentFieldsChanged)
	//{
	//	OnComponentFieldsChanged();
	//}
}

void RmlModel_CompositorSelection::appendFieldListToFieldMap(
	Rml::Vector<RmlModel_ComponentField>& fieldList)
{
	for (RmlModel_ComponentField& field : fieldList)
	{
		m_fieldNameToFieldMap.insert({field.field_name, &field});
		m_fieldCount++;
	}
}

void RmlModel_CompositorSelection::rebuildFunctionList()
{
	m_componentFunctionModels.clear();

	SceneComponentPtr currentSelection = m_selectedComponentWeakPtr.lock();
	if (currentSelection)
	{
		std::vector<std::string> functionNames;
		currentSelection->getFunctionNames(functionNames);

		for (const std::string& functionName : functionNames)
		{
			FunctionDescriptor functionDesc;
			Rml::Variant propertyValue;
			if (currentSelection->getFunctionDescriptor(functionName, functionDesc))
			{
				int function_index = (int)m_componentFunctionModels.size();

				m_componentFunctionModels.push_back({
					function_index,
					functionName,
					functionDesc.displayName});
			}
		}
	}

	m_modelHandle.DirtyVariable("component_functions");
}

void RmlModel_CompositorSelection::rebuildAnchorList()
{
	m_spatialAnchorIds.clear();
	auto anchorMap = m_anchorSystemPtr->getAnchorMap();
	for (auto it = anchorMap.begin(); it != anchorMap.end(); it++)
	{
		const MikanSpatialAnchorID anchorId = it->first;

		m_spatialAnchorIds.push_back(anchorId);
	}
	m_modelHandle.DirtyVariable("spatial_anchor_ids");
}