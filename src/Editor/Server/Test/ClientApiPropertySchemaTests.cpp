#include "ClientApiPropertySchemaTests.h"
#include "unit_test.h"

// -- Property interface / serialization plumbing --
#include "PropertyInterface.h"
#include "SerializationVisitor.h"
#include "SerializableList.h"
#include "SerializableMap.h"
#include "SerializableString.h"
#include "SerializableObjectPtr.h"
#include "MikanVariantTypes.h"
#include "MikanMathTypes.h"
#include "StringUtils.h"

// -- Client API values struct definitions --
#include "MikanAnchorTypes.h"
#include "MikanCameraTypes.h"
#include "MikanCompositorTypes.h"
#include "MikanEditorTypes.h"
#include "MikanLightTypes.h"
#include "MikanMarkerTypes.h"
#include "MikanSceneTypes.h"
#include "MikanShapeTypes.h"
#include "MikanStageTypes.h"
#include "MikanStencilTypes.h"
#include "MikanTextureSourceTypes.h"
#include "MikanTrackingMountTypes.h"
#include "MikanTrackingVolumeTypes.h"
#include "MikanTransformTypes.h"
#include "MikanVideoSourceTypes.h"
#include "MikanVRDeviceTypes.h"

// -- Component / object-system classes that expose client API values structs --
#include "Anchor/AnchorComponent.h"
#include "Camera/CameraComponent.h"
#include "Compositor/CompositorComponent.h"
#include "Editor/EditorObjectSystem.h"
#include "Light/DMXFixtureComponent.h"
#include "Light/DMXObjectSystem.h"
#include "Light/RGBPixelGridComponent.h"
#include "Light/RGBSpotLightComponent.h"
#include "Marker/MarkerComponent.h"
#include "Marker/MarkerObjectSystem.h"
#include "Scene/SceneComponent.h"
#include "Scene/TransformComponent.h"
#include "Shape/BoxShapeComponent.h"
#include "Shape/BoxShapeSystem.h"
#include "Shape/ModelShapeComponent.h"
#include "Shape/ModelShapeSystem.h"
#include "Shape/QuadShapeComponent.h"
#include "Shape/QuadShapeSystem.h"
#include "Stage/StageComponent.h"
#include "Stencil/BoxStencilComponent.h"
#include "Stencil/ModelStencilComponent.h"
#include "Stencil/QuadStencilComponent.h"
#include "Stencil/StencilComponent.h"
#include "TextureSource/CEFTextureSourceComponent.h"
#include "TextureSource/ClientTextureSourceComponent.h"
#include "TextureSource/SpoutTextureSourceComponent.h"
#include "TextureSource/TextureSourceComponent.h"
#include "TrackingMount/TrackingMountComponent.h"
#include "TrackingVolume/MarkerTrackingVolumeComponent.h"
#include "TrackingVolume/TrackingVolumeComponent.h"
#include "TrackingVolume/VRTrackingVolumeComponent.h"
#include "VideoSource/NetworkVideoSourceComponent.h"
#include "VideoSource/USBVideoSourceComponent.h"
#include "VideoSource/USBVideoSourceSystem.h"
#include "VideoSource/VideoSourceComponent.h"
#include "VRObject/VRDeviceComponent.h"
#include "VRObject/VRObjectSystem.h"

#include "Refureku/Refureku.h"

#include <map>
#include <string>
#include <vector>

namespace
{
// Walks a client API values struct and records, per field, the MikanVariantType
// that ServerEntitySerializer's read visitor will demand for that field. The
// per-type mapping here MUST mirror EntityAccessorReadVisitor in
// ServerEntitySerializer.cpp (this is the single source of truth for the
// "what variant type does the server expect for this field" contract).
class PropertySchemaVisitor : public Serialization::IVisitor
{
public:
	std::map<std::string, MikanVariantType> fieldTypes;

	void record(Serialization::ValueAccessor const& accessor, MikanVariantType type)
	{
		fieldTypes[accessor.getName()]= type;
	}

