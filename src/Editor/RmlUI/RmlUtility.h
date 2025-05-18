#pragma once

namespace Rml
{
	class ElementDocument;
	class Event;

	namespace Utilities
	{
		bool IsElementDocumentVisible(ElementDocument* elementDoc);
		void ToggleElementDocumentVisibility(ElementDocument* element);

		bool GetBoolValueFromEvent(const Rml::Event& ev);
		int GetIntValueFromEvent(const Rml::Event& ev);
		bool TryGetStringValueFromEvent(const Rml::Event& ev, Rml::String& outStringValue);
	}
}