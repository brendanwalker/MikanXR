<template>
  <li
    :class="{ selected: node.componentId === selectedId }"
    @click.stop="emit('select', node.componentId)"
  >
    {{ node.name }}
    <ul v-if="node.children.length > 0">
      <OutlinerNode
        v-for="child in node.children"
        :key="child.componentId"
        :node="child"
        :selected-id="selectedId"
        @select="emit('select', $event)"
      />
    </ul>
  </li>
</template>

<script setup lang="ts">
interface SceneOutlinerNode {
  name: string
  componentId: number
  ownerSystem: string
  componentType: string
  children: SceneOutlinerNode[]
}

defineProps<{
  node: SceneOutlinerNode
  selectedId: number
}>()

const emit = defineEmits<{
  (e: 'select', componentId: number): void
}>()
</script>

<style scoped>
li {
  display: block;
  position: relative;
  padding-left: calc(2 * var(--spacing) - var(--radius) - 2px);
  padding-top: 5px;
  padding-bottom: 5px;
  padding-right: 12px;
  color: #fff;
  cursor: pointer;
  font-size: 13px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.05);
  transition: background-color 0.15s;
}

li:hover {
  background-color: rgba(255, 255, 255, 0.06);
}

li.selected {
  background-color: rgba(92, 184, 92, 0.25);
  color: #5cb85c;
  font-weight: 600;
}

/* Nested list */
ul {
  list-style: none;
  margin-left: calc(var(--radius) - var(--spacing));
  padding-left: 0;
}

/* Vertical connecting line on non-root items */
ul li {
  border-left: 2px solid rgba(255, 255, 255, 0.18);
}

/* Remove the vertical line after the last child */
ul li:last-child {
  border-color: transparent;
}

/* L-shaped connector elbow for each non-root item */
ul li::before {
  content: '';
  display: block;
  position: absolute;
  top: calc(var(--spacing) / -2);
  left: -2px;
  width: calc(var(--spacing) + 2px);
  height: calc(var(--spacing) + 1px);
  border: solid rgba(255, 255, 255, 0.18);
  border-width: 0 0 2px 2px;
}
</style>
