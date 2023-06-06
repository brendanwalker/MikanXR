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
		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_ComponentField>())
		{
			layer_model_handle.RegisterMember("field_index", &RmlModel_ComponentField::field_index);
			layer_model_handle.RegisterMember("field_name", &RmlModel_ComponentField::field_name);
			layer_model_handle.RegisterMember("semantic", &RmlModel_ComponentField::semantic);
			layer_model_handle.RegisterMember("boolean", &RmlModel_ComponentField::getBoolean, &RmlModel_ComponentField::setBoolean);
			layer_model_handle.RegisterMember("int", &RmlModel_ComponentField::getInt, &RmlModel_ComponentField::setInt);
			layer_model_handle.RegisterMember("float", &RmlModel_ComponentField::getFloat, &RmlModel_ComponentField::setFloat);
			layer_model_handle.RegisterMember("string", &RmlModel_ComponentField::getString, &RmlModel_ComponentField::setString);

			layer_model_handle.RegisterMember("vec2_x", &RmlModel_ComponentField::getVec2X, &RmlModel_ComponentField::setVector2X);
			layer_model_handle.RegisterMember("vec2_y", &RmlModel_ComponentField::getVec2Y, &RmlModel_ComponentField::setVector2Y);

			layer_model_handle.RegisterMember("vec3_x", &RmlModel_ComponentField::getVec3X, &RmlModel_ComponentField::setVector3X);
			layer_model_handle.RegisterMember("vec3_y", &RmlModel_ComponentField::getVec3Y, &RmlModel_ComponentField::setVector3Y);
			layer_model_handle.RegisterMember("vec3_z", &RmlModel_ComponentField::getVec3Z, &RmlModel_ComponentField::setVector3Z);

			layer_model_handle.RegisterMember("vec4_x", &RmlModel_ComponentField::getVec4X, &RmlModel_ComponentField::setVector4X);
			layer_model_handle.RegisterMember("vec4_y", &RmlModel_ComponentField::getVec4Y, &RmlModel_ComponentField::setVector4Y);
			layer_model_handle.RegisterMember("vec4_z", &RmlModel_ComponentField::getVec4Z, &RmlModel_ComponentField::setVector4Z);
			layer_model_handle.RegisterMember("vec4_w", &RmlModel_ComponentField::getVec4W, &RmlModel_ComponentField::setVector4W);
		}

		// One time registration for an array of component fields.
		constructor.RegisterArray<decltype(m_componentFieldModels)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("component_fields", &m_componentFieldModels);
	constructor.Bind("spatial_anchor_ids", &m_spatialAnchorIds);	

	// Bind data model callbacks
	constructor.BindEventCallback(
		"modify_field",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int field_index = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (field_index >= 0)
			{
				const RmlModel_ComponentField& rmlFieldModel= m_componentFieldModels[field_index];
				SceneComponentPtr selectedComponent = m_selectedComponentWeakPtr.lock();
				if (selectedComponent)
				{
					selectedComponent->setPropertyValue(rmlFieldModel.field_name, rmlFieldModel.valueContainer);
				}
			}
		});
	constructor.BindEventCallback(
		"select_filepath",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int field_index = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (field_index >= 0)
			{
				const RmlModel_ComponentField& rmlFieldModel = m_componentFieldModels[field_index];
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
					bUpdatedField= true;
				}
			}
		}

		if (bUpdatedField)
		{
			m_modelHandle.DirtyVariable("component_fields");
		}
	}
}

RmlModel_ComponentField* RmlModel_CompositorSelection::getFieldByPropertyName(const std::string& propertyName)
{
	auto it= m_fieldNameToIndexMap.find(propertyName);
	if (it != m_fieldNameToIndexMap.end())
	{
		const int fieldIndex = it->second;

		return &m_componentFieldModels[fieldIndex];
	}

	return nullptr;
}

RmlModel_ComponentField* RmlModel_CompositorSelection::getFieldBySemantic(const std::string& semantic)
{
	auto it = std::find_if(
		m_componentFieldModels.begin(), m_componentFieldModels.end(), 
		[&semantic](const RmlModel_ComponentField& field) {
			return field.semantic == semantic;
		});

	if (it != m_componentFieldModels.end())
	{
		RmlModel_ComponentField& field = *it;

		return &field;
	}

	return nullptr;
}

void RmlModel_CompositorSelection::updateSelection()
{
	SceneComponentPtr oldSelection = m_selectedComponentWeakPtr.lock();

	SceneComponentPtr newSelection;
	SelectionComponentPtr selectionComponent= m_editorSystemPtr->getSelection();
	if (selectionComponent)
	{
		newSelection= selectionComponent->getOwnerObject()->getRootComponent();
	}

	if (newSelection != oldSelection)
	{
		m_selectedComponentWeakPtr= newSelection;;
		rebuildFieldList();
	}
}

void RmlModel_CompositorSelection::rebuildFieldList()
{
	m_fieldNameToIndexMap.clear();
	m_componentFieldModels.clear();

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
				int field_index= (int)m_componentFieldModels.size();
				ePropertyDataType field_type= propertyDesc.dataType;
				Rml::String field_name= propertyName;
				Rml::String semantic= k_PropertySemanticNames[(int)propertyDesc.semantic];

				m_componentFieldModels.push_back({field_index, field_type, field_name, semantic, propertyValue});
				m_fieldNameToIndexMap.insert({field_name, field_index});
			}
		}
	}

	m_modelHandle.DirtyVariable("component_fields");
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