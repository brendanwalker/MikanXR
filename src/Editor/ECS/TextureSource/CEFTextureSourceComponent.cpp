#include "IMkTexture.h"
#include "Logger.h"
#include "MikanTextureSourceTypes.h"
#include "CEFBrowserClient.h"
#include "CEFTextureSourceComponent.h"
#include "StringUtils.h"
#include "TextureSourceSettings/AppStage_TextureSourceSettings.h"

#include "include/cef_browser.h"

#include <easy/profiler.h>

// -- CEFTextureSourceDefinition ------
const std::string CEFTextureSourceDefinition::k_urlPropertyId= "url";
const std::string CEFTextureSourceDefinition::k_widthPropertyId= "width";
const std::string CEFTextureSourceDefinition::k_heightPropertyId= "height";

CEFTextureSourceDefinition::CEFTextureSourceDefinition()
	: TextureSourceDefinition()
{
}

CEFTextureSourceDefinition::CEFTextureSourceDefinition(MikanTextureSourceID textureSourceId)
	: TextureSourceDefinition(textureSourceId)
{
}

configuru::Config CEFTextureSourceDefinition::writeToJSON()
{
	configuru::Config pt= TextureSourceDefinition::writeToJSON();

	pt["cef_url"]= m_url;
	pt["cef_width"]= m_width;
	pt["cef_height"]= m_height;

	return pt;
}

void CEFTextureSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	TextureSourceDefinition::readFromJSON(pt);

	m_url= pt.get_or<std::string>("cef_url", m_url);
	m_width= pt.get_or<int>("cef_width", m_width);
	m_height= pt.get_or<int>("cef_height", m_height);
}

bool CEFTextureSourceDefinition::readFromInitParams(MikanObjectSystem* ownerObjectSystem,
													const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!TextureSourceDefinition::readFromInitParams(ownerObjectSystem, initParams))
		return false;

	const auto* componentValues= initParams.getTypedPointer<MikanCEFTextureSourceValues>();
	if (componentValues)
	{
		m_url= componentValues->url.getUtf8Value();
		m_width= componentValues->width;
		m_height= componentValues->height;
	}

	return true;
}

void CEFTextureSourceDefinition::setUrl(const std::string& url)
{
	if (url != m_url)
	{
		m_url= url;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_urlPropertyId));
	}
}

void CEFTextureSourceDefinition::setWidth(int width)
{
	if (width != m_width)
	{
		m_width= width;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_widthPropertyId));
	}
}

void CEFTextureSourceDefinition::setHeight(int height)
{
	if (height != m_height)
	{
		m_height= height;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_heightPropertyId));
	}
}

// -- CEFTextureSourceComponent -----
CEFTextureSourceComponent::CEFTextureSourceComponent(MikanObjectWeakPtr owner)
	: TextureSourceComponent(owner)
{
	m_bWantsUpdate= true;
}

// -- IEntityAccessor ----
rfk::Struct const* CEFTextureSourceComponent::getClientAPIValuesStructType() const
{
	return &MikanCEFTextureSourceValues::staticGetArchetype();
}

void CEFTextureSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	openTextureSource();
}

void CEFTextureSourceComponent::onDefinitionMarkedDirty(CommonConfigPtr configPtr,
														const ConfigPropertyChangeSet& changedPropertySet)
{
	TextureSourceComponent::onDefinitionMarkedDirty(configPtr, changedPropertySet);

	if (changedPropertySet.hasPropertyName(CEFTextureSourceDefinition::k_urlPropertyId))
	{
		const std::string& url= getCEFTextureSourceDefinition()->getUrl();
		if (m_browser && !url.empty())
		{
			// Navigate in-place — much cheaper than tearing down and recreating the browser.
			m_browser->GetMainFrame()->LoadURL(url);
		}
		else
		{
			// No live browser yet (e.g. URL was previously empty); do a full open.
			openTextureSource();
		}
	}
	else if (changedPropertySet.hasPropertyName(CEFTextureSourceDefinition::k_widthPropertyId)
			 || changedPropertySet.hasPropertyName(CEFTextureSourceDefinition::k_heightPropertyId))
	{
		// Size changes are handled by the browser sending a new OnPaint at the updated GetViewRect size
		if (m_browser)
		{
			m_browser->GetHost()->WasResized();
		}
	}
}

void CEFTextureSourceComponent::update(float deltaSeconds)
{
	EASY_BLOCK("CEFTextureSource: upload texture");

	std::lock_guard<std::mutex> lock(m_stagingMutex);

	if (!m_dirty || m_stagingBuffer.empty())
		return;

	const int w= m_stagingWidth;
	const int h= m_stagingHeight;

	if (w <= 0 || h <= 0)
		return;

	// Reallocate GPU texture if size changed
	const int textureWidth= m_colorTexture ? m_colorTexture->getTextureWidth() : 0;
	const int textureHeight= m_colorTexture ? m_colorTexture->getTextureHeight() : 0;

	if (w != textureWidth || h != textureHeight)
	{
		if (m_colorTexture)
		{
			m_colorTexture->disposeTexture();
			m_colorTexture= nullptr;
		}

		m_colorTexture= CreateMkTexture();
		m_colorTexture->setSize((uint16_t)w, (uint16_t)h);
		m_colorTexture->setTextureFormat(MK_BGRA);
		m_colorTexture->setBufferFormat(MK_BGRA);
		m_colorTexture->setPixelBufferObjectMode(IMkTexture::PixelBufferObjectMode::SinglePBOWrite);
		m_colorTexture->createTexture();
	}

	if (m_colorTexture)
	{
		m_colorTexture->copyBufferIntoTexture(m_stagingBuffer.data(), m_stagingBuffer.size());
	}

	m_dirty= false;
}

