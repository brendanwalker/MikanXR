import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import {
  MikanClient,
  MikanAPIResult,
  GetComponentListRequest,
  ComponentListResponse,
  ComponentGetValuesRequest,
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
  MikanVRDeviceComponentValues,
  MikanUSBVideoSourceValues,
  MikanNetworkVideoSourceValues,
  MikanClientTextureSourceValues,
  MikanSpoutTextureSourceValues,
  GetFunctionListRequest,
  FunctionDescriptorResponse,
  InvokeComponentFunctionRequest,
  MikanFunctionDescriptor,
  PolymorphicObject,
  SystemCreateObjectRequest,
  SystemDestroyObjectRequest,
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
  { ownerSystem: 'VRObjectSystem', componentClassName: 'VRDeviceComponent' },
  { ownerSystem: 'USBVideoSourceSystem', componentClassName: 'USBVideoSourceComponent' },
  { ownerSystem: 'NetworkVideoObjectSystem', componentClassName: 'NetworkVideoSourceComponent' },
  { ownerSystem: 'SpoutTextureSourceSystem', componentClassName: 'SpoutTextureSourceComponent' },
  { ownerSystem: 'ClientTextureSourceSystem', componentClassName: 'ClientTextureSourceComponent' }
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

  // Function database - maps component key (system:id) to list of function descriptors
  const componentFunctions = ref<Map<string, MikanFunctionDescriptor[]>>(new Map())

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

  function getUSBVideoSourceComponent(componentId: number): MikanUSBVideoSourceValues | undefined {
    return getComponent(componentId, 'USBVideoSourceSystem') as MikanUSBVideoSourceValues | undefined
  }

  function getNetworkVideoSourceComponent(componentId: number): MikanNetworkVideoSourceValues | undefined {
    return getComponent(componentId, 'NetworkVideoObjectSystem') as MikanNetworkVideoSourceValues | undefined
  }

  function getSpoutTextureSourceComponent(componentId: number): MikanSpoutTextureSourceValues | undefined {
    return getComponent(componentId, 'SpoutTextureSourceSystem') as MikanSpoutTextureSourceValues | undefined
  }

  function getClientTextureSourceComponent(componentId: number): MikanClientTextureSourceValues | undefined {
    return getComponent(componentId, 'ClientTextureSourceSystem') as MikanClientTextureSourceValues | undefined
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
    componentFunctions.value.clear()
  }

  // Fetch components from server
  async function fetchComponentList(
    client: MikanClient,
    ownerSystem: string,
    componentClassName: string
  ): Promise<void> {
    try {
      const request: GetComponentListRequest = {
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

  // Handle system-level property updates (componentId === -1)
  // e.g. MarkerComponentIdList changing when a marker is added/removed
  async function handleSystemPropertyUpdate(
    client: MikanClient,
    fieldName: string,
    fieldValue: MikanVariant
  ): Promise<void> {
    const entry = COMPONENT_SYSTEMS.find(s => fieldName === `${s.componentClassName}IdList`)
    if (!entry) return

    const newIds: number[] = extractVariantValue(fieldValue) || []
    if (!Array.isArray(newIds)) {
      console.warn(`[ComponentStore] Expected array for ${fieldName}, got:`, newIds)
      return
    }
    const newIdSet = new Set(newIds)

    // Determine which IDs currently exist for this system
    const existingComponents = getComponentsBySystem.value(entry.ownerSystem)
    const existingIdSet = new Set(existingComponents.map(c => (c as any).component_id as number))

    // Remove stale components (present locally but not in server list)
    for (const existingId of existingIdSet) {
      if (!newIdSet.has(existingId)) {
        console.log(`[ComponentStore] Removing stale ${entry.componentClassName} ${existingId}`)
        removeComponent(existingId, entry.ownerSystem)
      }
    }

    // Fetch values for new components (in server list but not locally)
    for (const newId of newIds) {
      if (!existingIdSet.has(newId)) {
        console.log(`[ComponentStore] Fetching new ${entry.componentClassName} ${newId}`)
        await fetchComponentValues(client, entry.ownerSystem, newId, entry.componentClassName)
      }
    }
  }

  // Handle property update events from the server
  function handlePropertyUpdate(event: MikanPropertyUpdateEvent, client?: MikanClient) {
    const propertyValue: MikanPropertyValue = event.propertyValue
    const componentId = propertyValue.componentId
    const ownerSystem = propertyValue.ownerSystem

    // System-level property updates have componentId === -1
    if (componentId === MikanConstants.InvalidMikanID) {
      if (client) {
        handleSystemPropertyUpdate(client, propertyValue.fieldName, propertyValue.fieldValue)
          .catch(err => console.error('[ComponentStore] Error handling system property update:', err))
      }
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
    if (!variant.value_ptr) {
      return null
    }

    // Try .value first (standard path), then fall back to .instance
    const valueObj = (variant.value_ptr as any).value || (variant.value_ptr as any).instance
    if (!valueObj) {
      return null
    }

    // Handle different value types
    if ('value' in valueObj) {
      return valueObj.value
    }

    return valueObj
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

  // Fetch function list for a specific component
  async function fetchComponentFunctions(
    client: MikanClient,
    ownerSystem: string,
    componentClassName: string,
    componentId: number
  ): Promise<MikanFunctionDescriptor[]> {
    try {
      const request: GetFunctionListRequest = {
        requestTypeName: 'GetFunctionListRequest',
        requestId: 0,
        systemFilter: ownerSystem,
        componentFilter: componentClassName
      }

      const future = client.sendRequest(request)
      const response = await future.await()

      if (response.resultCode === MikanAPIResult.Success) {
        const functionResponse = response as FunctionDescriptorResponse
        const functions = functionResponse.descriptor_list || []

        // Cache the functions for this component
        const key = makeComponentKey(ownerSystem, componentId)
        componentFunctions.value.set(key, functions)

        console.log(
          `[ComponentStore] Fetched ${functions.length} functions for ${componentClassName} (${componentId})`
        )

        return functions
      } else {
        console.warn(
          `[ComponentStore] Failed to fetch functions for ${componentClassName}: ${response.resultCode}`
        )
        return []
      }
    } catch (error) {
      console.error(
        `[ComponentStore] Error fetching functions for ${componentClassName}:`,
        error
      )
      return []
    }
  }

  // Get cached functions for a component
  function getComponentFunctions(componentId: number, systemName: string): MikanFunctionDescriptor[] {
    const key = makeComponentKey(systemName, componentId)
    return componentFunctions.value.get(key) || []
  }

  // Create a new object in the given component system
  async function createObject(
    client: MikanClient,
    componentClassName: string,
    initParams: PolymorphicObject = new PolymorphicObject()
  ): Promise<boolean> {
    const entry = COMPONENT_SYSTEMS.find(s => s.componentClassName === componentClassName)
    if (!entry) {
      console.error(`[ComponentStore] No system found for component class "${componentClassName}"`)
      return false
    }
    try {
      const request: SystemCreateObjectRequest = {
        requestTypeName: 'SystemCreateObjectRequest',
        requestId: 0,
        ownerSystem: entry.ownerSystem,
        componentClassName,
        initParams
      }
      const future = client.sendRequest(request)
      const response = await future.await()
      if (response.resultCode === MikanAPIResult.Success) {
        console.log(`[ComponentStore] Created ${componentClassName} in ${entry.ownerSystem}`)
        await fetchComponentList(client, entry.ownerSystem, componentClassName)
        return true
      } else {
        console.error(`[ComponentStore] Failed to create ${componentClassName}: ${response.resultCode}`)
        return false
      }
    } catch (error) {
      console.error(`[ComponentStore] Error creating ${componentClassName}:`, error)
      return false
    }
  }

  // Destroy an existing object in the given component system
  async function destroyObject(
    client: MikanClient,
    componentClassName: string,
    componentId: number
  ): Promise<boolean> {
    const entry = COMPONENT_SYSTEMS.find(s => s.componentClassName === componentClassName)
    if (!entry) {
      console.error(`[ComponentStore] No system found for component class "${componentClassName}"`)
      return false
    }
    try {
      const request: SystemDestroyObjectRequest = {
        requestTypeName: 'SystemDestroyObjectRequest',
        requestId: 0,
        ownerSystem: entry.ownerSystem,
        componentClassName,
        componentId
      }
      const future = client.sendRequest(request)
      const response = await future.await()
      if (response.resultCode === MikanAPIResult.Success) {
        console.log(`[ComponentStore] Destroyed ${componentClassName} ${componentId} in ${entry.ownerSystem}`)
        removeComponent(componentId, entry.ownerSystem)
        return true
      } else {
        console.error(`[ComponentStore] Failed to destroy ${componentClassName} ${componentId}: ${response.resultCode}`)
        return false
      }
    } catch (error) {
      console.error(`[ComponentStore] Error destroying ${componentClassName} ${componentId}:`, error)
      return false
    }
  }

  // Invoke a function on a component
  async function invokeComponentFunction(
    client: MikanClient,
    ownerSystem: string,
    componentId: number,
    functionName: string
  ): Promise<boolean> {
    try {
      const request: InvokeComponentFunctionRequest = {
        requestTypeName: 'InvokeComponentFunctionRequest',
        requestId: 0,
        ownerSystem,
        componentId,
        functionName
      }

      const future = client.sendRequest(request)
      const response = await future.await()

      if (response.resultCode === MikanAPIResult.Success) {
        console.log(
          `[ComponentStore] Successfully invoked function "${functionName}" on component ${componentId}`
        )
        return true
      } else {
        console.error(
          `[ComponentStore] Failed to invoke function "${functionName}" on component ${componentId}: ${response.resultCode}`
        )
        return false
      }
    } catch (error) {
      console.error(
        `[ComponentStore] Error invoking function "${functionName}" on component ${componentId}:`,
        error
      )
      return false
    }
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
    getUSBVideoSourceComponent,
    getNetworkVideoSourceComponent,
    getSpoutTextureSourceComponent,
    getClientTextureSourceComponent,

    // Helpers
    getComponentClassForField,
    getComponentName,

    // Object lifecycle
    createObject,
    destroyObject,

    // Function management
    fetchComponentFunctions,
    getComponentFunctions,
    invokeComponentFunction
  }
})
