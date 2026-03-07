import { computed } from 'vue'
import { useSettingsStore } from '../stores/settingsStore.js'

// Define which fields should be hidden in non-developer mode for each component type
const DEVELOPER_ONLY_FIELDS: Record<string, string[]> = {
  // Camera Component
  AnchorComponent: [
    'component_class',
    'component_script',
    'stage_id'
  ],
  BoxStencilComponent: [
    'component_class',
    'component_script',
  ],
  CameraComponent: [
    'component_class',
    'component_script',
    'aperture_orientation_offset',
    'aperture_position_offset',
    'component_script',
    'stage_id'
  ],
  CompositorComponent: [
    'component_class',
    'component_script',
    'owner_stage_id'
  ],
  MarkerComponent: [
    'component_class',
    'component_script',
  ],
  MarkerTrackingVolumeComponent: [
    'component_class',
    'component_script',
  ],
  ModelStencilComponent: [
    'component_class',
    'component_script',
  ],
  QuadStencilComponent: [
    'component_class',
    'component_script',
  ],
  SceneComponent: [
    'component_class',
    'relative_scale',
    'relative_rotation',
    'relative_position',
    'compositor_list',    
    'parent_stage_id'
  ],
  StageComponent: [
    'component_class',
    'component_script',
    'relative_scale',
    'relative_rotation',
    'relative_position'
  ],
  TrackingMountComponent: [
    'component_class',
    'component_script',
  ],
  VRTrackingVolumeComponent: [
    'component_class',
    'component_script',
    'vr_device_pose_offset'
  ],
  VRDeviceComponent: [
    'component_class',
    'component_script',
  ]
}

export function useDeveloperFields() {
  const settingsStore = useSettingsStore()

  /**
   * Check if a field should be displayed based on developer mode and component type
   */
  function shouldShowField(componentClass: string, fieldName: string): boolean {
    // If developer mode is enabled, show all fields
    if (settingsStore.developerMode) {
      return true
    }

    // Check if this field is developer-only for this component type
    const developerFields = DEVELOPER_ONLY_FIELDS[componentClass] || []
    const shouldHide = developerFields.includes(fieldName)

    // Debug logging
    if (componentClass === 'CameraComponent' && shouldHide) {
      console.log(`[useDeveloperFields] Hiding field "${fieldName}" for ${componentClass} (dev mode: ${settingsStore.developerMode})`)
    }

    return !shouldHide
  }

  /**
   * Filter component properties to only show appropriate fields
   */
  function filterComponentProperties(
    componentClass: string,
    properties: Record<string, any>
  ): Record<string, any> {
    const filtered: Record<string, any> = {}

    console.log(`[useDeveloperFields] Filtering properties for ${componentClass}, dev mode: ${settingsStore.developerMode}`)

    for (const [key, value] of Object.entries(properties)) {
      if (shouldShowField(componentClass, key)) {
        filtered[key] = value
      }
    }

    return filtered
  }

  return {
    shouldShowField,
    filterComponentProperties,
    isDeveloperMode: computed(() => settingsStore.developerMode)
  }
}
