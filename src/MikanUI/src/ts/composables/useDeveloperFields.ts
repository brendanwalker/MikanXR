import { computed } from 'vue'
import { useSettingsStore } from '../stores/settingsStore.js'

// Define which fields should be hidden in non-developer mode for each component type
const DEVELOPER_ONLY_FIELDS: Record<string, string[]> = {
  // Camera Component
  AnchorComponent: [
    'component_class',
    'component_script',
    'parent_transform_id'
  ],
  BoxStencilComponent: [
    'component_class',
    'component_script',
    'parent_transform_id'
  ],
  CameraComponent: [
    'component_class',
    'component_script',
    'aperture_orientation_offset',
    'aperture_position_offset',
    'component_script',
    'stage_id',
    'parent_transform_id'
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
    'parent_transform_id'
  ],
  SceneComponent: [
    'component_class',
    'relative_scale',
    'relative_rotation',
    'relative_position',
    'compositor_list',    
    'parent_transform_id',
  ],
  StageComponent: [
    'component_class',
    'component_script',
    'relative_scale',
    'relative_rotation',
    'relative_position',
    'parent_transform_id'
  ],
  TrackingMountComponent: [
    'component_class',
    'component_script',
    'available_socket_names'
  ],
  VRTrackingVolumeComponent: [
    'component_class',
    'component_script',
    'vr_device_pose_offset',
    'tracking_mount_ids'
  ],
  VRDeviceComponent: [
    'component_class',
    'component_script',
  ],
  USBVideoSourceComponent: [
    'component_script',
    'current_device_path',
    'intrinsics_ptr',
    'intrinsics_type',
    'video_settings',
    'component_class',
    'brightness_valid',
    'brightness_fraction',
    'contrast_valid',
    'contrast_fraction',
    'hue_valid',
    'hue_fraction',
    'saturation_valid',
    'saturation_fraction',
    'sharpness_valid',
    'sharpness_fraction',
    'gamma_valid',
    'gamma_fraction',
    'white_balance_valid',
    'white_balance_fraction',
    'red_balance_valid',
    'red_balance_fraction',
    'green_balance_valid',
    'green_balance_fraction',
    'blue_balance_valid',
    'blue_balance_fraction',
    'gain_valid',
    'gain_fraction',
    'pan_valid',
    'pan_fraction',
    'tilt_valid',
    'tilt_fraction',
    'roll_valid',
    'roll_fraction',
    'zoom_valid',
    'zoom_fraction',
    'exposure_valid',
    'exposure_fraction',
    'iris_valid',
    'iris_fraction',
    'focus_valid',
    'focus_fraction',
    'is_buffer_mirrored',
    'is_frame_mirrored',
    'video_format',
    'video_formats',
    'video_fps',
    'video_frame_rates',
    'video_resolution',
    'video_resolutions',
    'video_frame_queue_size'
  ],
  NetworkVideoSourceComponent: [
    'component_script',
    'component_class'
  ],
  SpoutTextureSourceComponent: [
    'component_script',
    'component_class'
  ],
  ClientTextureSourceComponent: [
    'component_script',
    'component_class'
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
