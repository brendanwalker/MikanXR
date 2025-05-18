#pragma once

#include "RmlUI/Core/ElementDocument.h"
#include "RmlUI/Core/Event.h"

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
	}
}