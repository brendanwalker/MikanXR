#pragma once

#include "TextureSourceComponent.h"

#include "include/internal/cef_ptr.h"

#include <mutex>
#include <vector>

// Forward declaration so CEFBrowserClient can reference the component
class CEFTextureSourceComponent;

// -------------------------------------------------------------------------------------------------
// CEFTextureSourceDefinition
// -------------------------------------------------------------------------------------------------
class CEFTextureSourceDefinition : public TextureSourceDefinition
{
public:
	CEFTextureSourceDefinition();
	CEFTextureSourceDefinition(MikanTextureSourceID TextureSourceId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
	virtual bool readFromInitParams(
		MikanObjectSystem* ownerObjectSystem,
		const Serialization::PolymorphicObjectPtr& initParams) override;

	static const std::string k_urlPropertyId;
	static const std::string k_widthPropertyId;
	static const std::string k_heightPropertyId;

	inline const std::string& getUrl() const { return m_url; }
	inline int getWidth() const { return m_width; }
	inline int getHeight() const { return m_height; }

	void setUrl(const std::string& url);
	void setWidth(int width);
	void setHeight(int height);

private:
	std::string m_url;
	int m_width= 1280;
	int m_height= 720;
};

// -------------------------------------------------------------------------------------------------
// CEFTextureSourceComponent
// -------------------------------------------------------------------------------------------------
class CEFTextureSourceComponent : public TextureSourceComponent
{
public:
	CEFTextureSourceComponent(MikanObjectWeakPtr owner);

	inline CEFTextureSourceDefinitionPtr getCEFTextureSourceDefinition() const
	{
		return std::static_pointer_cast<CEFTextureSourceDefinition>(m_definition);
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;
	virtual void update(float deltaSeconds) override;
	virtual void dispose() override;

	inline static const std::string k_componentClassName= "CEFTextureSourceComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	// Texture Source Interface
	virtual IMkTexturePtr getClientColorSourceTexture(MikanCameraID cameraId, eTextureSourceColorType textureSourceColorType, int64_t frameIndex= -1) const override;

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- IFunctionInterface ----
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
	{
		TextureSourceComponent::getFunctionDescriptors(outDescriptors);
	}
	virtual void showTextureSourceSettings() override;

	// -- CEFBrowserClient callbacks (called from CEFBrowserClient) ----
	void onCefGetViewRect(class CefRect& rect);
	void onCefPaint(const void* buffer, int width, int height);
	void onCefBrowserCreated(CefRefPtr<class CefBrowser> browser);
	void onCefBrowserClosed();

protected:
	virtual void onDefinitionMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet) override;
	void closeTextureSource();
	void openTextureSource();

private:
	CefRefPtr<class CefBrowser> m_browser;
	CefRefPtr<class CEFBrowserClient> m_cefClient;
	IMkTexturePtr m_colorTexture;

	std::mutex m_stagingMutex;
	std::vector<uint8_t> m_stagingBuffer;
	int m_stagingWidth= 0;
	int m_stagingHeight= 0;
	bool m_dirty= false;
};
