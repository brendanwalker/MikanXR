#pragma once

#include <memory>

// -- DMXFixtureComponent --
class DMXFixtureComponentDefinition;
using DMXFixtureComponentDefinitionPtr = std::shared_ptr<DMXFixtureComponentDefinition>;
using DMXFixtureComponentDefinitionConstPtr = std::shared_ptr<const DMXFixtureComponentDefinition>;
using DMXFixtureComponentDefinitionWeakPtr = std::weak_ptr<DMXFixtureComponentDefinition>;

class DMXFixtureComponent;
using DMXFixtureComponentPtr = std::shared_ptr<DMXFixtureComponent>;
using DMXFixtureComponentConstPtr = std::shared_ptr<const DMXFixtureComponent>;
using DMXFixtureComponentWeakPtr = std::weak_ptr<DMXFixtureComponent>;

class DMXObjectSystemDefinition;
using DMXObjectSystemDefinitionPtr = std::shared_ptr<DMXObjectSystemDefinition>;
using DMXObjectSystemDefinitionConstPtr = std::shared_ptr<const DMXObjectSystemDefinition>;
using DMXObjectSystemDefinitionWeakPtr = std::weak_ptr<DMXObjectSystemDefinition>;

class DMXObjectSystem;
using DMXObjectSystemPtr = std::shared_ptr<DMXObjectSystem>;
using DMXObjectSystemConstPtr = std::shared_ptr<const DMXObjectSystem>;
using DMXObjectSystemWeakPtr = std::weak_ptr<DMXObjectSystem>;

// -- RGBSpotLight --
class RGBSpotLightDefinition;
using RGBSpotLightDefinitionPtr = std::shared_ptr<RGBSpotLightDefinition>;
using RGBSpotLightDefinitionConstPtr = std::shared_ptr<const RGBSpotLightDefinition>;
using RGBSpotLightDefinitionWeakPtr = std::weak_ptr<RGBSpotLightDefinition>;

class RGBSpotLightComponent;
using RGBSpotLightComponentPtr = std::shared_ptr<RGBSpotLightComponent>;
using RGBSpotLightComponentConstPtr = std::shared_ptr<const RGBSpotLightComponent>;
using RGBSpotLightComponentWeakPtr = std::weak_ptr<RGBSpotLightComponent>;

class RGBSpotLightSystemDefinition;
using RGBSpotLightSystemDefinitionPtr = std::shared_ptr<RGBSpotLightSystemDefinition>;
using RGBSpotLightSystemDefinitionConstPtr = std::shared_ptr<const RGBSpotLightSystemDefinition>;
using RGBSpotLightSystemDefinitionWeakPtr = std::weak_ptr<RGBSpotLightSystemDefinition>;

class RGBSpotLightSystem;
using RGBSpotLightSystemPtr = std::shared_ptr<RGBSpotLightSystem>;
using RGBSpotLightSystemConstPtr = std::shared_ptr<const RGBSpotLightSystem>;
using RGBSpotLightSystemWeakPtr = std::weak_ptr<RGBSpotLightSystem>;

// -- RGBPixelGrid --
class RGBPixelGridDefinition;
using RGBPixelGridDefinitionPtr = std::shared_ptr<RGBPixelGridDefinition>;
using RGBPixelGridDefinitionConstPtr = std::shared_ptr<const RGBPixelGridDefinition>;
using RGBPixelGridDefinitionWeakPtr = std::weak_ptr<RGBPixelGridDefinition>;

class RGBPixelGridComponent;
using RGBPixelGridComponentPtr = std::shared_ptr<RGBPixelGridComponent>;
using RGBPixelGridComponentConstPtr = std::shared_ptr<const RGBPixelGridComponent>;
using RGBPixelGridComponentWeakPtr = std::weak_ptr<RGBPixelGridComponent>;

class RGBPixelGridSystemDefinition;
using RGBPixelGridSystemDefinitionPtr = std::shared_ptr<RGBPixelGridSystemDefinition>;
using RGBPixelGridSystemDefinitionConstPtr = std::shared_ptr<const RGBPixelGridSystemDefinition>;
using RGBPixelGridSystemDefinitionWeakPtr = std::weak_ptr<RGBPixelGridSystemDefinition>;

class RGBPixelGridSystem;
using RGBPixelGridSystemPtr = std::shared_ptr<RGBPixelGridSystem>;
using RGBPixelGridSystemConstPtr = std::shared_ptr<const RGBPixelGridSystem>;
using RGBPixelGridSystemWeakPtr = std::weak_ptr<RGBPixelGridSystem>;
