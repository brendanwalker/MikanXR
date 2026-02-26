<template>
  <div id="app">
    <ConnectionPanel v-if="!mikanStore.isConnected" />
    <MainContent v-else />
  </div>
</template>

<script setup lang="ts">
import { onBeforeUnmount } from 'vue'
import { useMikanStore } from './stores/mikanStore'
import ConnectionPanel from './components/ConnectionPanel.vue'
import MainContent from './components/MainContent.vue'

const mikanStore = useMikanStore()

// Cleanup on component unmount
onBeforeUnmount(() => {
  mikanStore.disconnect()
})
</script>

<style>
/* Global styles can go here or keep using the existing style.css */
</style>
