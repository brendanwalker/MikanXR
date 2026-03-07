import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import {
  MikanClient,
  MikanAPIResult,
  GetComponentListRequest,
  CLASS_ID_GET_COMPONENT_LIST_REQUEST,
  ComponentListResponse,
  ComponentGetValuesRequest,
  CLASS_ID_COMPONENT_GET_VALUES_REQUEST,
  ComponentGetValuesResponse,
  MikanPropertyUpdateEvent,
  MikanComponentValues,
  MikanPropertyValue,
  MikanVariant,
  MikanConstants,
  MikanAnchorComponentValues,
  MikanCameraComponentValues,
  MikanCompositorComponentValues,
  MikanMarkerComponentValues,
  MikanSceneComponentValues,
  MikanStageComponentValues,
  MikanQuadStencilComponentValues,
  MikanBoxStencilSystemValues,
  MikanModelStencilSystemValues,
  MikanTrackingMountComponentValues,
  MikanTrackingVolumeComponentValues,
  MikanVRTrackingVolumeComponentValues,
  MikanVRDeviceComponentValues
} from '@mikanxr/client'

// Component registry mapping system names to component class names
const COMPONENT_SYSTEMS = [
  { ownerSystem: 'AnchorObjectSystem', componentClassName: 'AnchorComponent' },
  { ownerSystem: 'CameraObjectSystem', componentClassName: 'CameraComponent' },
  { ownerSystem: 'CompositorObjectSystem', componentClassName: 'CompositorComponent' },
  { ownerSystem: 'MarkerObjectSystem', componentClassName: 'MarkerComponent' },
  { ownerSystem: 'SceneObjectSystem', componentClassName: 'SceneComponent' },
  { ownerSystem: 'StageObjectSystem', componentClassName: 'StageComponent' },
  { ownerSystem: 'QuadStencilSystem', componentClassName: 'QuadStencilComponent' },
  { ownerSystem: 'BoxStencilSystem', componentClassName: 'BoxStencilComponent' },
  { ownerSystem: 'ModelStencilSystem', componentClassName: 'ModelStencilComponent' },
  { ownerSystem: 'TrackingMountObjectSystem', componentClassName: 'TrackingMountComponent' },
  { ownerSystem: 'MarkerTrackingVolumeSystem', componentClassName: 'MarkerTrackingVolumeComponent' },
  { ownerSystem: 'VRTrackingVolumeSystem', componentClassName: 'VRTrackingVolumeComponent' },
  { ownerSystem: 'VRObjectSystem', componentClassName: 'VRDeviceComponent' }
]

// Helper to create a unique key for each component (system:id)
function makeComponentKey(systemName: string, componentId: number): string {
  return `${systemName}:${componentId}`
}

