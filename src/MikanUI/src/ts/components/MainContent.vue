<template>
  <div class="panel">
    <div class="header">
      <h2>{{ appStageName }}</h2>
      <button @click="handleDisconnect" class="disconnect-btn">
        Disconnect
      </button>
    </div>

    <!-- Dynamic component based on app stage -->
    <component :is="currentStageComponent" v-if="currentStageComponent" />
    <div v-else>
      <p>Unknown app stage: {{ mikanStore.appStage }}</p>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { useMikanStore } from '../stores/mikanStore'
import MainMenu from './stages/MainMenu.vue'
import VideoSourceSettings from './stages/VideoSourceSettings.vue'
import AnchorSetup from './stages/AnchorSetup.vue'

const mikanStore = useMikanStore()

const appStageName = computed(() => {
  const stage = mikanStore.appStage
  // Convert camelCase to Title Case
  return stage.replace(/([A-Z])/g, ' $1').trim()
})

const currentStageComponent = computed(() => {
  switch (mikanStore.appStage) {
    case 'MainMenu':
      return MainMenu
    case 'VideoSourceSettings':
      return VideoSourceSettings
    case 'AnchorSetup':
      return AnchorSetup
    default:
      return null
  }
})

function handleDisconnect() {
  mikanStore.disconnect()
}
</script>

<style scoped>
.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
  padding-bottom: 10px;
  border-bottom: 1px solid #ccc;
}

.disconnect-btn {
  background-color: #d9534f;
  color: white;
  border: none;
  padding: 8px 16px;
  border-radius: 4px;
  cursor: pointer;
}

.disconnect-btn:hover {
  background-color: #c9302c;
}
</style>
