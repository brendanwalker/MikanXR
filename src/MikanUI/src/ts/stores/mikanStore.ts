import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import {
  MikanClient,
  MikanClientOptions,
  MikanLogLevel,
  MikanAPIResult,
  GetAppStageInfo,
  CLASS_ID_GET_APP_STAGE_INFO,
  MikanAppStageInfoResponse,
  MikanComponent
} from '@mikanxr/client'

export type ConnectionStatus = 'disconnected' | 'connecting' | 'connected' | 'error'

export const useMikanStore = defineStore('mikan', () => {
  // State
  const client = ref<MikanClient | null>(null)
  const connectionStatus = ref<ConnectionStatus>('disconnected')
  const errorMessage = ref<string | null>(null)
  const appStage = ref<string>('MainMenu')
  const isConnecting = ref(false)

  // Component state management
  const components = ref<Map<number, MikanComponent>>(new Map())

  // Computed
  const isConnected = computed(() => connectionStatus.value === 'connected')

  // Actions
  async function connect(host: string, port: string) {
    if (isConnecting.value) return

    isConnecting.value = true
    connectionStatus.value = 'connecting'
    errorMessage.value = null

    try {
      const options: MikanClientOptions = {
        host,
        port,
        autoReconnect: false
      }

      client.value = new MikanClient(options)

      // Set log callback
      client.value.setLogCallback((level: MikanLogLevel, message: string) => {
        console.log(`[Mikan ${MikanLogLevel[level]}] ${message}`)
      })

      // Initialize client
      const initResult = client.value.initialize(MikanLogLevel.Info)
      if (initResult !== 0) {
        throw new Error('Failed to initialize Mikan client')
      }

      // Connect
      const connectResult = await client.value.connect()
      if (connectResult !== 0) {
        throw new Error('Failed to connect to Mikan')
      }

      connectionStatus.value = 'connected'
      await fetchAppStageInfo()

    } catch (error) {
      console.error('Connection error:', error)
      errorMessage.value = error instanceof Error ? error.message : 'Unknown error'
      connectionStatus.value = 'error'
      client.value = null
    } finally {
      isConnecting.value = false
    }
  }

  function disconnect() {
    if (client.value) {
      client.value.disconnect()
      client.value.shutdown()
      client.value = null
    }
    connectionStatus.value = 'disconnected'
    errorMessage.value = null
    components.value.clear()
  }

  async function fetchAppStageInfo() {
    if (!client.value) return

    try {
      const request: GetAppStageInfo = {
        requestTypeId: CLASS_ID_GET_APP_STAGE_INFO,
        requestTypeName: 'GetAppStageInfo',
        requestId: 0
      }

      const future = client.value.sendRequest(request)
      const response = await future.await()

      if (response.resultCode === MikanAPIResult.Success) {
        const appStageInfoResponse = response as MikanAppStageInfoResponse
        appStage.value = appStageInfoResponse.app_stage_info.app_state_name
      }
    } catch (error) {
      console.error('Failed to get app stage info:', error)
      errorMessage.value = 'Failed to get app info'
    }
  }

  // Component state management
  function updateComponent(componentId: number, component: MikanComponent) {
    components.value.set(componentId, component)
  }

  function getComponent(componentId: number): MikanComponent | undefined {
    return components.value.get(componentId)
  }

  function removeComponent(componentId: number) {
    components.value.delete(componentId)
  }

  return {
    // State
    client,
    connectionStatus,
    errorMessage,
    appStage,
    isConnecting,
    components,
    // Computed
    isConnected,
    // Actions
    connect,
    disconnect,
    fetchAppStageInfo,
    updateComponent,
    getComponent,
    removeComponent
  }
})
