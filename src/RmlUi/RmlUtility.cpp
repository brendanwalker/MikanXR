#pragma once

#include "RmlUI/Core/ElementDocument.h"

namespace Rml
{
	namespace Utilities
	{
		void ToggleElementDocumentVisibility(ElementDocument* elementDoc)
		{
			if (elementDoc != nullptr)
			{
				const Rml::Property* property = elementDoc->GetProperty(Rml::PropertyId::Visibility);

				if (property != nullptr)
				{
					const auto Visibility = (Rml::Style::Visibility)property->Get<int>();

					if (Visibility == Rml::Style::Visibility::Hidden)
					{
						elementDoc->Show(Rml::ModalFlag::None, Rml::FocusFlag::Document);
					}
					else
					{
						elementDoc->Hide();
					}
				}
			}
		}
	}
}