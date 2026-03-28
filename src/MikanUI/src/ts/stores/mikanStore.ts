import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import {
  MikanClient,
  MikanClientOptions,
  MikanLogLevel,
  MikanAPIResult,
  GetAppStageInfo,
  MikanAppStageInfoResponse,
  MikanEvent,
  MikanDisconnectedEvent,
  MikanPropertyUpdateEvent,
  MikanAppStageChangedEvent,
  SetPropertyNotifyMode,
  MikanPropertyNotifyMode
} from '@mikanxr/client'
import { useComponentStore } from './componentStore.js'

export type ConnectionStatus = 'disconnected' | 'connecting' | 'connected' | 'error'

export const useMikanStore = defineStore('mikan', () => {
  // State
  const client = ref<MikanClient | null>(null)
  const connectionStatus = ref<ConnectionStatus>('disconnected')
  const errorMessage = ref<string | null>(null)
  const appStage = ref<string>('MainMenu')
  const isConnecting = ref(false)

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

      // Enable property update notifications
      await setPropertyNotifyMode()

      // Fetch initial data
      await fetchAppStageInfo()

      // Fetch all components from the server
      const componentStore = useComponentStore()
      await componentStore.fetchAllComponents(client.value)
      
      // Set up event listeners
      setupEventListeners()

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

    // Clear component database
    const componentStore = useComponentStore()
    componentStore.clearComponents()
  }

  async function setPropertyNotifyMode() {
    if (!client.value) return

    try {
      const request: SetPropertyNotifyMode = {
        requestTypeName: 'SetPropertyNotifyMode',
        requestId: 0,
        systemFilter: '',
        componentFilter: '',
        propertyFilter: '',
        notifyMode: MikanPropertyNotifyMode.NAME_AND_VALUE
      }

      const future = client.value.sendRequest(request)
      const response = await future.await()

      if (response.resultCode === MikanAPIResult.Success) {
        console.log('[MikanStore] Property notifications enabled')
      } else {
        console.error('[MikanStore] Failed to enable property notifications:', response.resultCode)
      }
    } catch (error) {
      console.error('[MikanStore] Error setting property notify mode:', error)
    }
  }

  async function fetchAppStageInfo() {
    if (!client.value) return

    try {
      const request: GetAppStageInfo = {
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

  // Set up event listeners using MikanEventManager
  function setupEventListeners() {
    if (!client.value) return

    const componentStore = useComponentStore()

    // Listen for connection events
    client.value.onConnected((event: MikanEvent) => {
      console.log('[MikanStore] Connected event received')
    })

    client.value.onDisconnected((event: MikanDisconnectedEvent) => {
      console.log(`[MikanStore] Disconnected event received: ${event.code}`)
      disconnect()
    })

    // Listen for app stage change events
    client.value.onAppStageChanged((event: MikanAppStageChangedEvent) => {
      console.log(`[MikanStore] App stage changed: ${event.old_app_state_name} -> ${event.new_app_state_name}`)
      appStage.value = event.new_app_state_name
    })

    // Listen for property update events
    client.value.onPropertyUpdate((event: MikanPropertyUpdateEvent) => {
      console.log(`[MikanStore] Property update event received`)
      componentStore.handlePropertyUpdate(event, client.value as MikanClient)
    })

    // Listen for all events for debugging (optional)
    client.value.onAnyEvent((event: MikanEvent) => {
      console.log(`[MikanStore] Event: ${event.eventTypeName}`)
    })
  }

  return {
    // State
    client,
    connectionStatus,
    errorMessage,
    appStage,
    isConnecting,
    // Computed
    isConnected,
    // Actions
    connect,
    disconnect,
    fetchAppStageInfo
  }
})
