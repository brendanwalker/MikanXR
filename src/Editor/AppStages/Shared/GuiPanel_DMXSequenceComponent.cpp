#include "AppStage.h"
#include "DMXFixtureComponent.h"
#include "DMXFixtureGroupComponent.h"
#include "DMXFixtureGroupSystem.h"
#include "DMXSequenceComponent.h"
#include "DMXSequenceSystem.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "ProjectManager.h"
#include "ProjectScriptContext.h"
#include "RGBPixelGridComponent.h"
#include "RGBSpotLightComponent.h"
#include "ScriptComponent.h"
#include "ScriptObjectSystem.h"
#include "Shared/GuiPanel_DMXSequenceComponent.h"

#include "imgui.h"

#include <algorithm>

GuiPanel_DMXSequenceComponent::GuiPanel_DMXSequenceComponent(AppStage* ownerAppStage)
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_groupDataSource(ownerAppStage->getProjectManager(), {{DMXFixtureGroupSystem::k_objectSystemClassName,
															  DMXFixtureGroupComponent::k_componentClassName}})
{
}

bool GuiPanel_DMXSequenceComponent::init() { return initTypedPropertyInterface<DMXSequenceComponent>(); }

void GuiPanel_DMXSequenceComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	// The owning fixture group as a picker over the live groups
	m_entityAccessor->setPropertyRenderer(
		DMXSequenceDefinition::k_groupIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			DMXSequenceComponentPtr sequenceComp= getDMXSequenceComponent();
			if (!sequenceComp)
				return false;

			m_groupDataSource.refreshEntries();
			if (m_groupDataSource.getEntryCount() == 0)
				return false;

			DMXSequenceDefinitionPtr sequenceDef= sequenceComp->getDMXSequenceDefinition();
			int selectedIndex= m_groupDataSource.getEntryIndexByComponentId(sequenceDef->getGroupId());
			if (MkGui::drawComboBoxProperty(
					m_defaultGuiStyle,
					sequenceComp->makePropertyUIIdentifier(DMXSequenceDefinition::k_groupIdPropertyId),
					locText("componentPanel.presetGroup"), &m_groupDataSource, selectedIndex))
			{
				MikanComponentPtr newGroup= m_groupDataSource.getEntryAtIndex(selectedIndex);
				if (newGroup)
				{
					const MikanDMXFixtureGroupID newGroupId= newGroup->getComponentId();
					addDeferredGuiEvent([sequenceComp, newGroupId]()
										{ sequenceComp->getDMXSequenceDefinition()->setGroupId(newGroupId); });
				}
			}

			return true;
		});

	// The registered script sequence handler this component plays, plus a
	// shortcut to the script that registered it
	m_entityAccessor->setPropertyRenderer(
		DMXSequenceDefinition::k_sequenceNamePropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			DMXSequenceComponentPtr sequenceComp= getDMXSequenceComponent();
			if (!sequenceComp)
				return false;

			ScriptObjectSystemPtr scriptSystem=
				getOwnerAppStage()->getProjectManager()->getSystemOfType<ScriptObjectSystem>();
			CommonScriptContextPtr context= scriptSystem ? scriptSystem->getScriptContext() : nullptr;

			std::vector<std::string> names;
			if (context)
				context->getSequenceNames(names);

			std::vector<std::string> choices;
			choices.push_back(locText("componentPanel.none"));
			choices.insert(choices.end(), names.begin(), names.end());

			DMXSequenceDefinitionPtr sequenceDef= sequenceComp->getDMXSequenceDefinition();
			const std::string& currentName= sequenceDef->getSequenceName();

			int selectedIndex= 0;
			for (size_t nameIndex= 0; nameIndex < names.size(); ++nameIndex)
			{
				if (names[nameIndex] == currentName)
				{
					selectedIndex= (int)nameIndex + 1;
					break;
				}
			}

			if (MkGui::drawEnumComboBoxProperty(
					m_defaultGuiStyle,
					sequenceComp->makePropertyUIIdentifier(DMXSequenceDefinition::k_sequenceNamePropertyId),
					locText("componentPanel.sequenceHandler"), choices, selectedIndex))
			{
				const std::string newName= (selectedIndex == 0) ? "" : names[selectedIndex - 1];
				addDeferredGuiEvent([sequenceComp, newName]()
									{ sequenceComp->getDMXSequenceDefinition()->setSequenceName(newName); });
			}

			if (!currentName.empty())
			{
				if (context && context->hasSequence(currentName))
				{
					ScriptComponentPtr script=
						scriptSystem->getTypedComponentById(context->getSequenceScriptId(currentName));
					if (script)
					{
						MkGui::drawStaticTextProperty(m_defaultGuiStyle, locText("componentPanel.sequenceScript"),
													  script->getName());

						if (MkGui::drawGlyphButtonWithLabel(
								sequenceComp->makePropertyUIIdentifier("edit_sequence_script"), ICON_FK_PENCIL,
								locText("componentPanel.editScript")))
						{
							addDeferredGuiEvent([script]() { script->editScript(); });
						}
					}
				}
				else
				{
					MkGui::drawStaticTextProperty(m_defaultGuiStyle, locText("componentPanel.sequenceScript"),
												  locText("componentPanel.sequenceHandlerMissing"));
				}
			}

			return true;
		});

	// Only the settings the active source reads stay on the sheet. The handler
	// picker is left visible in every mode, since a rasterized source can still
	// name a handler to pick its content on start.
	hideUnlessContentSource(DMXSequenceDefinition::k_contentPathPropertyId,
							{eDMXSequenceContentSource::scrollBitmap, eDMXSequenceContentSource::playAnimation});
	hideUnlessContentSource(DMXSequenceDefinition::k_scrollTextPropertyId, {eDMXSequenceContentSource::scrollText});
	hideUnlessContentSource(DMXSequenceDefinition::k_fontPathPropertyId, {eDMXSequenceContentSource::scrollText});
	hideUnlessContentSource(DMXSequenceDefinition::k_textPixelHeightPropertyId,
							{eDMXSequenceContentSource::scrollText});
	hideUnlessContentSource(DMXSequenceDefinition::k_scrollDirectionPropertyId,
							{eDMXSequenceContentSource::scrollBitmap, eDMXSequenceContentSource::scrollText});
	hideUnlessContentSource(DMXSequenceDefinition::k_scrollSpeedPropertyId,
							{eDMXSequenceContentSource::scrollBitmap, eDMXSequenceContentSource::scrollText});
	hideUnlessContentSource(DMXSequenceDefinition::k_spriteFrameWidthPropertyId,
							{eDMXSequenceContentSource::playAnimation});
	hideUnlessContentSource(DMXSequenceDefinition::k_spriteFrameHeightPropertyId,
							{eDMXSequenceContentSource::playAnimation});
	hideUnlessContentSource(DMXSequenceDefinition::k_spriteFpsPropertyId, {eDMXSequenceContentSource::playAnimation});
	hideUnlessContentSource(DMXSequenceDefinition::k_playbackSpeedScalePropertyId,
							{eDMXSequenceContentSource::playAnimation});

	// The foreground only means anything to rasterized text; the background
	// also fills wherever a scrolled image does not reach. Both draw as color
	// pickers, and both hide the same way the rest do, which is why one
	// renderer covers both jobs: the map holds a single renderer per property.
	addColorPropertyRenderer(DMXSequenceDefinition::k_foregroundColorPropertyId, "properties.foreground_color",
							 {eDMXSequenceContentSource::scrollText});
	addColorPropertyRenderer(DMXSequenceDefinition::k_backgroundColorPropertyId, "properties.background_color",
							 {eDMXSequenceContentSource::scrollBitmap, eDMXSequenceContentSource::scrollText});

	// Playback status, then the current per-fixture frame the handler wrote,
	// drawn read-only by fixture kind the same way the preset panel draws its
	// editable swatches
	m_entityAccessor->setPropertyRenderer(
		DMXSequenceComponent::k_playbackStatePropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			DMXSequenceComponentPtr sequenceComp= getDMXSequenceComponent();
			if (!sequenceComp)
				return false;

			const char* stateText= "";
			switch (sequenceComp->getPlaybackState())
			{
			case eDMXSequenceState::Stopped:
				stateText= locText("propertyValues.sequence_stopped");
				break;
			case eDMXSequenceState::Playing:
				stateText= locText("propertyValues.sequence_playing");
				break;
			case eDMXSequenceState::Paused:
				stateText= locText("propertyValues.sequence_paused");
				break;
			}
			ImGui::Text(locText("componentPanel.sequenceStatusFmt"), stateText, sequenceComp->getTimeSinceStart());

			DMXFixtureGroupComponentPtr group= sequenceComp->getGroup();
			if (!group)
				return true;

			// Spot light swatches flow back to back and wrap at the panel edge;
			// a pixel grid keeps its rows and starts a new line. The fixture name
			// is a hover tooltip rather than a label, to keep the strip dense.
			const float swatchSize= ImGui::GetFrameHeight();
			bool bStripOpen= false;
			for (MikanLightID fixtureId : group->getDMXFixtureGroupDefinition()->getFixtureIds())
			{
				DMXFixtureComponentPtr fixture= group->resolveFixture(fixtureId);
				if (!fixture)
					continue;

				std::vector<uint8_t> values;
				if (!sequenceComp->getFrameBufferValues(fixtureId, values))
					continue;

				const std::string componentClass= fixture->getComponentClassName();
				if (componentClass == RGBSpotLightComponent::k_componentClassName)
				{
					const ImVec4 rgb(values.size() > 0 ? values[0] / 255.0f : 0.0f,
									 values.size() > 1 ? values[1] / 255.0f : 0.0f,
									 values.size() > 2 ? values[2] / 255.0f : 0.0f, 1.0f);

					if (bStripOpen)
						MkGui::sameLineIfFits(swatchSize);
					bStripOpen= true;

					const std::string id=
						"##" + sequenceComp->makePropertyUIIdentifier("seq_" + std::to_string(fixtureId));
					ImGui::ColorButton(id.c_str(), rgb,
									   ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop
										   | ImGuiColorEditFlags_NoAlpha);
					ImGui::SetItemTooltip("%s", fixture->getName().c_str());
				}
				else if (componentClass == RGBPixelGridComponent::k_componentClassName)
				{
					RGBPixelGridComponentPtr pixelGrid= std::static_pointer_cast<RGBPixelGridComponent>(fixture);
					const int columns= pixelGrid->getRGBPixelGridDefinition()->getColumns();
					const int rows= pixelGrid->getRGBPixelGridDefinition()->getRows();
					bStripOpen= false;

					for (int row= 0; row < rows; ++row)
					{
						for (int col= 0; col < columns; ++col)
						{
							if (col > 0)
								ImGui::SameLine();

							const int wireIndex= pixelGrid->getRGBPixelGridDefinition()->getPixelWireIndex(col, row);
							const size_t offset= wireIndex >= 0 ? (size_t)wireIndex * 3 : values.size();
							ImVec4 rgb(0.0f, 0.0f, 0.0f, 1.0f);
							if (offset + 3 <= values.size())
							{
								rgb.x= values[offset] / 255.0f;
								rgb.y= values[offset + 1] / 255.0f;
								rgb.z= values[offset + 2] / 255.0f;
							}

							const std::string id= "##"
												  + sequenceComp->makePropertyUIIdentifier(
													  "seq_" + std::to_string(fixtureId) + "_" + std::to_string(row)
													  + "_" + std::to_string(col));
							ImGui::ColorButton(id.c_str(), rgb,
											   ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop
												   | ImGuiColorEditFlags_NoAlpha);
							ImGui::SetItemTooltip("%s (%d, %d)", fixture->getName().c_str(), col, row);
						}
					}
				}
				else
				{
					bStripOpen= false;
					ImGui::TextUnformatted(fixture->getName().c_str());
					const std::string baseId=
						sequenceComp->makePropertyUIIdentifier("seq_" + std::to_string(fixtureId));
					for (int channelIndex= 0; channelIndex < (int)values.size(); ++channelIndex)
					{
						ImGui::Text("%d", channelIndex + 1);
						ImGui::SameLine();

						int channelValue= values[channelIndex];
						const std::string id= "##" + baseId + "_" + std::to_string(channelIndex);
						ImGui::BeginDisabled();
						ImGui::SliderInt(id.c_str(), &channelValue, 0, 255);
						ImGui::EndDisabled();
					}
				}
			}

			return true;
		});
}

