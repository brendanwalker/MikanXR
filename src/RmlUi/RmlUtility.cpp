#pragma once

#include "RmlUI/Core/ElementDocument.h"

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
	}
}