import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useSettingsStore = defineStore('settings', () => {
  // Developer mode toggle
  const developerMode = ref<boolean>(false)

  // Load from localStorage on initialization
  const storedDevMode = localStorage.getItem('mikan.developerMode')
  if (storedDevMode !== null) {
    developerMode.value = storedDevMode === 'true'
  }

  // Toggle developer mode
  function toggleDeveloperMode() {
    developerMode.value = !developerMode.value
    localStorage.setItem('mikan.developerMode', developerMode.value.toString())
  }

  // Set developer mode
  function setDeveloperMode(enabled: boolean) {
    developerMode.value = enabled
    localStorage.setItem('mikan.developerMode', enabled.toString())
  }

  return {
    // State
    developerMode,

    // Actions
    toggleDeveloperMode,
    setDeveloperMode
  }
})
