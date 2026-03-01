<template>
  <div class="project-panel">
    <h2>Settings Panel</h2>
    <p>Project-wide configuration and preferences.</p>

    <div class="placeholder-content">
      <div class="info-box">
        <h3>Coming Soon</h3>
        <ul>
          <li>Project Name & Description</li>
          <li>Default Camera Settings</li>
          <li>Rendering Options</li>
          <li>Debug Visualization Settings</li>
          <li>Export/Import Configuration</li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'

// TODO: Add project settings logic
</script>

<style scoped>
.project-panel {
  max-width: 1200px;
}

.project-panel h2 {
  color: #5cb85c;
  margin-bottom: 10px;
}

.project-panel p {
  color: #b0b0b0;
  margin-bottom: 20px;
}

.placeholder-content {
  margin-top: 30px;
}

.info-box {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 20px;
}

.info-box h3 {
  color: #ffffff;
  margin-bottom: 15px;
}

.info-box ul {
  list-style: none;
  padding: 0;
}

.info-box li {
  color: #b0b0b0;
  padding: 8px 0;
  border-bottom: 1px solid #404040;
}

.info-box li:last-child {
  border-bottom: none;
}

.info-box li::before {
  content: '○ ';
  color: #5cb85c;
  font-weight: bold;
  margin-right: 8px;
}
</style>
