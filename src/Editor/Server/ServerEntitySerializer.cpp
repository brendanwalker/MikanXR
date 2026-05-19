#include "ServerEntitySerializer.h"
#include "SerializableList.h"
#include "SerializableObjectPtr.h"
#include "SerializableString.h"
#include "SerializationProperty.h"
#include "StringUtils.h"
#include "Logger.h"

#include "Refureku/Refureku.h"

namespace Serialization
{
	class EntityAccessorReadVisitor : public IVisitor
	{
	public:
		EntityAccessorReadVisitor(IEntityAccessorConstPtr entityAccessor) : m_entityAccessor(entityAccessor) {}

		virtual void visitClass(ValueAccessor const& accessor) override
		{
			rfk::Type const& fieldType = accessor.getType();
			rfk::Class const* fieldClassType = accessor.getClassType();
			rfk::EClassKind classKind = fieldClassType->getClassKind();

			if (fieldType == rfk::getType<Serialization::BoolList>())
			{
				visitBoolList(accessor);
			}
			else if (classKind == rfk::EClassKind::TemplateInstantiation)
			{
				void* arrayInstance = accessor.getUntypedValueMutablePtr();
				const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(fieldClassType);
				std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

				// See if the field is a Serialization::List<T>
				if (templateTypeName == "List" &&
					templateClassInstanceType->getTemplateArgumentsCount() == 1)
				{
					// Get the type of the elements in the array from the template argument
					auto const& templateArg =
						static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(0));
					rfk::Type const& elementType = templateArg.getType();

					if (elementType == rfk::getType<uint8_t>())
					{
						visitUByteList(accessor);
					}
					else if (elementType == rfk::getType<int>())
					{
						visitIntList(accessor);
					}
					else if (elementType == rfk::getType<float>())
					{
						visitFloatList(accessor);
					}
					else if (elementType == rfk::getType<Serialization::String>())
					{
						visitStringList(accessor);
					}
					else
					{
						throw std::runtime_error(
							"EntityAccessorReadVisitor::visitClass() List with unsupported element type");
					}
				}
				// See if the field is a Serialization::Map<K,V>
				else if (templateTypeName == "Map" &&
					templateClassInstanceType->getTemplateArgumentsCount() == 2)
				{
					visitMap(accessor, *templateClassInstanceType);
				}
				else
				{
					throw std::runtime_error(
						StringUtils::stringify("EntityAccessorReadVisitor::visitClass() ",
							"Template Class Type ", templateTypeName, " is not supported"));
				}
			}
			else if (fieldType == rfk::getType<PolymorphicObjectPtr>())
			{
				visitObjectPtr(accessor);
			}
			else if (fieldType == rfk::getType<Serialization::String>())
			{
				visitString(accessor);
			}
			else
			{
				EntityAccessorReadVisitor::visitStruct(accessor);
			}
		}

