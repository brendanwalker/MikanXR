#pragma once

namespace Rml
{
	class ElementDocument;
	class Event;
	class Variant;

	namespace Utilities
	{
		bool IsElementDocumentVisible(ElementDocument* elementDoc);
		void ToggleElementDocumentVisibility(ElementDocument* element);

		bool GetBoolValueFromEvent(const Rml::Event& ev);
		int GetIntValueFromEvent(const Rml::Event& ev);
		bool TryGetStringValueFromEvent(const Rml::Event& ev, Rml::String& outStringValue);

		void MikanVariantToRmlVariant(const struct MikanVariant& mkVariant, Rml::Variant& outRmlVariant);
		MikanVariant RmlVariantToMikanVariant(const Rml::Variant& rmlVariant, MikanVariantType mikanVariantType);
	}
}