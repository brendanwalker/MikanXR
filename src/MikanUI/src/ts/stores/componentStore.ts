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
  MikanConstants
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

export const useComponentStore = defineStore('components', () => {
  // Component database - maps component ID to component values
  const components = ref<Map<number, MikanComponentValues>>(new Map())

  // Component indices by system for quick lookups
  const componentsBySystem = ref<Map<string, Set<number>>>(new Map())

  // Component indices by class name
  const componentsByClass = ref<Map<string, Set<number>>>(new Map())

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
    if (!ids) return []
    return Array.from(ids)
      .map(id => components.value.get(id))
      .filter((c): c is MikanComponentValues => c !== undefined)
  })

  // Actions
  function getComponent(componentId: number): MikanComponentValues | undefined {
    return components.value.get(componentId)
  }

  function addComponent(
    componentId: number,
    component: MikanComponentValues,
    systemName: string,
    className: string
  ) {
    components.value.set(componentId, component)

    // Add to system index
    if (!componentsBySystem.value.has(systemName)) {
      componentsBySystem.value.set(systemName, new Set())
    }
    componentsBySystem.value.get(systemName)!.add(componentId)

    // Add to class index
    if (!componentsByClass.value.has(className)) {
      componentsByClass.value.set(className, new Set())
    }
    componentsByClass.value.get(className)!.add(componentId)
  }

  function removeComponent(componentId: number) {
    const component = components.value.get(componentId)
    if (!component) return

    // Remove from all indices
    componentsBySystem.value.forEach((ids) => ids.delete(componentId))
    componentsByClass.value.forEach((ids) => ids.delete(componentId))

    // Remove from main map
    components.value.delete(componentId)
  }

  function updateComponentProperty(
    componentId: number,
    fieldName: string,
    fieldValue: any
  ) {
    const component = components.value.get(componentId)
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
        const componentValues = valuesResponse.valuesObject.instance as MikanComponentValues

        if (componentValues) {
          addComponent(componentId, componentValues, ownerSystem, componentClassName)
          console.log(
            `[ComponentStore] Added component ${componentId} (${componentClassName}): ${componentValues.component_name}`
          )
        }
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

    // Only handle component property updates (not system properties)
    if (componentId === MikanConstants.InvalidMikanID) {
      return
    }

    const component = components.value.get(componentId)
    if (!component) {
      console.warn(`[ComponentStore] Component ${componentId} not found for property update`)
      return
    }

    const fieldName = propertyValue.fieldName
    const fieldValue = propertyValue.fieldValue

    // Extract the actual value from the variant
    const actualValue = extractVariantValue(fieldValue)

    console.log(
      `[ComponentStore] Component ${componentId} property update: ${fieldName} = ${JSON.stringify(actualValue)}`
    )

    // Update the component property
    updateComponentProperty(componentId, fieldName, actualValue)
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
    handlePropertyUpdate
  }
})
