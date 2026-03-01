<template>
  <div class="component-list">
    <div class="list-header">
      <h2 class="list-title">{{ title }}</h2>
      <span class="component-count">{{ components.length }} {{ components.length === 1 ? 'component' : 'components' }}</span>
    </div>

    <div v-if="components.length === 0" class="empty-state">
      <p>No {{ title.toLowerCase() }} found</p>
    </div>

    <div v-else class="components-container">
      <ComponentCard
        v-for="(component, index) in sortedComponents"
        :key="getComponentKey(component, index)"
        :component-id="getComponentId(component, index)"
        :component="component"
        :show-all-properties="showAllProperties"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import ComponentCard from './ComponentCard.vue'
import type { MikanComponentValues } from '@mikanxr/client'

interface Props {
  title: string
  components: MikanComponentValues[]
  showAllProperties?: boolean
  sortBy?: 'name' | 'id'
}

const props = withDefaults(defineProps<Props>(), {
  showAllProperties: false,
  sortBy: 'name'
})

// Sort components by name or ID
const sortedComponents = computed(() => {
  const sorted = [...props.components]

  if (props.sortBy === 'name') {
    sorted.sort((a, b) => {
      const nameA = a.component_name || ''
      const nameB = b.component_name || ''
      return nameA.localeCompare(nameB)
    })
  } else if (props.sortBy === 'id') {
    sorted.sort((a, b) => {
      const idA = (a as any).component_id || 0
      const idB = (b as any).component_id || 0
      return idA - idB
    })
  }

  return sorted
})

// Extract component ID from the component data
function getComponentId(component: MikanComponentValues, index: number): number {
  return (component as any).component_id ?? index
}

// Generate a unique key for each component
function getComponentKey(component: MikanComponentValues, index: number): string {
  const id = getComponentId(component, index)
  return `${component.component_name}-${id}`
}
</script>

<style scoped>
.component-list {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.list-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding-bottom: 12px;
  border-bottom: 2px solid rgba(255, 255, 255, 0.2);
}

.list-title {
  font-size: 20px;
  font-weight: 700;
  margin: 0;
  color: #fff;
}

.component-count {
  font-size: 14px;
  color: rgba(255, 255, 255, 0.6);
  font-weight: 500;
}

.empty-state {
  padding: 32px;
  text-align: center;
  color: rgba(255, 255, 255, 0.5);
  font-style: italic;
}

.empty-state p {
  margin: 0;
}

.components-container {
  display: flex;
  flex-direction: column;
}
</style>
