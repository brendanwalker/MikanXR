// This file is auto generated. DO NOT EDIT.

import { EnumRegistry } from '../Serialization/EnumRegistry.js';
import { MikanAPIResult } from './MikanAPITypes.js';
import { MikanClientGraphicsApi, MikanColorBufferType, MikanConstants, MikanCoreResult, MikanDepthBufferType, MikanDisconnectCode, MikanLogLevel } from './MikanCoreConstants.js';
import { MikanMarkerDictionaryType } from './MikanMarkerTypes.js';
import { MikanPropertyNotifyMode } from './MikanPropertyRequests.js';
import { MikanStageTrackingVolume } from './MikanStageTypes.js';
import { MikanStencilCullMode } from './MikanStencilTypes.js';
import { MikanTrackingRuntime, MikanTrackingVolumeType } from './MikanTrackingVolumeTypes.js';
import { MikanVRDeviceApi, MikanVRDeviceType } from './MikanVRDeviceTypes.js';
import { MikanVariantType } from './MikanVariantTypes.js';
import { MikanIntrinsicsType, MikanVideoSettingType, MikanVideoSourceType } from './MikanVideoSourceTypes.js';

export function registerAllEnums(): void {
  EnumRegistry.register('MikanAPIResult', MikanAPIResult);
  EnumRegistry.register('MikanClientGraphicsApi', MikanClientGraphicsApi);
  EnumRegistry.register('MikanColorBufferType', MikanColorBufferType);
  EnumRegistry.register('MikanConstants', MikanConstants);
  EnumRegistry.register('MikanCoreResult', MikanCoreResult);
  EnumRegistry.register('MikanDepthBufferType', MikanDepthBufferType);
  EnumRegistry.register('MikanDisconnectCode', MikanDisconnectCode);
  EnumRegistry.register('MikanLogLevel', MikanLogLevel);
  EnumRegistry.register('MikanMarkerDictionaryType', MikanMarkerDictionaryType);
  EnumRegistry.register('MikanPropertyNotifyMode', MikanPropertyNotifyMode);
  EnumRegistry.register('MikanStageTrackingVolume', MikanStageTrackingVolume);
  EnumRegistry.register('MikanStencilCullMode', MikanStencilCullMode);
  EnumRegistry.register('MikanTrackingRuntime', MikanTrackingRuntime);
  EnumRegistry.register('MikanTrackingVolumeType', MikanTrackingVolumeType);
  EnumRegistry.register('MikanVRDeviceApi', MikanVRDeviceApi);
  EnumRegistry.register('MikanVRDeviceType', MikanVRDeviceType);
  EnumRegistry.register('MikanVariantType', MikanVariantType);
  EnumRegistry.register('MikanIntrinsicsType', MikanIntrinsicsType);
  EnumRegistry.register('MikanVideoSettingType', MikanVideoSettingType);
  EnumRegistry.register('MikanVideoSourceType', MikanVideoSourceType);
}

// Auto-register when this module is first imported.
// Because this file is re-exported from types/index.ts → bindings/index.ts,
// this call runs automatically the moment any symbol from @mikanxr/client is imported.
registerAllEnums();