export const useComponentStore = defineStore('components', () => {
  // Component database - maps composite key (system:id) to component values
  const components = ref<Map<string, MikanComponentValues>>(new Map())

  // Component indices by system for quick lookups
  const componentsBySystem = ref<Map<string, Set<string>>>(new Map())

  // Component indices by class name
  const componentsByClass = ref<Map<string, Set<string>>>(new Map())

  // Computed getters
  const allComponents = computed(() => Array.from(components.value.values()))

  const getComponentsBySystem = computed(() => (systemName: string) => {
    const ids = componentsBySystem.value.get(systemName)
    if (!ids) return []
    return Array.from(ids)
      .map(id => components.value.get(id))
      .filter((c): c is MikanComponentValues => c !== undefined)
  })

  const getComponentsByClass = computed(() => (className: string) => {
    const ids = componentsByClass.value.get(className)
    console.log(`[ComponentStore] getComponentsByClass("${className}"): found ${ids?.size || 0} component IDs`)
    if (!ids) return []
    const result = Array.from(ids)
      .map(id => components.value.get(id))
      .filter((c): c is MikanComponentValues => c !== undefined)
    console.log(`[ComponentStore] Returning ${result.length} components for class "${className}"`)
    return result
  })

  // Actions
  function getComponent(componentId: number, systemName: string): MikanComponentValues | undefined {
    const key = makeComponentKey(systemName, componentId)
    return components.value.get(key)
  }

  // Typed component getters for each system
  function getAnchorComponent(componentId: number): MikanAnchorComponentValues | undefined {
    return getComponent(componentId, 'AnchorObjectSystem') as MikanAnchorComponentValues | undefined
  }

  function getCameraComponent(componentId: number): MikanCameraComponentValues | undefined {
    return getComponent(componentId, 'CameraObjectSystem') as MikanCameraComponentValues | undefined
  }

  function getCompositorComponent(componentId: number): MikanCompositorComponentValues | undefined {
    return getComponent(componentId, 'CompositorObjectSystem') as MikanCompositorComponentValues | undefined
  }

  function getMarkerComponent(componentId: number): MikanMarkerComponentValues | undefined {
    return getComponent(componentId, 'MarkerObjectSystem') as MikanMarkerComponentValues | undefined
  }

  function getSceneComponent(componentId: number): MikanSceneComponentValues | undefined {
    return getComponent(componentId, 'SceneObjectSystem') as MikanSceneComponentValues | undefined
  }

  function getStageComponent(componentId: number): MikanStageComponentValues | undefined {
    return getComponent(componentId, 'StageObjectSystem') as MikanStageComponentValues | undefined
  }

  function getQuadStencilComponent(componentId: number): MikanQuadStencilComponentValues | undefined {
    return getComponent(componentId, 'QuadStencilSystem') as MikanQuadStencilComponentValues | undefined
  }

  function getBoxStencilComponent(componentId: number): MikanBoxStencilSystemValues | undefined {
    return getComponent(componentId, 'BoxStencilSystem') as MikanBoxStencilSystemValues | undefined
  }

  function getModelStencilComponent(componentId: number): MikanModelStencilSystemValues | undefined {
    return getComponent(componentId, 'ModelStencilSystem') as MikanModelStencilSystemValues | undefined
  }

  function getTrackingMountComponent(componentId: number): MikanTrackingMountComponentValues | undefined {
    return getComponent(componentId, 'TrackingMountObjectSystem') as MikanTrackingMountComponentValues | undefined
  }

  function getMarkerTrackingVolumeComponent(componentId: number): MikanTrackingVolumeComponentValues | undefined {
    return getComponent(componentId, 'MarkerTrackingVolumeSystem') as MikanTrackingVolumeComponentValues | undefined
  }

  function getVRTrackingVolumeComponent(componentId: number): MikanVRTrackingVolumeComponentValues | undefined {
    return getComponent(componentId, 'VRTrackingVolumeSystem') as MikanVRTrackingVolumeComponentValues | undefined
  }

  function getVRDeviceComponent(componentId: number): MikanVRDeviceComponentValues | undefined {
    return getComponent(componentId, 'VRObjectSystem') as MikanVRDeviceComponentValues | undefined
  }

  function addComponent(
    componentId: number,
    component: MikanComponentValues,
    systemName: string,
    className: string
  ) {
    const key = makeComponentKey(systemName, componentId)

    // Augment component with class name for easier filtering
    ;(component as any).component_class = className

    components.value.set(key, component)

    // Add to system index
    if (!componentsBySystem.value.has(systemName)) {
      componentsBySystem.value.set(systemName, new Set())
    }
    componentsBySystem.value.get(systemName)!.add(key)

    // Add to class index
    if (!componentsByClass.value.has(className)) {
      componentsByClass.value.set(className, new Set())
    }
    componentsByClass.value.get(className)!.add(key)
  }

  function removeComponent(componentId: number, systemName: string) {
    const key = makeComponentKey(systemName, componentId)
    const component = components.value.get(key)
    if (!component) return

    // Remove from all indices
    componentsBySystem.value.forEach((ids) => ids.delete(key))
    componentsByClass.value.forEach((ids) => ids.delete(key))

    // Remove from main map
    components.value.delete(key)
  }

  function updateComponentProperty(
    componentId: number,
    systemName: string,
    fieldName: string,
    fieldValue: any
  ) {
    const key = makeComponentKey(systemName, componentId)
    const component = components.value.get(key)
    if (!component) return

    // Update the property value
    ;(component as any)[fieldName] = fieldValue
  }

  function clearComponents() {
    components.value.clear()
    componentsBySystem.value.clear()
    componentsByClass.value.clear()
  }

  // Fetch components from server
  async function fetchComponentList(
    client: MikanClient,
    ownerSystem: string,
    componentClassName: string
  ): Promise<void> {
    try {
      const request: GetComponentListRequest = {
        requestTypeId: CLASS_ID_GET_COMPONENT_LIST_REQUEST,
        requestTypeName: 'GetComponentListRequest',
        requestId: 0,
        ownerSystem,
        componentClassName
      }

      const future = client.sendRequest(request)
      const response = await future.await()

      if (response.resultCode === MikanAPIResult.Success) {
        const listResponse = response as ComponentListResponse
        const componentIds = listResponse.componentIdList

        console.log(`[ComponentStore] Fetched ${componentIds.length} ${componentClassName} components`)

        // Fetch values for each component
        for (const componentId of componentIds) {
          await fetchComponentValues(client, ownerSystem, componentId, componentClassName)
        }
      } else if (response.resultCode === MikanAPIResult.Uninitialized) {
        // Component type doesn't exist in this version - silently skip
        console.log(`[ComponentStore] Component type ${componentClassName} not available (skipped)`)
      } else {
        console.warn(
          `[ComponentStore] Failed to fetch component list for ${componentClassName}: ${response.resultCode}`
        )
      }
    } catch (error) {
      console.error(`[ComponentStore] Error fetching component list for ${componentClassName}:`, error)
    }
  }

  async function fetchComponentValues(
    client: MikanClient,
    ownerSystem: string,
    componentId: number,
    componentClassName: string
  ): Promise<void> {
    try {
      const request: ComponentGetValuesRequest = {
        requestTypeId: CLASS_ID_COMPONENT_GET_VALUES_REQUEST,
        requestTypeName: 'ComponentGetValuesRequest',
        requestId: 0,
        ownerSystem,
        componentId
      }

      const future = client.sendRequest(request)
      const response = await future.await()

      if (response.resultCode === MikanAPIResult.Success) {
        const valuesResponse = response as ComponentGetValuesResponse

        if (!valuesResponse.valuesObject) {
          console.error(`[ComponentStore] No valuesObject in response for component ${componentId}`)
          return
        }

        // The PolymorphicObject structure has the actual data in the 'value' property
        const polymorphicObj = valuesResponse.valuesObject as any

        // Try to get component values from either .instance, .value, or directly on the object
        let componentValues: MikanComponentValues | null = null

        if (polymorphicObj.instance) {
          componentValues = polymorphicObj.instance as MikanComponentValues
        } else if (polymorphicObj.value) {
          // This is the expected path for PolymorphicObject
          componentValues = polymorphicObj.value as MikanComponentValues
        } else if (polymorphicObj.component_name !== undefined) {
          // Fallback: data might be directly on the object
          componentValues = polymorphicObj as MikanComponentValues
        }

        if (!componentValues) {
          console.error(`[ComponentStore] Could not find component data for component ${componentId}`)
          console.log(`[ComponentStore] valuesObject keys:`, Object.keys(polymorphicObj))
          return
        }

        console.log(
          `[ComponentStore] Adding component ${componentId}: class="${componentClassName}", system="${ownerSystem}", name="${componentValues.component_name}"`
        )
        addComponent(componentId, componentValues, ownerSystem, componentClassName)
        console.log(
          `[ComponentStore] Successfully added component ${componentId} (${componentClassName}): ${componentValues.component_name}`
        )
      } else {
        console.error(
          `[ComponentStore] Failed to fetch component values for ${componentId}: ${response.resultCode}`
        )
      }
    } catch (error) {
      console.error(`[ComponentStore] Error fetching component values for ${componentId}:`, error)
    }
  }

  async function fetchAllComponents(client: MikanClient): Promise<void> {
    console.log('[ComponentStore] Fetching all components...')

    const initialCount = components.value.size

    for (const { ownerSystem, componentClassName } of COMPONENT_SYSTEMS) {
      await fetchComponentList(client, ownerSystem, componentClassName)
    }

    const fetchedCount = components.value.size - initialCount
    console.log(`[ComponentStore] Component database populated with ${fetchedCount} new components (${components.value.size} total)`)
  }

  // Handle property update events from the server
  function handlePropertyUpdate(event: MikanPropertyUpdateEvent) {
    const propertyValue: MikanPropertyValue = event.propertyValue
    const componentId = propertyValue.componentId
    const ownerSystem = propertyValue.ownerSystem

    // Only handle component property updates (not system properties)
    if (componentId === MikanConstants.InvalidMikanID) {
      return
    }

    const component = getComponent(componentId, ownerSystem)
    if (!component) {
      console.warn(`[ComponentStore] Component ${componentId} in system ${ownerSystem} not found for property update`)
      return
    }

    const fieldName = propertyValue.fieldName
    const fieldValue = propertyValue.fieldValue

    // Extract the actual value from the variant
    const actualValue = extractVariantValue(fieldValue)

    console.log(
      `[ComponentStore] Component ${componentId} (${ownerSystem}) property update: ${fieldName} = ${JSON.stringify(actualValue)}`
    )

    // Update the component property
    updateComponentProperty(componentId, ownerSystem, fieldName, actualValue)
  }

  // Helper to extract value from MikanVariant
  function extractVariantValue(variant: MikanVariant): any {
    if (!variant.value_ptr?.instance) {
      return null
    }

    const value = variant.value_ptr.instance

    // Handle different value types
    if ('value' in value) {
      return (value as any).value
    }

    return value
  }

  // Helper to map field names to component classes for component references
  function getComponentClassForField(fieldName: string): string | string[] | null {
    const fieldMappings: Record<string, string | string[]> = {
      'camera_id': 'CameraComponent',
      'parent_stage_id': 'StageComponent',
      'stage_id': 'StageComponent',
      'display_compositor_id': 'CompositorComponent',
      'tracking_volume_id': ['MarkerTrackingVolumeComponent', 'VRTrackingVolumeComponent'],
      'video_source_id': 'VideoSourceComponent',
      'tracking_mount_id': 'TrackingMountComponent',
      'charuco_mount_id': 'TrackingMountComponent',
      'utility_marker_id': 'MarkerComponent',
      'origin_marker_id': 'MarkerComponent'
    }

    return fieldMappings[fieldName] || null
  }

  // Helper to get component name by ID
  function getComponentName(componentId: number, systemName: string): string {
    if (componentId === -1 || componentId === MikanConstants.InvalidMikanID) {
      return '<None>'
    }

    const component = getComponent(componentId, systemName)
    if (component) {
      return component.component_name || `Component ${componentId}`
    }

    return `Unknown (${componentId})`
  }

  return {
    // State
    components,
    componentsBySystem,
    componentsByClass,

    // Computed
    allComponents,
    getComponentsBySystem,
    getComponentsByClass,

    // Actions
    getComponent,
    addComponent,
    removeComponent,
    updateComponentProperty,
    clearComponents,
    fetchComponentList,
    fetchComponentValues,
    fetchAllComponents,
    handlePropertyUpdate,

    // Typed component getters
    getAnchorComponent,
    getCameraComponent,
    getCompositorComponent,
    getMarkerComponent,
    getSceneComponent,
    getStageComponent,
    getQuadStencilComponent,
    getBoxStencilComponent,
    getModelStencilComponent,
    getTrackingMountComponent,
    getMarkerTrackingVolumeComponent,
    getVRTrackingVolumeComponent,
    getVRDeviceComponent,

    // Helpers
    getComponentClassForField,
    getComponentName
  }
})
