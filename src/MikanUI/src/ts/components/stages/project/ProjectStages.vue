<template>
  <div class="project-panel">
    <h2>Stages Panel</h2>
    <p>Click on a stage or compositor component to edit its properties.</p>

    <div class="stages-content">
      <!-- Property Editor Modal -->
      <div v-if="editingComponent" class="editor-overlay">
        <PropertyEditor
          :title="editingComponent.component.component_name || 'Component'"
          :component-id="editingComponent.id"
          :component="editingComponent.component"
          :owner-system="editingComponent.system"
          @close="closeEditor"
          @save="handleSaveProperties"
        />
      </div>

      <!-- Stage Components -->
      <ComponentList
        title="Stage Components"
        :components="stageComponents"
        :selectable="true"
        :selected-component-id="selectedComponentId"
        sort-by="name"
        @select="handleSelectComponent($event, 'StageObjectSystem')"
      />

      <!-- Compositor Components -->
      <ComponentList
        title="Compositor Components"
        :components="compositorComponents"
        :selectable="true"
        :selected-component-id="selectedComponentId"
        sort-by="name"
        @select="handleSelectComponent($event, 'CompositorObjectSystem')"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useMikanStore } from '../../../stores/mikanStore.js'
import { usePropertyEditor } from '../../../composables/usePropertyEditor.js'
import ComponentList from '../../shared/ComponentList.vue'
import PropertyEditor from '../../shared/PropertyEditor.vue'
import {
  MikanAPIResult,
  PropertySetValueRequest,
  CLASS_ID_PROPERTY_SET_VALUE_REQUEST
} from '@mikanxr/client'

const componentStore = useComponentStore()
const mikanStore = useMikanStore()
const { createVariantFromValue } = usePropertyEditor()

// Selection state
const selectedComponentId = ref<number | null>(null)
const editingComponent = ref<{
  id: number
  component: any
  system: string
} | null>(null)

// Get components by system
const stageComponents = computed(() =>
  componentStore.getComponentsByClass('StageComponent')
)

const compositorComponents = computed(() =>
  componentStore.getComponentsByClass('CompositorComponent')
)

// Handle component selection
function handleSelectComponent(componentId: number, ownerSystem: string) {
  selectedComponentId.value = componentId
  const component = componentStore.getComponent(componentId)

  if (component) {
    editingComponent.value = {
      id: componentId,
      component,
      system: ownerSystem
    }
  }
}

function closeEditor() {
  editingComponent.value = null
  selectedComponentId.value = null
}

// Handle property save
async function handleSaveProperties(changes: Record<string, any>) {
  if (!editingComponent.value || !mikanStore.client) {
    console.error('[ProjectStages] Cannot save: no component selected or not connected')
    return
  }

  const { id: componentId, system: ownerSystem } = editingComponent.value

  console.log(`[ProjectStages] Saving ${Object.keys(changes).length} property changes for component ${componentId}`)

  for (const [fieldName, fieldValue] of Object.entries(changes)) {
    try {
      const variant = createVariantFromValue(fieldValue)

      const request: PropertySetValueRequest = {
        requestTypeId: CLASS_ID_PROPERTY_SET_VALUE_REQUEST,
        requestTypeName: 'PropertySetValueRequest',
        requestId: 0,
        ownerSystem,
        componentId,
        fieldName,
        fieldValue: variant
      }

      const future = mikanStore.client.sendRequest(request)
      const response = await future.await()

      if (response.resultCode === MikanAPIResult.Success) {
        console.log(`[ProjectStages] Successfully updated ${fieldName}`)
      } else {
        console.error(`[ProjectStages] Failed to update ${fieldName}: ${response.resultCode}`)
      }
    } catch (error) {
      console.error(`[ProjectStages] Error updating ${fieldName}:`, error)
    }
  }

  closeEditor()
}
</script>

<style scoped>
.project-panel {
  max-width: 1200px;
}

.project-panel h2 {
  color: #5cb85c;
  margin-bottom: 10px;
}

.project-panel p {
  color: #b0b0b0;
  margin-bottom: 20px;
}

.stages-content {
  display: flex;
  flex-direction: column;
  gap: 24px;
}

.editor-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.8);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
  padding: 20px;
}
</style>