		void visitObjectPtr(ValueAccessor const& accessor)
		{
			// Get the shared pointer we are writing 
			PolymorphicObjectPtr& objPtrInstance = accessor.getTypedValueMutableRef<PolymorphicObjectPtr>();

			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::POLYMORPHIC_OBJECT)
			{
				// Directly use the PolymorphicObjectPtr from the source property value
				objPtrInstance = sourcePropertyValue.getPolymorphicObjectValue();
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitObjectPtr() ",
						"Missing valid PolymorphicObjectPtr for", accessor.getName()));
			}
		}

		void visitBoolList(ValueAccessor const& arrayAccessor)
		{
			auto& boolListWrapper = arrayAccessor.getTypedValueMutableRef<Serialization::BoolList>();
			auto& boolList = boolListWrapper.getVectorMutable();

			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(arrayAccessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::BOOL_ARRAY)
			{
				boolList = sourcePropertyValue.getBoolArrayValue();
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitBoolList() ",
						"Missing valid BoolList for", arrayAccessor.getName()));
			}
		}

		void visitUByteList(ValueAccessor const& arrayAccessor)
		{
			auto* destArray =
				reinterpret_cast<Serialization::List<uint8_t> *>(
					arrayAccessor.getUntypedValueMutablePtr());

			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(arrayAccessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::UBYTE_ARRAY)
			{
				const std::vector<uint8_t>& sourceArray = sourcePropertyValue.getUByteArrayValue();

				destArray->assign(sourceArray.begin(), sourceArray.end());
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitUByteList() ",
						"Missing valid UByteList for", arrayAccessor.getName()));
			}
		}

		void visitIntList(ValueAccessor const& arrayAccessor)
		{
			auto* destArray = 
				reinterpret_cast<Serialization::List<int> *>(
					arrayAccessor.getUntypedValueMutablePtr());

			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(arrayAccessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT_ARRAY)
			{
				const std::vector<int>& sourceArray = sourcePropertyValue.getIntArrayValue();

				destArray->assign(sourceArray.begin(), sourceArray.end());
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitIntList() ",
						"Missing valid IntList for", arrayAccessor.getName()));
			}
		}

		void visitFloatList(ValueAccessor const& arrayAccessor)
		{
			auto* destArray =
				reinterpret_cast<Serialization::List<float> *>(
					arrayAccessor.getUntypedValueMutablePtr());

			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(arrayAccessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::FLOAT_ARRAY)
			{
				const std::vector<float>& sourceArray = sourcePropertyValue.getFloatArrayValue();

				destArray->assign(sourceArray.begin(), sourceArray.end());
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitFloatList() ",
						"Missing valid FloatList for", arrayAccessor.getName()));
			}
		}

		void visitStringList(ValueAccessor const& arrayAccessor)
		{
			auto* destArray =
				reinterpret_cast<Serialization::List<Serialization::String> *>(
					arrayAccessor.getUntypedValueMutablePtr());

			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(arrayAccessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::STRING_ARRAY)
			{
				const std::vector<Serialization::String>& sourceArray = sourcePropertyValue.getStringArrayValue();

				destArray->assign(sourceArray.begin(), sourceArray.end());
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitStringList() ",
						"Missing valid StringList for", arrayAccessor.getName()));
			}
		}

		void visitMap(
			ValueAccessor const& mapAccessor,
			rfk::ClassTemplateInstantiation const& templatedMapType)
		{
			// Get the key type of the map from the template argument
			auto const& templateKeyArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedMapType.getTemplateArgumentAt(0));
			auto const& templateValueArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedMapType.getTemplateArgumentAt(1));
			rfk::Type const& keyType = templateKeyArg.getType();
			rfk::Type const& valueType = templateValueArg.getType();

			if (keyType == rfk::getType<std::string>() &&
				valueType == rfk::getType<Serialization::String>())
			{
				visitStringMap(mapAccessor, templatedMapType);
			}
			else
			{
				rfk::Archetype const* keyArchetype = keyType.getArchetype();
				rfk::Archetype const* valueArchetype = valueType.getArchetype();

				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitMap() ",
						"Map Key Archetype ", keyArchetype != nullptr ? keyArchetype->getName() : "<Null Archetype>",
						" Value Archetype ", valueArchetype != nullptr ? valueArchetype->getName() : "<Null Archetype>",
						" is not supported"));
			}
		}

		void visitStringMap(ValueAccessor const& mapAccessor,
			rfk::ClassTemplateInstantiation const& templatedMapType)
		{
			// Get an enumerator to the target string map
			auto* mapInstance =
				reinterpret_cast<Serialization::Map<std::string, Serialization::String> *>(
					mapAccessor.getUntypedValueMutablePtr());

			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(mapAccessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::STRING_MAP)
			{
				const Serialization::Map<std::string, Serialization::String>& sourceMap =
					sourcePropertyValue.getStringMapValue();

				mapInstance->clear();
				mapInstance->insert(sourceMap.begin(), sourceMap.end());
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitStringMap() ",
						"Missing valid StringMap for", mapAccessor.getName()));
			}
		}

		void visitString(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::STRING)
			{
				auto* variablePtr = accessor.getTypedValueMutablePtr<Serialization::String>();

				variablePtr->setValue(sourcePropertyValue.getStringValue());
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitString() ",
						"Missing valid String for", accessor.getName()));
			}
		}

		virtual void visitStruct(ValueAccessor const& accessor) override
		{
			rfk::Type const& fieldType = accessor.getType();

			if (fieldType == rfk::getType<MikanVector2f>())
			{
				visitVector2f(accessor);
			}
			else if (fieldType == rfk::getType<MikanVector3f>())
			{
				visitVector3f(accessor);
			}
			else if (fieldType == rfk::getType<MikanVector4f>())
			{
				visitVector4f(accessor);
			}
			else if (fieldType == rfk::getType<MikanQuatf>())
			{
				visitQuaternionf(accessor);
			}
			else if (fieldType == rfk::getType<MikanMatrix4f>())
			{
				visitMatrix4f(accessor);
			}
			else if (fieldType == rfk::getType<MikanVector2d>())
			{
				visitVector2d(accessor);
			}
			else if (fieldType == rfk::getType<MikanVector3d>())
			{
				visitVector3d(accessor);
			}
			else if (fieldType == rfk::getType<MikanVector4d>())
			{
				visitVector4d(accessor);
			}
			else if (fieldType == rfk::getType<MikanQuatd>())
			{
				visitQuaterniond(accessor);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitStruct() ",
						"Struct Accessor ", accessor.getName(),
						" was not a recognized struct type"));
			}
		}

		void visitVector2f(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::VECTOR2F)
			{
				MikanVector2f value = sourcePropertyValue.getVector2fValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitVector2f() ",
						"Vector2f Accessor ", accessor.getName(),
						" was not a Vector2f value"));
			}
		}

		void visitVector3f(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::VECTOR3F)
			{
				MikanVector3f value = sourcePropertyValue.getVector3fValue();
				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitVector3f() ",
						"Vector3f Accessor ", accessor.getName(),
						" was not a Vector3f value"));
			}
		}

		void visitVector4f(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::VECTOR4F)
			{
				MikanVector4f value = sourcePropertyValue.getVector4fValue();
				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitVector4f() ",
						"Vector4f Accessor ", accessor.getName(),
						" was not a Vector4f value"));
			}
		}

		void visitQuaternionf(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::QUATERNIONF)
			{
				MikanQuatf value = sourcePropertyValue.getQuaternionfValue();
				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitQuaternionf() ",
						"Quaternionf Accessor ", accessor.getName(),
						" was not a Quaternionf value"));
			}
		}

		void visitMatrix4f(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::MATRIX4F)
			{
				MikanMatrix4f value = sourcePropertyValue.getMatrix4fValue();
				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitMatrix4f() ",
						"Matrix4f Accessor ", accessor.getName(),
						" was not a Matrix4f value"));
			}
		}

		void visitVector2d(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::VECTOR2D)
			{
				MikanVector2d value = sourcePropertyValue.getVector2dValue();
				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitVector2d() ",
						"Vector2d Accessor ", accessor.getName(),
						" was not a Vector2d value"));
			}
		}

		void visitVector3d(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::VECTOR3D)
			{
				MikanVector3d value = sourcePropertyValue.getVector3dValue();
				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitVector3d() ",
						"Vector3d Accessor ", accessor.getName(),
						" was not a Vector3d value"));
			}
		}

		void visitVector4d(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::VECTOR4D)
			{
				MikanVector4d value = sourcePropertyValue.getVector4dValue();
				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitVector4d() ",
						"Vector4d Accessor ", accessor.getName(),
						" was not a Vector4d value"));
			}
		}

		void visitQuaterniond(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::QUATERNIOND)
			{
				MikanQuatd value = sourcePropertyValue.getQuaterniondValue();
				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitQuaterniond() ",
						"Quaterniond Accessor ", accessor.getName(),
						" was not a Quaterniond value"));
			}
		}

		virtual void visitEnum(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				const int sourceEnumIntValue = sourcePropertyValue.getIntValue();

				rfk::Enum const& enumType = *accessor.getEnumType();
				rfk::Archetype const& enumArchetype = enumType.getUnderlyingArchetype();
				rfk::EnumValue const* enumValue = enumType.getEnumValue(sourceEnumIntValue);

				if (enumValue != nullptr)
				{
					void* enumInstance = accessor.getInstanceMutable();
					rfk::Field const* enumField = accessor.getField();
					const int64_t enumInt64Value = enumValue->getValue();

					if (enumField != nullptr)
					{
						enumField->setUnsafe(enumInstance, &enumInt64Value, enumArchetype.getMemorySize());
					}
					else
					{
						std::memcpy(enumInstance, &enumInt64Value, enumArchetype.getMemorySize());
					}
				}
				else
				{
					throw std::runtime_error(
						StringUtils::stringify("EntityAccessorReadVisitor::visitEnum() ",
							"Enum Accessor ", accessor.getName(),
							" has an invalid value ", sourceEnumIntValue));
				}
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitEnum() ",
						"Missing valid Enum for", accessor.getName()));
			}
		}

		virtual void visitBool(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::BOOL)
			{
				bool value = sourcePropertyValue.getBoolValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitBool() ",
						"Bool Accessor ", accessor.getName(),
						" was not a bool value"));
			}
		}

		virtual void visitByte(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				int8_t value = (int8_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitByte() ",
						"Byte Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitUByte(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				uint8_t value = (uint8_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitUByte() ",
						"UByte Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitShort(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				int16_t value = (int16_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitShort() ",
						"Short Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitUShort(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				uint16_t value = (uint16_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitUShort() ",
						"UShort Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitInt(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				int32_t value = (int32_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitInt() ",
						"Int32 Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitUInt(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				uint32_t value = (uint32_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitUInt() ",
						"UInt32 Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitLong(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::LONG)
			{
				int64_t value = (int64_t)sourcePropertyValue.getLongValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitLong() ",
						"Int64 Accessor ", accessor.getName(),
						" was not a long value"));
			}
		}

		virtual void visitULong(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::LONG)
			{
				uint64_t value = (uint64_t)sourcePropertyValue.getLongValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitULong() ",
						"UInt64 Accessor ", accessor.getName(),
						" was not a long value"));
			}
		}

		virtual void visitFloat(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::FLOAT)
			{
				float value = sourcePropertyValue.getFloatValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitFloat() ",
						"Float Accessor ", accessor.getName(),
						" was not a float value"));
			}
		}

		virtual void visitDouble(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::DOUBLE)
			{
				double value = sourcePropertyValue.getDoubleValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitDouble() ",
						"Double Accessor ", accessor.getName(),
						" was not a double value"));
			}
		}

	private:
		IEntityAccessorConstPtr m_entityAccessor;
	};

	// Public API
	bool serializeFromEntity(IEntityAccessorConstPtr entityAccessor, void* instance, rfk::Struct const& structType)
	{
		try
		{
			EntityAccessorReadVisitor visitor(entityAccessor);
			Serialization::visitStruct(instance, structType, &visitor);

			return true;
		}
		catch (std::runtime_error* e)
		{
			MIKAN_LOG_ERROR("Serialization::serializeFromEntity") << e->what();
			return false;
		}
	}
};