	// -- Fundamental scalars --
	void visitBool(Serialization::ValueAccessor const& a) override { record(a, MikanVariantType::BOOL); }
	void visitByte(Serialization::ValueAccessor const& a) override
	{
		record(a, MikanVariantType::UBYTE);
	} // int8_t -> UBYTE (see visitByte in serializer)
	void visitUByte(Serialization::ValueAccessor const& a) override { record(a, MikanVariantType::UBYTE); }
	void visitShort(Serialization::ValueAccessor const& a) override
	{
		record(a, MikanVariantType::USHORT);
	} // int16_t -> USHORT
	void visitUShort(Serialization::ValueAccessor const& a) override { record(a, MikanVariantType::USHORT); }
	void visitInt(Serialization::ValueAccessor const& a) override { record(a, MikanVariantType::INT); }
	void visitUInt(Serialization::ValueAccessor const& a) override
	{
		record(a, MikanVariantType::INT);
	} // uint32_t -> INT
	void visitLong(Serialization::ValueAccessor const& a) override { record(a, MikanVariantType::LONG); }
	void visitULong(Serialization::ValueAccessor const& a) override
	{
		record(a, MikanVariantType::LONG);
	} // uint64_t -> LONG
	void visitFloat(Serialization::ValueAccessor const& a) override { record(a, MikanVariantType::FLOAT); }
	void visitDouble(Serialization::ValueAccessor const& a) override { record(a, MikanVariantType::DOUBLE); }
	void visitEnum(Serialization::ValueAccessor const& a) override { record(a, MikanVariantType::INT); }

	// -- Math structs --
	void visitStruct(Serialization::ValueAccessor const& a) override
	{
		rfk::Type const& t= a.getType();

		if (t == rfk::getType<MikanVector2f>())
			record(a, MikanVariantType::VECTOR2F);
		else if (t == rfk::getType<MikanVector3f>())
			record(a, MikanVariantType::VECTOR3F);
		else if (t == rfk::getType<MikanVector4f>())
			record(a, MikanVariantType::VECTOR4F);
		else if (t == rfk::getType<MikanQuatf>())
			record(a, MikanVariantType::QUATERNIONF);
		else if (t == rfk::getType<MikanMatrix4f>())
			record(a, MikanVariantType::MATRIX4F);
		else if (t == rfk::getType<MikanVector2d>())
			record(a, MikanVariantType::VECTOR2D);
		else if (t == rfk::getType<MikanVector3d>())
			record(a, MikanVariantType::VECTOR3D);
		else if (t == rfk::getType<MikanVector4d>())
			record(a, MikanVariantType::VECTOR4D);
		else if (t == rfk::getType<MikanQuatd>())
			record(a, MikanVariantType::QUATERNIOND);
		else
			setError(StringUtils::stringify("Field '", a.getName(), "' has unsupported struct type"));
	}

