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
function handleResumeProject() {
  // TODO: Implement resume project
  console.log('Resume project clicked')
}

function handleOpenProject() {
  // TODO: Implement open project
  console.log('Open project clicked')
}

function handleNewProject() {
  // TODO: Implement new project
  console.log('New project clicked')
}

function handleLaunchTutorial() {
  // TODO: Implement tutorial
  console.log('Tutorial clicked')
}

function handleExit() {
  // TODO: Implement exit
  console.log('Exit clicked')
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
