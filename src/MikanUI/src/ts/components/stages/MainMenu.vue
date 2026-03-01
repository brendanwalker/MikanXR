<template>
  <div class="main-menu">
    <div class="menu-buttons">
      <button @click="handleResumeProject" class="menu-btn">
        Resume Project
      </button>
      <button @click="handleOpenProject" class="menu-btn">
        Open Project
      </button>
      <button @click="handleNewProject" class="menu-btn">
        New Project
      </button>
      <button @click="handleLaunchTutorial" class="menu-btn">
        Launch Tutorial
      </button>
      <button @click="handleExit" class="menu-btn exit-btn">
        Exit
      </button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { useRemoteControl } from '../../composables/useRemoteControl.js'
import { nativeBridge } from '../../native-bridge.js'

const { sendRemoteControlCommand } = useRemoteControl()

function handleResumeProject() {
  sendRemoteControlCommand('resume_project')
}

async function handleOpenProject() {
  // Show native file open dialog
  const response = await nativeBridge.showOpenFileDialog({
    title: 'Open Project',
    filter: '*.mikanproj',
    filterDescription: 'Project Files (*.mikanproj)'
  })

  if (response.success && response.data?.filePath) {
    sendRemoteControlCommand('open_project', [response.data.filePath])
  } else if (!response.data?.canceled) {
    console.error('Failed to open file dialog:', response.error)
  }
}

async function handleNewProject() {
  // Show native file save dialog
  const response = await nativeBridge.showSaveFileDialog({
    title: 'New Project',
    filter: '*.mikanproj',
    filterDescription: 'Project Files (*.mikanproj)'
  })

  if (response.success && response.data?.filePath) {
    sendRemoteControlCommand('new_project', [response.data.filePath])
  } else if (!response.data?.canceled) {
    console.error('Failed to open file dialog:', response.error)
  }
}

function handleLaunchTutorial() {
  sendRemoteControlCommand('tutorial')
}

function handleExit() {
  sendRemoteControlCommand('exit')
}
</script>

<style scoped>
.main-menu {
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: 400px;
}

.menu-buttons {
  display: flex;
  flex-direction: column;
  gap: 15px;
  width: 300px;
}

.menu-btn {
  padding: 15px 30px;
  font-size: 16px;
  background-color: #5cb85c;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  transition: background-color 0.2s;
}

.menu-btn:hover {
  background-color: #4cae4c;
}

.exit-btn {
  background-color: #d9534f;
  margin-top: 20px;
}

.exit-btn:hover {
  background-color: #c9302c;
}
</style>
