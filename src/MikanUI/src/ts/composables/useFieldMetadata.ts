import {
  EnumRegistry,
  MikanAnchorComponentValues,
  MikanBoxStencilComponentValues,
  MikanCameraComponentValues,
  MikanClientTextureSourceValues,
  MikanCompositorComponentValues,
  MikanMarkerComponentValues,
  MikanMarkerTrackingVolumeComponentValues,
  MikanModelStencilComponentValues,
  MikanNetworkVideoSourceValues,
  MikanQuadStencilComponentValues,
  MikanSceneComponentValues,
  MikanSpoutTextureSourceValues,
  MikanStageComponentValues,
  MikanTrackingMountComponentValues,
  MikanUSBVideoSourceValues,
  MikanVRDeviceComponentValues,
  MikanVRTrackingVolumeComponentValues
} from '@mikanxr/client'

/**
 * Maps the API component class name (e.g. 'ModelStencilComponent') to the
 * corresponding TypeScript class that carries __serializationMetadata.
 */
const COMPONENT_CLASS_MAP: Record<string, any> = {
  AnchorComponent: MikanAnchorComponentValues,
  BoxStencilComponent: MikanBoxStencilComponentValues,
  CameraComponent: MikanCameraComponentValues,
  ClientTextureSourceComponent: MikanClientTextureSourceValues,
  CompositorComponent: MikanCompositorComponentValues,
  MarkerComponent: MikanMarkerComponentValues,
  MarkerTrackingVolumeComponent: MikanMarkerTrackingVolumeComponentValues,
  ModelStencilComponent: MikanModelStencilComponentValues,
  NetworkVideoSourceComponent: MikanNetworkVideoSourceValues,
  QuadStencilComponent: MikanQuadStencilComponentValues,
  SceneComponent: MikanSceneComponentValues,
  SpoutTextureSourceComponent: MikanSpoutTextureSourceValues,
  StageComponent: MikanStageComponentValues,
  TrackingMountComponent: MikanTrackingMountComponentValues,
  USBVideoSourceComponent: MikanUSBVideoSourceValues,
  VRDeviceComponent: MikanVRDeviceComponentValues,
  VRTrackingVolumeComponent: MikanVRTrackingVolumeComponentValues
}

/**
 * Collect serialization metadata from an entire class hierarchy by walking
 * up the prototype chain. Each class in the chain can declare its own
 * __serializationMetadata static array.
 */
function getAllFieldMetadata(cls: any): Array<{ name: string; type: string }> {
  const allMetadata: Array<{ name: string; type: string }> = []
  let current = cls
  while (current && current !== Function.prototype) {
    if (Object.prototype.hasOwnProperty.call(current, '__serializationMetadata')) {
      allMetadata.push(...current.__serializationMetadata)
    }
    current = Object.getPrototypeOf(current)
  }
  return allMetadata
}

export function useFieldMetadata() {
  /**
   * Look up the serialization type string for a field on a component class.
   * Returns values like 'boolean', 'float', 'enum:MikanStencilCullMode', etc.
   * Returns undefined when the component class or field is not found.
   */
  function getFieldMetaType(componentClassName: string, fieldName: string): string | undefined {
    const cls = COMPONENT_CLASS_MAP[componentClassName]
    if (!cls) return undefined
    const metadata = getAllFieldMetadata(cls)
    return metadata.find((m) => m.name === fieldName)?.type
  }

  /**
   * Returns label/value pairs for a registered enum type.
   * Pass the full meta type string like 'enum:MikanStencilCullMode'.
   */
  function getEnumOptions(metaType: string): { label: string; value: number }[] {
    if (!metaType.startsWith('enum:')) return []
    return EnumRegistry.getOptions(metaType.substring(5))
  }

  return { getFieldMetaType, getEnumOptions }
}
