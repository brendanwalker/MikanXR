#pragma once

#include <memory>

// -- DMXFixtureComponent --
class DMXFixtureComponentDefinition;
using DMXFixtureComponentDefinitionPtr= std::shared_ptr<DMXFixtureComponentDefinition>;
using DMXFixtureComponentDefinitionConstPtr= std::shared_ptr<const DMXFixtureComponentDefinition>;
using DMXFixtureComponentDefinitionWeakPtr= std::weak_ptr<DMXFixtureComponentDefinition>;

class DMXFixtureComponent;
using DMXFixtureComponentPtr= std::shared_ptr<DMXFixtureComponent>;
using DMXFixtureComponentConstPtr= std::shared_ptr<const DMXFixtureComponent>;
using DMXFixtureComponentWeakPtr= std::weak_ptr<DMXFixtureComponent>;

class DMXObjectSystemDefinition;
using DMXObjectSystemDefinitionPtr= std::shared_ptr<DMXObjectSystemDefinition>;
using DMXObjectSystemDefinitionConstPtr= std::shared_ptr<const DMXObjectSystemDefinition>;
using DMXObjectSystemDefinitionWeakPtr= std::weak_ptr<DMXObjectSystemDefinition>;

class DMXObjectSystem;
using DMXObjectSystemPtr= std::shared_ptr<DMXObjectSystem>;
using DMXObjectSystemConstPtr= std::shared_ptr<const DMXObjectSystem>;
using DMXObjectSystemWeakPtr= std::weak_ptr<DMXObjectSystem>;

// -- LightEnvironment --
class LightEnvironmentDefinition;
using LightEnvironmentDefinitionPtr= std::shared_ptr<LightEnvironmentDefinition>;
using LightEnvironmentDefinitionConstPtr= std::shared_ptr<const LightEnvironmentDefinition>;
using LightEnvironmentDefinitionWeakPtr= std::weak_ptr<LightEnvironmentDefinition>;

class LightEnvironmentComponent;
using LightEnvironmentComponentPtr= std::shared_ptr<LightEnvironmentComponent>;
using LightEnvironmentComponentConstPtr= std::shared_ptr<const LightEnvironmentComponent>;
using LightEnvironmentComponentWeakPtr= std::weak_ptr<LightEnvironmentComponent>;

class LightEnvironmentSystemDefinition;
using LightEnvironmentSystemDefinitionPtr= std::shared_ptr<LightEnvironmentSystemDefinition>;
using LightEnvironmentSystemDefinitionConstPtr= std::shared_ptr<const LightEnvironmentSystemDefinition>;

class LightEnvironmentSystem;
using LightEnvironmentSystemPtr= std::shared_ptr<LightEnvironmentSystem>;
using LightEnvironmentSystemConstPtr= std::shared_ptr<const LightEnvironmentSystem>;
using LightEnvironmentSystemWeakPtr= std::weak_ptr<LightEnvironmentSystem>;

// -- RGBSpotLight --
class RGBSpotLightDefinition;
using RGBSpotLightDefinitionPtr= std::shared_ptr<RGBSpotLightDefinition>;
using RGBSpotLightDefinitionConstPtr= std::shared_ptr<const RGBSpotLightDefinition>;
using RGBSpotLightDefinitionWeakPtr= std::weak_ptr<RGBSpotLightDefinition>;

class RGBSpotLightComponent;
using RGBSpotLightComponentPtr= std::shared_ptr<RGBSpotLightComponent>;
using RGBSpotLightComponentConstPtr= std::shared_ptr<const RGBSpotLightComponent>;
using RGBSpotLightComponentWeakPtr= std::weak_ptr<RGBSpotLightComponent>;

class RGBSpotLightSystemDefinition;
using RGBSpotLightSystemDefinitionPtr= std::shared_ptr<RGBSpotLightSystemDefinition>;
using RGBSpotLightSystemDefinitionConstPtr= std::shared_ptr<const RGBSpotLightSystemDefinition>;
using RGBSpotLightSystemDefinitionWeakPtr= std::weak_ptr<RGBSpotLightSystemDefinition>;

class RGBSpotLightSystem;
using RGBSpotLightSystemPtr= std::shared_ptr<RGBSpotLightSystem>;
using RGBSpotLightSystemConstPtr= std::shared_ptr<const RGBSpotLightSystem>;
using RGBSpotLightSystemWeakPtr= std::weak_ptr<RGBSpotLightSystem>;

// -- RGBPixelGrid --
class RGBPixelGridDefinition;
using RGBPixelGridDefinitionPtr= std::shared_ptr<RGBPixelGridDefinition>;
using RGBPixelGridDefinitionConstPtr= std::shared_ptr<const RGBPixelGridDefinition>;
using RGBPixelGridDefinitionWeakPtr= std::weak_ptr<RGBPixelGridDefinition>;

class RGBPixelGridComponent;
using RGBPixelGridComponentPtr= std::shared_ptr<RGBPixelGridComponent>;
using RGBPixelGridComponentConstPtr= std::shared_ptr<const RGBPixelGridComponent>;
using RGBPixelGridComponentWeakPtr= std::weak_ptr<RGBPixelGridComponent>;

