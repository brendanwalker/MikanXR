<template>
  <div class="project-panel">
    <h2>Scenes Panel</h2>
    <p>Click on a component to edit its properties.</p>

    <div class="scenes-content">
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

      <!-- Scene Components with Actions -->
      <div class="component-section">
        <div class="section-header">
          <h3>Scene Components</h3>
          <div class="section-actions">
            <button @click="handleAddScene" class="action-btn add-btn">+ Add Scene</button>
            <button
              @click="handleReloadScript"
              :disabled="sceneComponents.length === 0"
              class="action-btn"
            >
              Reload Script
            </button>
          </div>
        </div>
        <ComponentList
          :components="sceneComponents"
          :selectable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent($event, 'SceneObjectSystem')"
        />
        <button
          v-if="selectedComponentId && isSceneSelected"
          @click="handleRemoveScene"
          class="action-btn remove-btn"
        >
          Remove Selected Scene
        </button>
      </div>

      <!-- Anchor Components with Actions -->
      <div class="component-section">
        <div class="section-header">
          <h3>Anchor Components</h3>
          <div class="section-actions">
            <button @click="handleAddAnchor" class="action-btn add-btn">+ Add Anchor</button>
          </div>
        </div>
        <ComponentList
          :components="anchorComponents"
          :selectable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent($event, 'AnchorObjectSystem')"
        />
        <div v-if="selectedComponentId && isAnchorSelected" class="action-buttons">
          <button @click="handleEditAnchor" class="action-btn">Edit Anchor Alignment</button>
          <button @click="handleRemoveAnchor" class="action-btn remove-btn">Remove Anchor</button>
        </div>
      </div>

      <!-- Stencil Components with Actions -->
      <div class="component-section">
        <div class="section-header">
          <h3>Stencil Components</h3>
          <div class="section-actions">
            <button @click="handleAddQuadStencil" class="action-btn add-btn">+ Quad</button>
            <button @click="handleAddBoxStencil" class="action-btn add-btn">+ Box</button>
            <button @click="handleAddModelStencil" class="action-btn add-btn">+ Model</button>
          </div>
        </div>

        <div v-if="quadStencilComponents.length > 0" class="stencil-subsection">
          <h4>Quad Stencils</h4>
          <ComponentList
            :components="quadStencilComponents"
            :selectable="true"
            :selected-component-id="selectedComponentId"
            sort-by="name"
            @select="handleSelectComponent($event, 'QuadStencilSystem')"
          />
        </div>

        <div v-if="boxStencilComponents.length > 0" class="stencil-subsection">
          <h4>Box Stencils</h4>
          <ComponentList
            :components="boxStencilComponents"
            :selectable="true"
            :selected-component-id="selectedComponentId"
            sort-by="name"
            @select="handleSelectComponent($event, 'BoxStencilSystem')"
          />
        </div>

        <div v-if="modelStencilComponents.length > 0" class="stencil-subsection">
          <h4>Model Stencils</h4>
          <ComponentList
            :components="modelStencilComponents"
            :selectable="true"
            :selected-component-id="selectedComponentId"
            sort-by="name"
            @select="handleSelectComponent($event, 'ModelStencilSystem')"
          />
        </div>

        <button
          v-if="selectedComponentId && isStencilSelected"
          @click="handleRemoveStencil"
          class="action-btn remove-btn"
        >
          Remove Selected Stencil
        </button>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useMikanStore } from '../../../stores/mikanStore.js'
import { usePropertyEditor } from '../../../composables/usePropertyEditor.js'
import { useRemoteControl } from '../../../composables/useRemoteControl.js'
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
const { sendRemoteControlCommand } = useRemoteControl()

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

const sceneComponents = computed(() =>
  componentStore.getComponentsByClass('SceneComponent')
)

const anchorComponents = computed(() =>
  componentStore.getComponentsByClass('AnchorComponent')
)

const quadStencilComponents = computed(() =>
  componentStore.getComponentsByClass('QuadStencilComponent')
)

const boxStencilComponents = computed(() =>
  componentStore.getComponentsByClass('BoxStencilComponent')
)

const modelStencilComponents = computed(() =>
  componentStore.getComponentsByClass('ModelStencilComponent')
)

const hasStencilComponents = computed(() =>
  quadStencilComponents.value.length > 0 ||
  boxStencilComponents.value.length > 0 ||
  modelStencilComponents.value.length > 0
)

