import { useMikanStore } from '../stores/mikanStore.js'
import {
  MikanRemoteControlCommand,
  CLASS_ID_MIKAN_REMOTE_CONTROL_COMMAND,
  MikanRemoteControlCommandResult
} from '@mikanxr/client'

/**
 * Composable for sending remote control commands to the Mikan server
 */
export function useRemoteControl() {
  const mikanStore = useMikanStore()

  /**
   * Send a remote control command to the current app stage
   * @param command - The command name (e.g., 'resume_project', 'open_project', 'exit')
   * @param parameters - Optional array of string parameters for the command
   * @returns Promise that resolves to true if command succeeded, false otherwise
   */
  async function sendRemoteControlCommand(
    command: string,
    parameters: string[] = []
  ): Promise<boolean> {
    if (!mikanStore.client) {
      console.error('Mikan client not connected')
      return false
    }

    try {
      const request: MikanRemoteControlCommand = {
        requestTypeId: CLASS_ID_MIKAN_REMOTE_CONTROL_COMMAND,
        requestTypeName: 'MikanRemoteControlCommand',
        requestId: 0,
        command,
        parameters
      }

      const future = mikanStore.client.sendRequest(request)
      const response = await future.await() as MikanRemoteControlCommandResult

      if (response.results && response.results.length > 0 && response.results[0] === 'success') {
        console.log(`Command '${command}' succeeded`)
        return true
      } else {
        console.error(`Command '${command}' failed`, response.results)
        return false
      }
    } catch (error) {
      console.error(`Error sending command '${command}':`, error)
      return false
    }
  }

  return {
    sendRemoteControlCommand
  }
}
