<template>
  <div class="color-picker">
    <div class="color-preview" :style="{ backgroundColor: displayColor }" @click="togglePicker"></div>
    <input
      v-if="showPicker"
      type="color"
      :value="hexColor"
      @input="handleColorChange"
      class="color-input"
    />
    <div v-if="showChannels" class="color-channels">
      <div class="channel">
        <label>R</label>
        <input type="number" min="0" max="255" :value="red" @input="updateRed" />
      </div>
      <div class="channel">
        <label>G</label>
        <input type="number" min="0" max="255" :value="green" @input="updateGreen" />
      </div>
      <div class="channel">
        <label>B</label>
        <input type="number" min="0" max="255" :value="blue" @input="updateBlue" />
      </div>
      <div v-if="hasAlpha" class="channel">
        <label>A</label>
        <input type="number" min="0" max="255" :value="alpha" @input="updateAlpha" />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch } from 'vue'

interface Props {
  modelValue: { r: number; g: number; b: number; a?: number }
  showChannels?: boolean
}

const props = withDefaults(defineProps<Props>(), {
  showChannels: true
})

const emit = defineEmits<{
  'update:modelValue': [value: { r: number; g: number; b: number; a?: number }]
}>()

const showPicker = ref(false)

const red = computed(() => Math.round(props.modelValue.r * 255))
const green = computed(() => Math.round(props.modelValue.g * 255))
const blue = computed(() => Math.round(props.modelValue.b * 255))
const alpha = computed(() => props.modelValue.a !== undefined ? Math.round(props.modelValue.a * 255) : 255)
const hasAlpha = computed(() => props.modelValue.a !== undefined)

const hexColor = computed(() => {
  const r = red.value.toString(16).padStart(2, '0')
  const g = green.value.toString(16).padStart(2, '0')
  const b = blue.value.toString(16).padStart(2, '0')
  return `#${r}${g}${b}`
})

const displayColor = computed(() => {
  if (hasAlpha.value) {
    return `rgba(${red.value}, ${green.value}, ${blue.value}, ${alpha.value / 255})`
  }
  return hexColor.value
})

function togglePicker() {
  showPicker.value = !showPicker.value
}

function handleColorChange(event: Event) {
  const target = event.target as HTMLInputElement
  const hex = target.value
  const r = parseInt(hex.slice(1, 3), 16) / 255
  const g = parseInt(hex.slice(3, 5), 16) / 255
  const b = parseInt(hex.slice(5, 7), 16) / 255

  const newValue = { r, g, b }
  if (hasAlpha.value) {
    (newValue as any).a = props.modelValue.a
  }
  emit('update:modelValue', newValue)
}

function updateRed(event: Event) {
  const value = parseInt((event.target as HTMLInputElement).value) / 255
  const newValue = { ...props.modelValue, r: Math.max(0, Math.min(1, value)) }
  emit('update:modelValue', newValue)
}

function updateGreen(event: Event) {
  const value = parseInt((event.target as HTMLInputElement).value) / 255
  const newValue = { ...props.modelValue, g: Math.max(0, Math.min(1, value)) }
  emit('update:modelValue', newValue)
}

function updateBlue(event: Event) {
  const value = parseInt((event.target as HTMLInputElement).value) / 255
  const newValue = { ...props.modelValue, b: Math.max(0, Math.min(1, value)) }
  emit('update:modelValue', newValue)
}

function updateAlpha(event: Event) {
  if (!hasAlpha.value) return
  const value = parseInt((event.target as HTMLInputElement).value) / 255
  const newValue = { ...props.modelValue, a: Math.max(0, Math.min(1, value)) }
  emit('update:modelValue', newValue)
}
</script>

<style scoped>
.color-picker {
  display: flex;
  align-items: center;
  gap: 12px;
}

.color-preview {
  width: 40px;
  height: 40px;
  border-radius: 4px;
  border: 2px solid rgba(255, 255, 255, 0.3);
  cursor: pointer;
  transition: border-color 0.2s;
}

.color-preview:hover {
  border-color: rgba(255, 255, 255, 0.6);
}

.color-input {
  width: 60px;
  height: 40px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  background: transparent;
}

.color-channels {
  display: flex;
  gap: 8px;
  flex: 1;
}

.channel {
  display: flex;
  flex-direction: column;
  gap: 4px;
  flex: 1;
}

.channel label {
  font-size: 12px;
  color: rgba(255, 255, 255, 0.7);
  font-weight: 500;
}

.channel input[type="number"] {
  background: rgba(255, 255, 255, 0.1);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 4px;
  padding: 6px 8px;
  color: white;
  font-size: 13px;
  font-family: monospace;
  width: 100%;
}

.channel input[type="number"]:focus {
  outline: none;
  border-color: #5cb85c;
  background: rgba(255, 255, 255, 0.15);
}

/* Remove number input arrows */
.channel input[type="number"]::-webkit-inner-spin-button,
.channel input[type="number"]::-webkit-outer-spin-button {
  -webkit-appearance: none;
  margin: 0;
}

.channel input[type="number"] {
  -moz-appearance: textfield;
}
</style>
