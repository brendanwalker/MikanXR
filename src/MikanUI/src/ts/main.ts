import { createApp } from 'vue'
import { createPinia } from 'pinia'
import App from './App.vue'
import '../assets/common-buttons.css'
import {
  EnumRegistry,
  MikanAPIResult,
  MikanClientGraphicsApi,
  MikanColorBufferType,
  MikanCoreResult,
  MikanDepthBufferType,
  MikanDisconnectCode,
  MikanIntrinsicsType,
  MikanLogLevel,
  MikanMarkerDictionaryType,
  MikanPropertyNotifyMode,
  MikanStageTrackingVolume,
  MikanStencilCullMode,
  MikanTrackingRuntime,
  MikanTrackingVolumeType,
  MikanVariantType,
  MikanVideoSettingType,
  MikanVideoSourceType,
  MikanVRDeviceApi,
  MikanVRDeviceType
} from '@mikanxr/client'

// Register all enum types so the serialization layer can convert
// string enum names (e.g. "NONE") ↔ integer values (e.g. 0).
EnumRegistry.register('MikanAPIResult', MikanAPIResult)
EnumRegistry.register('MikanClientGraphicsApi', MikanClientGraphicsApi)
EnumRegistry.register('MikanColorBufferType', MikanColorBufferType)
EnumRegistry.register('MikanCoreResult', MikanCoreResult)
EnumRegistry.register('MikanDepthBufferType', MikanDepthBufferType)
EnumRegistry.register('MikanDisconnectCode', MikanDisconnectCode)
EnumRegistry.register('MikanIntrinsicsType', MikanIntrinsicsType)
EnumRegistry.register('MikanLogLevel', MikanLogLevel)
EnumRegistry.register('MikanMarkerDictionaryType', MikanMarkerDictionaryType)
EnumRegistry.register('MikanPropertyNotifyMode', MikanPropertyNotifyMode)
EnumRegistry.register('MikanStageTrackingVolume', MikanStageTrackingVolume)
EnumRegistry.register('MikanStencilCullMode', MikanStencilCullMode)
EnumRegistry.register('MikanTrackingRuntime', MikanTrackingRuntime)
EnumRegistry.register('MikanTrackingVolumeType', MikanTrackingVolumeType)
EnumRegistry.register('MikanVariantType', MikanVariantType)
EnumRegistry.register('MikanVideoSettingType', MikanVideoSettingType)
EnumRegistry.register('MikanVideoSourceType', MikanVideoSourceType)
EnumRegistry.register('MikanVRDeviceApi', MikanVRDeviceApi)
EnumRegistry.register('MikanVRDeviceType', MikanVRDeviceType)

const app = createApp(App)
const pinia = createPinia()

app.use(pinia)
app.mount('#app')