void CEFTextureSourceComponent::dispose()
{
	closeTextureSource();

	TextureSourceComponent::dispose();
}

void CEFTextureSourceComponent::closeTextureSource()
{
	if (m_browser)
	{
		m_browser->GetHost()->CloseBrowser(true);
		m_browser= nullptr;
	}

	// Release the client after the browser is gone so CEF's refcount can reach zero cleanly
	m_cefClient= nullptr;

	if (m_colorTexture)
	{
		m_colorTexture->disposeTexture();
		m_colorTexture= nullptr;
	}

	{
		std::lock_guard<std::mutex> lock(m_stagingMutex);
		m_stagingBuffer.clear();
		m_stagingWidth= 0;
		m_stagingHeight= 0;
		m_dirty= false;
	}
}

void CEFTextureSourceComponent::openTextureSource()
{
	closeTextureSource();

	const auto def= getCEFTextureSourceDefinition();
	if (!def)
		return;

	const std::string& url= def->getUrl();
	if (url.empty())
		return;

	m_cefClient= new CEFBrowserClient(getSelfPtr<CEFTextureSourceComponent>());

	CefWindowInfo windowInfo;
	windowInfo.SetAsWindowless(nullptr);

	CefBrowserSettings browserSettings;
	browserSettings.windowless_frame_rate= 60;

	CefBrowserHost::CreateBrowser(windowInfo, m_cefClient, url, browserSettings, nullptr, nullptr);
}

// -- Texture Source Interface ---
IMkTexturePtr CEFTextureSourceComponent::getClientColorSourceTexture(MikanCameraID cameraId,
																	 eTextureSourceColorType textureSourceColorType,
																	 int64_t frameIndex) const
{
	return m_colorTexture;
}

// -- CEFBrowserClient callbacks ----
void CEFTextureSourceComponent::onCefGetViewRect(CefRect& rect)
{
	const auto def= getCEFTextureSourceDefinition();
	if (def)
	{
		rect= CefRect(0, 0, def->getWidth(), def->getHeight());
	}
	else
	{
		rect= CefRect(0, 0, 1280, 720);
	}
}

void CEFTextureSourceComponent::onCefPaint(const void* buffer, int width, int height)
{
	const size_t bufferSize= (size_t)width * height * 4; // BGRA = 4 bytes per pixel

	std::lock_guard<std::mutex> lock(m_stagingMutex);
	m_stagingBuffer.resize(bufferSize);
	memcpy(m_stagingBuffer.data(), buffer, bufferSize);
	m_stagingWidth= width;
	m_stagingHeight= height;
	m_dirty= true;
}

void CEFTextureSourceComponent::onCefBrowserCreated(CefRefPtr<CefBrowser> browser) { m_browser= browser; }

void CEFTextureSourceComponent::onCefBrowserClosed() { m_browser= nullptr; }

// -- IPropertyInterface ----
void CEFTextureSourceComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TextureSourceComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(CEFTextureSourceDefinition::k_urlPropertyId, MikanVariantType::STRING));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(CEFTextureSourceDefinition::k_widthPropertyId, MikanVariantType::INT));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(CEFTextureSourceDefinition::k_heightPropertyId, MikanVariantType::INT));
}

bool CEFTextureSourceComponent::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	const auto def= getCEFTextureSourceDefinition();

	if (propertyName == CEFTextureSourceDefinition::k_urlPropertyId)
	{
		outValue= def->getUrl();
		return true;
	}
	else if (propertyName == CEFTextureSourceDefinition::k_widthPropertyId)
	{
		outValue= def->getWidth();
		return true;
	}
	else if (propertyName == CEFTextureSourceDefinition::k_heightPropertyId)
	{
		outValue= def->getHeight();
		return true;
	}

	return TextureSourceComponent::getPropertyValue(propertyName, outValue);
}

bool CEFTextureSourceComponent::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	const auto def= getCEFTextureSourceDefinition();

	if (propertyName == CEFTextureSourceDefinition::k_urlPropertyId)
	{
		const std::string url= inValue.getUtf8Value();
		def->setUrl(url);
		return true;
	}
	else if (propertyName == CEFTextureSourceDefinition::k_widthPropertyId)
	{
		def->setWidth(inValue.getIntValue());
		return true;
	}
	else if (propertyName == CEFTextureSourceDefinition::k_heightPropertyId)
	{
		def->setHeight(inValue.getIntValue());
		return true;
	}

	return TextureSourceComponent::setPropertyValue(propertyName, inValue);
}

void CEFTextureSourceComponent::showTextureSourceSettings()
{
	auto* appStage= getOwnerEditorWindow()->pushAppStageOfType<AppStage_TextureSourceSettings>();

	appStage->setSourceCameraId(INVALID_MIKAN_ID);
	appStage->setTextureSourceComponent(getSelfPtr<TextureSourceComponent>());
}
