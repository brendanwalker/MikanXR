#include "AppStage.h"
#include "CalibrationPatternFinder.h"
#include "IMkTexture.h"
#include "MarkerComponent.h"
#include "OpenCVFwd.h"
#include "Shared/GuiDataSource_IntList.h"
#include "Shared/GuiPanel_MarkerComponent.h"
#include "MarkerObjectSystem.h"

#include "opencv2/objdetect/aruco_detector.hpp"
#include "opencv2/imgproc.hpp"

bool GuiPanel_MarkerComponent::init()
{
	m_markerObjectSystem = getOwnerAppStage()->getSystemOfType<MarkerObjectSystem>();
	return initTypedPropertyInterface<MarkerComponent>();
}

bool GuiPanel_MarkerComponent::setComponent(MikanComponentPtr component)
{
	if (GuiPanel_MikanComponent::setComponent(component))
	{
		MarkerComponentPtr markerComp = getMarkerComponent();
		if (markerComp && OnMarkerSelected)
		{
			OnMarkerSelected(markerComp->getMarkerDefinition()->getArucoId());
		}
		return true;
	}
	return false;
}

void GuiPanel_MarkerComponent::onConstruct()
{
	m_entityAccessor->setPropertyRenderer(
		MarkerDefinition::k_arucoIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			MarkerComponentPtr markerComp = getMarkerComponent();
			if (!markerComp) return false;

			// Build aruco ID list from all markers in the system
			std::vector<int> markerIdList;
			getMarkerObjectSystem()->getTypedDefinition()->getArucoIdList(markerIdList);
			m_arucoIdDataSource.setEntries(markerIdList);

			const int currentArucoId = markerComp->getMarkerDefinition()->getArucoId();
			int selectedIndex = m_arucoIdDataSource.getEntryIndexByValue(currentArucoId);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle, "markerArucoId", "Aruco ID",
				&m_arucoIdDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const int newArucoId = m_arucoIdDataSource.getEntryValue(selectedIndex);
					addDeferredGuiEvent([this, markerComp, newArucoId]() {
						markerComp->getMarkerDefinition()->setArucoId(newArucoId);
						if (OnMarkerSelected)
						{
							OnMarkerSelected(newArucoId);
						}
					});
				}
			}
			// Regenerate cached texture when the selected aruco ID changes
			if (currentArucoId >= 0 && currentArucoId != m_cachedArucoId)
			{
				m_cachedArucoId = currentArucoId;
				m_cachedMarkerTexture.reset();

				const eCharucoDictionaryType dictType =
					getMarkerObjectSystemDefinition()->getArucoDictionaryType();
				ArucoDictionaryPtr dictionary =
					CalibrationPatternFinder::getArucoDictionary(dictType);

				if (dictionary)
				{
					const int imageSize = 128;
					const int maxMarkerId = dictionary->bytesList.rows - 1;
					const int clampedId = std::max(0, std::min(currentArucoId, maxMarkerId));

					cv::Mat markerImage;
					cv::aruco::generateImageMarker(*dictionary, clampedId, imageSize, markerImage, 1);

					cv::Mat rgbImage;
					cv::cvtColor(markerImage, rgbImage, cv::COLOR_GRAY2RGB);

					IMkTexturePtr tex = CreateMkTexture(
						(uint16_t)imageSize, (uint16_t)imageSize,
						rgbImage.data,
						MK_RGB,
						MK_RGB);
					if (tex->createTexture())
						m_cachedMarkerTexture = tex;
				}
			}

			// Display the marker preview
			if (m_cachedMarkerTexture)
			{
				MkGui::drawImage(m_cachedMarkerTexture, 128.0f, 128.0f);
			}

			return true;
		});
}

MarkerObjectSystemPtr GuiPanel_MarkerComponent::getMarkerObjectSystem() const
{
	return m_markerObjectSystem.lock();
}

MarkerObjectSystemDefinitionPtr GuiPanel_MarkerComponent::getMarkerObjectSystemDefinition() const
{
	auto markerObjectSystem = getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getTypedDefinition();
	}
	return nullptr;
}

MarkerComponentPtr GuiPanel_MarkerComponent::getMarkerComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<MarkerComponent>(component);
	}
	return nullptr;
}
