#pragma once

#include "RmlUI/Core/ElementDocument.h"
#include "RmlUI/Core/Event.h"
#include "RmlUI/Core/Variant.h"
#include "RmlUI/Core/Types.h"

#include "MikanVariantTypes.h"

namespace Rml
{
	namespace Utilities
	{
		bool IsElementDocumentVisible(ElementDocument* elementDoc)
		{
			if (elementDoc != nullptr)
			{
				const Rml::Property* property = elementDoc->GetProperty(Rml::PropertyId::Visibility);

				if (property != nullptr)
				{
					const auto Visibility = (Rml::Style::Visibility)property->Get<int>();

					return Visibility == Rml::Style::Visibility::Visible;
				}
			}

			return false;
		}

		void ToggleElementDocumentVisibility(ElementDocument* elementDoc)
		{
			if (IsElementDocumentVisible(elementDoc))
			{
				elementDoc->Hide();
			}
			else
			{
				elementDoc->Show(Rml::ModalFlag::None, Rml::FocusFlag::Document);
			}
		}

		bool GetBoolValueFromEvent(const Rml::Event& ev)
		{
			const std::string stringValue = ev.GetParameter<Rml::String>("value", "");

			return !stringValue.empty();
		}

		int GetIntValueFromEvent(const Rml::Event& ev)
		{
			return ev.GetParameter<int>("value", 0);
		}

		bool TryGetStringValueFromEvent(const Rml::Event& ev, Rml::String& outStringValue)
		{
			if (ev.GetId() == Rml::EventId::Change)
			{
				const bool isLineBreak = ev.GetParameter("linebreak", false);
				const Rml::String newStringValue = ev.GetParameter<Rml::String>("value", "");
				if (isLineBreak && !newStringValue.empty())
				{
					outStringValue= newStringValue;
					return true;
				}
			}

			return false;
		}

		void MikanVariantToRmlVariant(const MikanVariant& mkVariant, Rml::Variant& outRmlVariant)
		{
			switch (mkVariant.value_type)
			{
			case MikanVariantType::BOOL:
				outRmlVariant = mkVariant.getBoolValue();
				break;
			case MikanVariantType::INT:
				outRmlVariant = mkVariant.getIntValue();
				break;
			case MikanVariantType::FLOAT:
				outRmlVariant = mkVariant.getFloatValue();
				break;
			case MikanVariantType::DOUBLE:
				outRmlVariant = mkVariant.getDoubleValue();
				break;
			case MikanVariantType::STRING:
				outRmlVariant = mkVariant.getStringValue();
				break;
			case MikanVariantType::INT_ARRAY:
			{
				const std::vector<int>& intArray = mkVariant.getIntArrayValue();
				std::string arrayString;
				for (size_t i = 0; i < intArray.size(); ++i)
				{
					if (i > 0)
						arrayString += ",";
					arrayString += std::to_string(intArray[i]);
				}
				outRmlVariant = arrayString;
			}
			break;
			case MikanVariantType::VECTOR2F:
			{
				const MikanVector2f& vec2 = mkVariant.getVector2fValue();
				outRmlVariant = Rml::Vector2f(vec2.x, vec2.y);
			}
			break;
			case MikanVariantType::VECTOR3F:
			{
				const MikanVector3f& vec3 = mkVariant.getVector3fValue();
				outRmlVariant = Rml::Vector3f(vec3.x, vec3.y, vec3.z);
			}
			break;
			case MikanVariantType::VECTOR4F:
			{
				const MikanVector4f& vec4 = mkVariant.getVector4fValue();
				outRmlVariant = Rml::Vector4f(vec4.x, vec4.y, vec4.z, vec4.w);
			}
			break;
			case MikanVariantType::INVALID:
			default:
				outRmlVariant.Clear();
				break;
			}
		}

		MikanVariant RmlVariantToMikanVariant(const Rml::Variant& rmlVariant, MikanVariantType mikanVariantType)
		{
			MikanVariant outMkVariant;

			// Convert the Rml::Variant (usually a string representation of a value) 
			// to the desired target MikanVariant type
			switch (mikanVariantType)
			{
			case MikanVariantType::BOOL:
				outMkVariant.setValue(rmlVariant.Get<bool>());
				break;
			case MikanVariantType::INT:
				outMkVariant.setValue(rmlVariant.Get<int>());
				break;
			case MikanVariantType::FLOAT:
				outMkVariant.setValue(rmlVariant.Get<float>());
				break;
			case MikanVariantType::DOUBLE:
				outMkVariant.setValue(rmlVariant.Get<double>());
				break;
			case MikanVariantType::STRING:
				outMkVariant.setValue(rmlVariant.Get<Rml::String>());
				break;
			case MikanVariantType::VECTOR2F:
			{
				const Rml::Vector2f& vec2 = rmlVariant.Get<Rml::Vector2f>();
				outMkVariant.setValue(MikanVector2f{ vec2.x, vec2.y });
			}
			break;
			case MikanVariantType::VECTOR3F:
			{
				const Rml::Vector3f& vec3 = rmlVariant.Get<Rml::Vector3f>();
				outMkVariant.setValue(MikanVector3f{ vec3.x, vec3.y, vec3.z });
			}
			break;
			case MikanVariantType::VECTOR4F:
			{
				const Rml::Vector4f& vec4 = rmlVariant.Get<Rml::Vector4f>();
				outMkVariant.setValue(MikanVector4f{ vec4.x, vec4.y, vec4.z, vec4.w });
			}
			break;
			case MikanVariantType::INVALID:
			default:
				outMkVariant.clear();
				break;
			}

			return outMkVariant;
		}
	}
}