	// -- Strings, lists, maps, polymorphic object ptrs --
	void visitClass(Serialization::ValueAccessor const& a) override
	{
		rfk::Type const& fieldType= a.getType();
		rfk::Class const* fieldClassType= a.getClassType();

		if (fieldClassType != nullptr && fieldClassType->getClassKind() == rfk::EClassKind::TemplateInstantiation)
		{
			const auto* templateInst= rfk::classTemplateInstantiationCast(fieldClassType);
			const std::string templateName= templateInst->getClassTemplate().getName();

			if (templateName == "List" && templateInst->getTemplateArgumentsCount() == 1)
			{
				auto const& templateArg=
					static_cast<rfk::TypeTemplateArgument const&>(templateInst->getTemplateArgumentAt(0));
				rfk::Type const& elementType= templateArg.getType();

				if (elementType == rfk::getType<bool>())
					record(a, MikanVariantType::BOOL_ARRAY);
				else if (elementType == rfk::getType<uint8_t>())
					record(a, MikanVariantType::UBYTE_ARRAY);
				else if (elementType == rfk::getType<int>())
					record(a, MikanVariantType::INT_ARRAY);
				else if (elementType == rfk::getType<float>())
					record(a, MikanVariantType::FLOAT_ARRAY);
				else if (elementType == rfk::getType<Serialization::String>())
					record(a, MikanVariantType::STRING_ARRAY);
				else
					setError(
						StringUtils::stringify("Field '", a.getName(), "' is a List with an unsupported element type"));
			}
			else if (templateName == "Map" && templateInst->getTemplateArgumentsCount() == 2)
			{
				// The serializer only supports Map<std::string, Serialization::String>.
				record(a, MikanVariantType::STRING_MAP);
			}
			else
			{
				setError(StringUtils::stringify("Field '", a.getName(), "' is an unsupported template class '",
												templateName, "'"));
			}
		}
		else if (fieldType == rfk::getType<Serialization::PolymorphicObjectPtr>())
		{
			record(a, MikanVariantType::POLYMORPHIC_OBJECT);
		}
		else if (fieldType == rfk::getType<Serialization::String>())
		{
			record(a, MikanVariantType::STRING);
		}
		else
		{
			// Fall back to the struct handler (mirrors ServerEntitySerializer).
			PropertySchemaVisitor::visitStruct(a);
		}
	}
};

using GetDescriptorsFn= void (*)(std::vector<PropertyDescriptorConstPtr>&);

struct SchemaTestEntry
{
	const char* label;
	rfk::Struct const* valuesStruct;
	GetDescriptorsFn getDescriptors;
};

#define SCHEMA_ENTRY(EditorClass, ValuesStruct)                                                                        \
	{#EditorClass, &ValuesStruct::staticGetArchetype(), &EditorClass::getPropertyDescriptors}

const SchemaTestEntry k_schemaTestEntries[]= {
	// -- Components --
	SCHEMA_ENTRY(AnchorComponent, MikanAnchorComponentValues),
	SCHEMA_ENTRY(CameraComponent, MikanCameraComponentValues),
	SCHEMA_ENTRY(CompositorComponent, MikanCompositorComponentValues),
	// DMXFixtureComponent is an abstract base (getPropertyDescriptors is protected and
	// it is never serialized directly); its fields are covered by the concrete
	// RGBSpotLightComponent / RGBPixelGridComponent entries below.
	SCHEMA_ENTRY(RGBPixelGridComponent, MikanRGBPixelGridComponentValues),
	SCHEMA_ENTRY(RGBSpotLightComponent, MikanRGBSpotLightComponentValues),
	SCHEMA_ENTRY(MarkerComponent, MikanMarkerComponentValues),
	SCHEMA_ENTRY(SceneComponent, MikanSceneComponentValues),
	SCHEMA_ENTRY(TransformComponent, MikanTransformComponentValues),
	SCHEMA_ENTRY(BoxShapeComponent, MikanBoxShapeComponentValues),
	SCHEMA_ENTRY(ModelShapeComponent, MikanModelShapeComponentValues),
	SCHEMA_ENTRY(QuadShapeComponent, MikanQuadShapeComponentValues),
	SCHEMA_ENTRY(StageComponent, MikanStageComponentValues),
	SCHEMA_ENTRY(BoxStencilComponent, MikanBoxStencilComponentValues),
	SCHEMA_ENTRY(ModelStencilComponent, MikanModelStencilComponentValues),
	SCHEMA_ENTRY(QuadStencilComponent, MikanQuadStencilComponentValues),
	SCHEMA_ENTRY(StencilComponent, MikanStencilComponentValues),
	SCHEMA_ENTRY(CEFTextureSourceComponent, MikanCEFTextureSourceValues),
	SCHEMA_ENTRY(ClientTextureSourceComponent, MikanClientTextureSourceValues),
	SCHEMA_ENTRY(SpoutTextureSourceComponent, MikanSpoutTextureSourceValues),
	SCHEMA_ENTRY(TextureSourceComponent, MikanTextureSourceValues),
	SCHEMA_ENTRY(TrackingMountComponent, MikanTrackingMountComponentValues),
	SCHEMA_ENTRY(MarkerTrackingVolumeComponent, MikanMarkerTrackingVolumeComponentValues),
	SCHEMA_ENTRY(TrackingVolumeComponent, MikanTrackingVolumeComponentValues),
	SCHEMA_ENTRY(VRTrackingVolumeComponent, MikanVRTrackingVolumeComponentValues),
	SCHEMA_ENTRY(NetworkVideoSourceComponent, MikanNetworkVideoSourceValues),
	SCHEMA_ENTRY(USBVideoSourceComponent, MikanUSBVideoSourceValues),
	SCHEMA_ENTRY(VideoSourceComponent, MikanVideoSourceValues),
	SCHEMA_ENTRY(VRDeviceComponent, MikanVRDeviceComponentValues),

	// -- Object systems --
	SCHEMA_ENTRY(EditorObjectSystem, MikanEditorSystemValues),
	SCHEMA_ENTRY(DMXObjectSystem, MikanDMXObjectSystemValues),
	SCHEMA_ENTRY(MarkerObjectSystem, MikanMarkerSystemValues),
	SCHEMA_ENTRY(BoxShapeSystem, MikanBoxShapeSystemValues),
	SCHEMA_ENTRY(ModelShapeSystem, MikanModelShapeSystemValues),
	SCHEMA_ENTRY(QuadShapeSystem, MikanQuadShapeSystemValues),
	SCHEMA_ENTRY(USBVideoSourceSystem, MikanUSBVideoSourceSystemValues),
	SCHEMA_ENTRY(VRObjectSystem, MikanVRObjectSystemValues),
};

#undef SCHEMA_ENTRY

// Validate a single entity's values struct against its property descriptors.
bool testSchemaEntry(const SchemaTestEntry& entry)
{
	bool success= true;

	// 1. Compute the variant type the serializer requires for each struct field.
	Serialization::PolymorphicObjectPtr instanceHolder;
	void* instance= instanceHolder.allocateByType(entry.valuesStruct);
	if (instance == nullptr)
	{
		MIKAN_LOG_ERROR("testSchemaEntry") << "[" << entry.label << "]" << " FAILED to allocate values struct instance";
		return false;
	}

	PropertySchemaVisitor schemaVisitor;
	Serialization::visitStruct(instance, *entry.valuesStruct, &schemaVisitor);
	if (schemaVisitor.hasError())
	{
		MIKAN_LOG_ERROR("testSchemaEntry")
			<< "[" << entry.label << "]" << "values struct has an unsupported field: " << schemaVisitor.getError();
		success= false;
	}

	// 2. Gather the property descriptors advertised for this class.
	std::vector<PropertyDescriptorConstPtr> descriptors;
	entry.getDescriptors(descriptors);

	std::map<std::string, MikanVariantType> descriptorTypesByName;
	for (const PropertyDescriptorConstPtr& descriptor : descriptors)
	{
		if (!descriptor->isClientAPIHidden())
		{
			descriptorTypesByName[descriptor->getName()]= descriptor->getDataType();
		}
	}

	// 3. Every values-struct field must have a descriptor of the exact required type.
	for (const auto& fieldEntry : schemaVisitor.fieldTypes)
	{
		const std::string& fieldName= fieldEntry.first;
		const MikanVariantType requiredType= fieldEntry.second;

		auto descriptorIt= descriptorTypesByName.find(fieldName);
		if (descriptorIt == descriptorTypesByName.end())
		{
			MIKAN_LOG_ERROR("testSchemaEntry") << "[" << entry.label << "]" << "values struct field '" << fieldName
											   << "' " << "(" << mikanVariantTypeToString(requiredType) << ") "
											   << "has no matching property descriptor (unfetchable / name mismatch)";
			success= false;
		}
		else if (descriptorIt->second != requiredType)
		{
			MIKAN_LOG_ERROR("testSchemaEntry")
				<< "[" << entry.label << "]" << "field '" << fieldName << "'"
				<< " type mismatch: descriptor=" << mikanVariantTypeToString(descriptorIt->second) << ", "
				<< "values struct requires=" << mikanVariantTypeToString(requiredType);
			success= false;
		}
	}

	// 4. Every descriptors not marked as ClientAPIHidde must have a matching values struct field
	for (const auto& descriptorEntry : descriptorTypesByName)
	{
		if (schemaVisitor.fieldTypes.find(descriptorEntry.first) == schemaVisitor.fieldTypes.end())
		{
			MIKAN_LOG_ERROR("testSchemaEntry")
				<< "[" << entry.label << "]" << "descriptor '" << descriptorEntry.first << "' " << "("
				<< mikanVariantTypeToString(descriptorEntry.second) << ") " << "has no field in the values struct";
			success= false;
		}
	}

	return success;
}
} // namespace

// Free function (external linkage) so the UNIT_TEST_MODULE_CALL_TEST forward
// declaration below resolves to this definition.
bool client_api_test_values_struct_matches_descriptors()
{
	UNIT_TEST_BEGIN("client API values structs match property descriptors")

	for (const SchemaTestEntry& entry : k_schemaTestEntries)
	{
		success&= testSchemaEntry(entry);
	}

	UNIT_TEST_COMPLETE()
}

bool run_client_api_property_schema_tests()
{
	UNIT_TEST_MODULE_BEGIN("client_api_property_schema")
	UNIT_TEST_MODULE_CALL_TEST(client_api_test_values_struct_matches_descriptors);
	UNIT_TEST_MODULE_END()
}
