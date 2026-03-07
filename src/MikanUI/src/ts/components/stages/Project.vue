<template>
  <div class="project-stage">
    <!-- Navigation Tabs -->
    <nav class="project-nav">
      <button
        v-for="panel in panels"
        :key="panel.id"
        :class="['nav-tab', { active: activePanel === panel.id }]"
        @click="switchPanel(panel.id)"
      >
        {{ panel.label }}
      </button>
      <button class="nav-tab nav-back" @click="handleReturn">
        Return
      </button>
    </nav>

    <!-- Panel Content Area -->
    <div class="panel-content">
      <component :is="activePanelComponent" />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import { useRemoteControl } from '../../composables/useRemoteControl.js'
import ProjectScenes from './project/ProjectScenes.vue'
import ProjectStages from './project/ProjectStages.vue'
import ProjectSources from './project/ProjectSources.vue'
import ProjectTracking from './project/ProjectTracking.vue'
import ProjectMarkers from './project/ProjectMarkers.vue'
import ProjectSettings from './project/ProjectSettings.vue'

type PanelId = 'scenes' | 'stages' | 'sources' | 'tracking' | 'markers' | 'settings'

interface Panel {
  id: PanelId
  label: string
  component: any
}

const panels: Panel[] = [
  { id: 'scenes', label: 'Scenes', component: ProjectScenes },
  { id: 'stages', label: 'Stages', component: ProjectStages },
  { id: 'sources', label: 'Sources', component: ProjectSources },
  { id: 'tracking', label: 'Tracking', component: ProjectTracking },
  { id: 'markers', label: 'Markers', component: ProjectMarkers },
  { id: 'settings', label: 'Settings', component: ProjectSettings }
]

const activePanel = ref<PanelId>('scenes')
const { sendRemoteControlCommand } = useRemoteControl()

const activePanelComponent = computed(() => {
  return panels.find(p => p.id === activePanel.value)?.component
})

function switchPanel(panelId: PanelId) {
  activePanel.value = panelId
}

function handleReturn() {
  // Send pop_app_stage command to return to main menu
  sendRemoteControlCommand('pop_app_stage')
}
</script>

<style scoped>
.project-stage {
  display: flex;
  flex-direction: column;
  width: 100%;
  height: 100%;
  background-color: #1e1e1e;
  color: #e0e0e0;
  overflow: hidden;
}

.project-nav {
  display: flex;
  flex-wrap: wrap;
  background-color: #2d2d2d;
  border-bottom: 2px solid #404040;
  padding: 0;
  flex-shrink: 0;
}

.nav-tab {
  padding: 12px 24px;
  background: none;
  border: none;
  color: #b0b0b0;
  cursor: pointer;
  font-size: 14px;
  font-weight: 500;
  border-bottom: 3px solid transparent;
  transition: all 0.2s;
  white-space: nowrap;
}

.nav-tab:hover {
  background-color: #363636;
  color: #ffffff;
}

.nav-tab.active {
  color: #ffffff;
  border-bottom-color: #5cb85c;
  background-color: #333333;
}

.nav-back {
  margin-left: auto;
  background-color: #d9534f;
  color: white;
  border-bottom: 3px solid transparent;
}

.nav-back:hover {
  background-color: #c9302c;
}

.panel-content {
  flex: 1;
  overflow-y: auto;
  padding: 20px;
}

/* Scrollbar styling */
.panel-content::-webkit-scrollbar {
  width: 12px;
}

.panel-content::-webkit-scrollbar-track {
  background: #2d2d2d;
}

.panel-content::-webkit-scrollbar-thumb {
  background: #555;
  border-radius: 6px;
}

.panel-content::-webkit-scrollbar-thumb:hover {
  background: #666;
}

/* Responsive layout for smaller screens */
@media (max-width: 768px) {
  .project-nav {
    padding: 12px;
  }

  .nav-tab {
    padding: 10px 16px;
    font-size: 13px;
  }

  .panel-content {
    padding: 12px;
  }
}

@media (max-width: 480px) {
  .nav-tab {
    padding: 8px 12px;
    font-size: 12px;
  }

  .panel-content {
    padding: 8px;
  }
}
</style>