// Check which type of component is selected
const isSceneSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return component?.component_class === 'SceneComponent'
})

const isAnchorSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return component?.component_class === 'AnchorComponent'
})

const isStencilSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return ['QuadStencilComponent', 'BoxStencilComponent', 'ModelStencilComponent'].includes(
    component?.component_class || ''
  )
})

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
    console.error('[ProjectScenes] Cannot save: no component selected or not connected')
    return
  }

  const { id: componentId, system: ownerSystem } = editingComponent.value

  console.log(`[ProjectScenes] Saving ${Object.keys(changes).length} property changes for component ${componentId}`)

  // Send each property update to the server
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
        console.log(`[ProjectScenes] Successfully updated ${fieldName}`)
      } else {
        console.error(`[ProjectScenes] Failed to update ${fieldName}: ${response.resultCode}`)
      }
    } catch (error) {
      console.error(`[ProjectScenes] Error updating ${fieldName}:`, error)
    }
  }

  closeEditor()
}

// Scene CRUD handlers
function handleAddScene() {
  sendRemoteControlCommand('add_scene')
}

function handleRemoveScene() {
  if (!selectedComponentId.value || !isSceneSelected.value) {
    console.error('[ProjectScenes] No scene selected')
    return
  }
  sendRemoteControlCommand('remove_scene', [selectedComponentId.value.toString()])
  selectedComponentId.value = null
}

function handleReloadScript() {
  sendRemoteControlCommand('reload_script')
}

// Anchor CRUD handlers
function handleAddAnchor() {
  sendRemoteControlCommand('add_anchor')
}

function handleRemoveAnchor() {
  if (!selectedComponentId.value || !isAnchorSelected.value) {
    console.error('[ProjectScenes] No anchor selected')
    return
  }
  sendRemoteControlCommand('remove_anchor', [selectedComponentId.value.toString()])
  selectedComponentId.value = null
}

function handleEditAnchor() {
  if (!selectedComponentId.value || !isAnchorSelected.value) {
    console.error('[ProjectScenes] No anchor selected')
    return
  }
  sendRemoteControlCommand('edit_anchor', [selectedComponentId.value.toString()])
}

// Stencil CRUD handlers
function handleAddQuadStencil() {
  sendRemoteControlCommand('add_quad_stencil')
}

function handleAddBoxStencil() {
  sendRemoteControlCommand('add_box_stencil')
}

function handleAddModelStencil() {
  sendRemoteControlCommand('add_model_stencil')
}

function handleRemoveStencil() {
  if (!selectedComponentId.value || !isStencilSelected.value) {
    console.error('[ProjectScenes] No stencil selected')
    return
  }
  sendRemoteControlCommand('remove_stencil', [selectedComponentId.value.toString()])
  selectedComponentId.value = null
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

.scenes-content {
  display: flex;
  flex-direction: column;
  gap: 24px;
}

.component-section {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 20px;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.section-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 8px;
}

.section-header h3 {
  color: #ffffff;
  margin: 0;
  font-size: 18px;
  font-weight: 600;
}

.section-actions {
  display: flex;
  gap: 8px;
}

.stencil-subsection {
  margin-top: 12px;
  padding-left: 16px;
  border-left: 2px solid #404040;
}

.stencil-subsection h4 {
  color: rgba(255, 255, 255, 0.8);
  margin: 0 0 12px 0;
  font-size: 14px;
  font-weight: 500;
}

.action-btn {
  padding: 8px 16px;
  font-size: 14px;
  background-color: #5cb85c;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  transition: background-color 0.2s;
  font-weight: 500;
}

.action-btn:hover:not(:disabled) {
  background-color: #4cae4c;
}

.action-btn:disabled {
  background-color: #3d3d3d;
  color: #999;
  cursor: not-allowed;
}

.action-btn.add-btn {
  background-color: #5cb85c;
}

.action-btn.add-btn:hover {
  background-color: #4cae4c;
}

.action-btn.remove-btn {
  background-color: #d9534f;
  width: 100%;
  margin-top: 8px;
}

.action-btn.remove-btn:hover {
  background-color: #c9302c;
}

.action-buttons {
  display: flex;
  flex-direction: column;
  gap: 8px;
  margin-top: 8px;
}

.action-buttons .action-btn {
  width: 100%;
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
