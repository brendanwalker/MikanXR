<template>
  <div class="project-panel">
    <h2>Scenes Panel</h2>
    <p>Scene and stage management with scene outliner and component properties.</p>

    <div class="scenes-content">
      <!-- Stage Components -->
      <ComponentList
        title="Stage Components"
        :components="stageComponents"
        sort-by="name"
      />

      <!-- Scene Components -->
      <ComponentList
        title="Scene Components"
        :components="sceneComponents"
        sort-by="name"
      />

      <!-- Anchor Components -->
      <ComponentList
        title="Anchor Components"
        :components="anchorComponents"
        sort-by="name"
      />

      <!-- Stencil Components -->
      <div v-if="hasStencilComponents" class="stencil-section">
        <ComponentList
          v-if="quadStencilComponents.length > 0"
          title="Quad Stencil Components"
          :components="quadStencilComponents"
          sort-by="name"
        />
        <ComponentList
          v-if="boxStencilComponents.length > 0"
          title="Box Stencil Components"
          :components="boxStencilComponents"
          sort-by="name"
        />
        <ComponentList
          v-if="modelStencilComponents.length > 0"
          title="Model Stencil Components"
          :components="modelStencilComponents"
          sort-by="name"
        />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import ComponentList from '../../shared/ComponentList.vue'

const componentStore = useComponentStore()

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

.stencil-section {
  display: flex;
  flex-direction: column;
  gap: 24px;
}
</style>
