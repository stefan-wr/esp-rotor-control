<template>
  <div class="trans-wrap" ref="wrap">
    <div ref="child">
      <slot></slot>
    </div>
  </div>
</template>

<script setup>
import { ref, watch, onMounted, onBeforeUnmount } from 'vue';
import { useUIStore } from '@/stores/ui';

const uiStore = useUIStore();

// Toggle
const props = defineProps({
  toggle: {
    type: Boolean,
    default: false,
  }
});

const wrap = ref(null);
const child = ref(null);

// Adjust height of wrapper to height of child
function applyHeight() {
  wrap.value.style.height = String(child.value.offsetHeight) + 'px';
}

// Initially, set wrapper height to height of child and
// reapply height when the browser window resizes
onMounted(() => {
  applyHeight();
  window.addEventListener('resize', applyHeight);
});

// Remove resize event-listener when component is unmounted
onBeforeUnmount(() => {
  window.removeEventListener('resize', applyHeight);
});

// Reapply height when font size changes
watch(
  () => uiStore.ui.fontSize,
  () => {
    applyHeight();
  }
);

// Reapply height when value of toggle changes
watch(
  () => props.toggle,
  () => {
    applyHeight();
  }
);

// Expose to parent components
defineExpose({
  applyHeight
});
</script>

<style lang="scss" scoped>
.trans-wrap {
  transition: height $slide-speed;
  overflow: hidden;
}
</style>
