<template>
  <div class="panel">
    <h1>Mikan Connection</h1>
    <div class="form-group">
      <label for="server-url">Server URL:</label>
      <input
        id="server-url"
        v-model="serverUrl"
        type="text"
        :disabled="mikanStore.isConnecting"
      />
    </div>
    <div class="status" :class="statusClass">
      {{ statusMessage }}
    </div>
    <div class="button-group">
      <button
        @click="handleConnect"
        :disabled="mikanStore.isConnecting"
      >
        {{ mikanStore.isConnecting ? 'Connecting...' : 'Connect' }}
      </button>
      <button
        @click="handleLaunch"
        :disabled="isLaunching"
      >
        {{ isLaunching ? 'Launching...' : 'Launch Mikan' }}
      </button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import { useMikanStore } from '../stores/mikanStore'
import { nativeBridge } from '../native-bridge'

const mikanStore = useMikanStore()
const serverUrl = ref('ws://localhost:8080')
const isLaunching = ref(false)

const statusClass = computed(() => mikanStore.connectionStatus)

const statusMessage = computed(() => {
  switch (mikanStore.connectionStatus) {
    case 'connecting':
      return 'Connecting to Mikan...'
    case 'disconnected':
      return 'Not connected'
    case 'error':
      return `Connection Error: ${mikanStore.errorMessage || 'Unknown error'}`
    default:
      return 'Not connected'
  }
})

function parseWebSocketUrl(url: string): { host: string; port: string } {
  try {
    const wsUrl = new URL(url)
    return {
      host: wsUrl.hostname,
      port: wsUrl.port || '8080'
    }
  } catch (e) {
    // Fallback parsing
    const parts = url.replace('ws://', '').replace('wss://', '').split(':')
    return {
      host: parts[0] || 'localhost',
      port: parts[1] || '8080'
    }
  }
}

async function handleConnect() {
  const url = serverUrl.value.trim()
  if (!url) return

  const { host, port } = parseWebSocketUrl(url)
  await mikanStore.connect(host, port)
}

async function handleLaunch() {
  isLaunching.value = true

  try {
    const response = await nativeBridge.launchMikan()

    if (response.success) {
      // Wait a moment for Mikan to start, then connect
      setTimeout(async () => {
        const { host, port } = parseWebSocketUrl('ws://localhost:8080')
        await mikanStore.connect(host, port)
        isLaunching.value = false
      }, 2000)
    } else {
      console.error('Failed to launch Mikan:', response.error)
      isLaunching.value = false
    }
  } catch (error) {
    console.error('Failed to launch Mikan:', error)
    isLaunching.value = false
  }
}
</script>

<style scoped>
.status.connecting {
  color: #f0ad4e;
}

.status.error {
  color: #d9534f;
}

.status.disconnected {
  color: #777;
}
</style>
