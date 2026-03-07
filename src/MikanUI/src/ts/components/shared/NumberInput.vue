<template>
  <div class="number-input">
    <input
      ref="inputRef"
      type="text"
      :value="displayValue"
      @input="handleInput"
      @blur="handleBlur"
      @keydown="handleKeyDown"
      @wheel="handleWheel"
      class="number-field"
    />
    <div class="stepper-buttons">
      <button @click="increment" class="step-btn step-up" type="button">▲</button>
      <button @click="decrement" class="step-btn step-down" type="button">▼</button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch } from 'vue'

interface Props {
  modelValue: number
  min?: number
  max?: number
  step?: number
  precision?: number
}

const props = withDefaults(defineProps<Props>(), {
  step: 0.1,
  precision: 3
})

const emit = defineEmits<{
  'update:modelValue': [value: number]
}>()

const inputRef = ref<HTMLInputElement | null>(null)
const tempValue = ref<string>('')
const isEditing = ref(false)

const displayValue = computed(() => {
  if (isEditing.value) {
    return tempValue.value
  }
  return props.modelValue.toFixed(props.precision)
})

function clampValue(value: number): number {
  let clamped = value
  if (props.min !== undefined) {
    clamped = Math.max(props.min, clamped)
  }
  if (props.max !== undefined) {
    clamped = Math.min(props.max, clamped)
  }
  return parseFloat(clamped.toFixed(props.precision))
}

function handleInput(event: Event) {
  const target = event.target as HTMLInputElement
  tempValue.value = target.value
  isEditing.value = true
}

function handleBlur() {
  const parsed = parseFloat(tempValue.value)
  if (!isNaN(parsed)) {
    emit('update:modelValue', clampValue(parsed))
  }
  isEditing.value = false
  tempValue.value = ''
}

function handleKeyDown(event: KeyboardEvent) {
  if (event.key === 'Enter') {
    inputRef.value?.blur()
  } else if (event.key === 'Escape') {
    isEditing.value = false
    tempValue.value = ''
    inputRef.value?.blur()
  } else if (event.key === 'ArrowUp') {
    event.preventDefault()
    increment()
  } else if (event.key === 'ArrowDown') {
    event.preventDefault()
    decrement()
  }
}

function handleWheel(event: WheelEvent) {
  if (document.activeElement === inputRef.value) {
    event.preventDefault()
    const delta = event.deltaY > 0 ? -1 : 1
    const stepSize = event.shiftKey ? props.step * 10 : event.ctrlKey ? props.step * 0.1 : props.step
    const newValue = props.modelValue + (delta * stepSize)
    emit('update:modelValue', clampValue(newValue))
  }
}

function increment() {
  const newValue = props.modelValue + props.step
  emit('update:modelValue', clampValue(newValue))
}

function decrement() {
  const newValue = props.modelValue - props.step
  emit('update:modelValue', clampValue(newValue))
}

// Update temp value when model value changes externally
watch(() => props.modelValue, () => {
  if (!isEditing.value) {
    tempValue.value = ''
  }
})
</script>

<style scoped>
.number-input {
  display: flex;
  align-items: stretch;
  gap: 2px;
  background: rgba(255, 255, 255, 0.1);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 4px;
  overflow: hidden;
  transition: border-color 0.2s;
}

.number-input:focus-within {
  border-color: #5cb85c;
  background: rgba(255, 255, 255, 0.15);
}

.number-field {
  flex: 1;
  background: transparent;
  border: none;
  padding: 6px 8px;
  color: white;
  font-size: 13px;
  font-family: monospace;
  min-width: 0;
  text-align: right;
}

.number-field:focus {
  outline: none;
}

.stepper-buttons {
  display: flex;
  flex-direction: column;
  background: rgba(0, 0, 0, 0.2);
  border-left: 1px solid rgba(255, 255, 255, 0.1);
}

.step-btn {
  flex: 1;
  background: transparent;
  border: none;
  color: rgba(255, 255, 255, 0.6);
  cursor: pointer;
  padding: 0 4px;
  font-size: 8px;
  line-height: 1;
  transition: all 0.2s;
  min-width: 10px;
}

.step-btn:hover {
  background: rgba(255, 255, 255, 0.1);
  color: rgba(255, 255, 255, 0.9);
}

.step-btn:active {
  background: rgba(255, 255, 255, 0.2);
  color: white;
}

.step-up {
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
}

/* Tooltip hint for advanced controls */
.number-input::after {
  content: '';
  display: none;
}

.number-input:focus-within::after {
  content: 'Wheel: ±step | Shift+Wheel: ±10x | Ctrl+Wheel: ±0.1x | ↑↓: step';
  position: absolute;
  bottom: -24px;
  left: 0;
  font-size: 10px;
  color: rgba(255, 255, 255, 0.5);
  white-space: nowrap;
  pointer-events: none;
}
</style>