class RGBPixelGridSystemDefinition;
using RGBPixelGridSystemDefinitionPtr= std::shared_ptr<RGBPixelGridSystemDefinition>;
using RGBPixelGridSystemDefinitionConstPtr= std::shared_ptr<const RGBPixelGridSystemDefinition>;
using RGBPixelGridSystemDefinitionWeakPtr= std::weak_ptr<RGBPixelGridSystemDefinition>;

class RGBPixelGridSystem;
using RGBPixelGridSystemPtr= std::shared_ptr<RGBPixelGridSystem>;
using RGBPixelGridSystemConstPtr= std::shared_ptr<const RGBPixelGridSystem>;
using RGBPixelGridSystemWeakPtr= std::weak_ptr<RGBPixelGridSystem>;

// -- DMXFixtureGroup --
class DMXFixtureGroupDefinition;
using DMXFixtureGroupDefinitionPtr= std::shared_ptr<DMXFixtureGroupDefinition>;
using DMXFixtureGroupDefinitionConstPtr= std::shared_ptr<const DMXFixtureGroupDefinition>;
using DMXFixtureGroupDefinitionWeakPtr= std::weak_ptr<DMXFixtureGroupDefinition>;

class DMXFixtureGroupComponent;
using DMXFixtureGroupComponentPtr= std::shared_ptr<DMXFixtureGroupComponent>;
using DMXFixtureGroupComponentConstPtr= std::shared_ptr<const DMXFixtureGroupComponent>;
using DMXFixtureGroupComponentWeakPtr= std::weak_ptr<DMXFixtureGroupComponent>;

class DMXFixtureGroupSystemDefinition;
using DMXFixtureGroupSystemDefinitionPtr= std::shared_ptr<DMXFixtureGroupSystemDefinition>;
using DMXFixtureGroupSystemDefinitionConstPtr= std::shared_ptr<const DMXFixtureGroupSystemDefinition>;
using DMXFixtureGroupSystemDefinitionWeakPtr= std::weak_ptr<DMXFixtureGroupSystemDefinition>;

class DMXFixtureGroupSystem;
using DMXFixtureGroupSystemPtr= std::shared_ptr<DMXFixtureGroupSystem>;
using DMXFixtureGroupSystemConstPtr= std::shared_ptr<const DMXFixtureGroupSystem>;
using DMXFixtureGroupSystemWeakPtr= std::weak_ptr<DMXFixtureGroupSystem>;

// -- DMXPreset --
class DMXPresetDefinition;
using DMXPresetDefinitionPtr= std::shared_ptr<DMXPresetDefinition>;
using DMXPresetDefinitionConstPtr= std::shared_ptr<const DMXPresetDefinition>;
using DMXPresetDefinitionWeakPtr= std::weak_ptr<DMXPresetDefinition>;

class DMXPresetComponent;
using DMXPresetComponentPtr= std::shared_ptr<DMXPresetComponent>;
using DMXPresetComponentConstPtr= std::shared_ptr<const DMXPresetComponent>;
using DMXPresetComponentWeakPtr= std::weak_ptr<DMXPresetComponent>;

class DMXPresetSystemDefinition;
using DMXPresetSystemDefinitionPtr= std::shared_ptr<DMXPresetSystemDefinition>;
using DMXPresetSystemDefinitionConstPtr= std::shared_ptr<const DMXPresetSystemDefinition>;
using DMXPresetSystemDefinitionWeakPtr= std::weak_ptr<DMXPresetSystemDefinition>;

class DMXPresetSystem;
using DMXPresetSystemPtr= std::shared_ptr<DMXPresetSystem>;
using DMXPresetSystemConstPtr= std::shared_ptr<const DMXPresetSystem>;
using DMXPresetSystemWeakPtr= std::weak_ptr<DMXPresetSystem>;

// -- DMXSequence --
class DMXSequenceDefinition;
using DMXSequenceDefinitionPtr= std::shared_ptr<DMXSequenceDefinition>;
using DMXSequenceDefinitionConstPtr= std::shared_ptr<const DMXSequenceDefinition>;
using DMXSequenceDefinitionWeakPtr= std::weak_ptr<DMXSequenceDefinition>;

class DMXSequenceComponent;
using DMXSequenceComponentPtr= std::shared_ptr<DMXSequenceComponent>;
using DMXSequenceComponentConstPtr= std::shared_ptr<const DMXSequenceComponent>;
using DMXSequenceComponentWeakPtr= std::weak_ptr<DMXSequenceComponent>;

class DMXSequenceSystemDefinition;
using DMXSequenceSystemDefinitionPtr= std::shared_ptr<DMXSequenceSystemDefinition>;
using DMXSequenceSystemDefinitionConstPtr= std::shared_ptr<const DMXSequenceSystemDefinition>;
using DMXSequenceSystemDefinitionWeakPtr= std::weak_ptr<DMXSequenceSystemDefinition>;

class DMXSequenceSystem;
using DMXSequenceSystemPtr= std::shared_ptr<DMXSequenceSystem>;
using DMXSequenceSystemConstPtr= std::shared_ptr<const DMXSequenceSystem>;
using DMXSequenceSystemWeakPtr= std::weak_ptr<DMXSequenceSystem>;