void GuiPanel_DMXSequenceComponent::hideUnlessContentSource(const std::string& propertyName,
															const std::vector<eDMXSequenceContentSource>& sources)
{
	m_entityAccessor->setPropertyRenderer(
		propertyName,
		[this, sources](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			DMXSequenceComponentPtr sequenceComp= getDMXSequenceComponent();
			if (!sequenceComp)
				return false;

			const eDMXSequenceContentSource activeSource= sequenceComp->getDMXSequenceDefinition()->getContentSource();

			// Claimed and drawn as nothing when the active source ignores it,
			// otherwise handed back to the default renderer
			return std::find(sources.begin(), sources.end(), activeSource) == sources.end();
		});
}

void GuiPanel_DMXSequenceComponent::addColorPropertyRenderer(const std::string& propertyName,
															 const std::string& labelKey,
															 const std::vector<eDMXSequenceContentSource>& sources)
{
	m_entityAccessor->setPropertyRenderer(
		propertyName,
		[this, propertyName, labelKey, sources](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			DMXSequenceComponentPtr sequenceComp= getDMXSequenceComponent();
			if (!sequenceComp)
				return false;

			const eDMXSequenceContentSource activeSource= sequenceComp->getDMXSequenceDefinition()->getContentSource();
			if (std::find(sources.begin(), sources.end(), activeSource) == sources.end())
				return true;

			MikanVariant value;
			if (!sequenceComp->getPropertyValue(propertyName, value) || value.value_type != MikanVariantType::VECTOR3F)
			{
				return true;
			}

			const MikanVector3f& color= value.getVector3fValue();
			float rgb[3]= {color.x, color.y, color.z};

			ImGui::TextUnformatted(locText(labelKey.c_str()));
			ImGui::SameLine();

			const std::string id= "##" + sequenceComp->makePropertyUIIdentifier(propertyName);
			if (ImGui::ColorEdit3(id.c_str(), rgb, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
			{
				MikanVariant newColor;
				newColor.setValue(MikanVector3f{rgb[0], rgb[1], rgb[2]});
				addDeferredGuiEvent([sequenceComp, propertyName, newColor]()
									{ sequenceComp->setPropertyValue(propertyName, newColor); });
			}

			return true;
		});
}

DMXSequenceComponentPtr GuiPanel_DMXSequenceComponent::getDMXSequenceComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
		return std::static_pointer_cast<DMXSequenceComponent>(component);
	return nullptr;
}
