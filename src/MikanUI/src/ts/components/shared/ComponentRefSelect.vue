<template>
  <select
    :value="modelValue"
    @change="handleChange"
    class="component-ref-select"
  >
    <option :value="-1">&lt;None&gt;</option>
    <option
      v-for="component in availableComponents"
      :key="component.id"
      :value="component.id"
    >
      {{ component.name }}
    </option>
  </select>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { useComponentStore } from '../../stores/componentStore.js'

interface Props {
  modelValue: number
  componentClass: string | string[]
}

const props = defineProps<Props>()
const emit = defineEmits<{
  (e: 'update:modelValue', value: number): void
}>()

const componentStore = useComponentStore()

// Get all components of the specified class(es)
const availableComponents = computed(() => {
  const classes = Array.isArray(props.componentClass) ? props.componentClass : [props.componentClass]

  const allComponents = classes.flatMap(className => {
    const components = componentStore.getComponentsByClass(className)
    return components.map(comp => ({
      id: (comp as any).component_id,
      name: comp.component_name || `Unnamed ${className}`,
      className
    }))
  })

  // Sort by name
  return allComponents.sort((a, b) => a.name.localeCompare(b.name))
})

function handleChange(event: Event) {
  const value = parseInt((event.target as HTMLSelectElement).value, 10)
  emit('update:modelValue', value)
}
</script>

<style scoped>
.component-ref-select {
  padding: 4px 8px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 4px;
  color: #fff;
  font-family: monospace;
  font-size: 14px;
  min-width: 150px;
  cursor: pointer;
}

.component-ref-select:focus {
  outline: none;
  border-color: #5cb85c;
  background: rgba(0, 0, 0, 0.4);
}

.component-ref-select:hover {
  background: rgba(0, 0, 0, 0.4);
  border-color: rgba(255, 255, 255, 0.3);
}

.component-ref-select option {
  background: #2d2d2d;
  color: #fff;
}
</style>
