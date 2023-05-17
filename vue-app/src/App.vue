<script setup>
import { RouterView } from 'vue-router';
import Header from '@/components/Header.vue';

import { watch, onBeforeMount } from 'vue';

import { useRotorStore } from '@/stores/rotor';
import { useUIStore } from '@/stores/ui';

const rotorStore = useRotorStore();
const uiStore = useUIStore();

// Show current rotor angle in browser tab title.
watch(
  () => rotorStore.rotor.angle,
  () => {
    document.title = `RotorControl [ ${rotorStore.angle}°, ${rotorStore.cardinal} ]`;
  }
);

// Apply ui settings from local storage
onBeforeMount(() => {
  uiStore.applyColorTheme();
  uiStore.applyFontSize();
});
</script>

<template>
  <div class="content">
    <Header />
    <RouterView />
  </div>
</template>

<style lang="scss" scoped>
.content {
  flex-grow: 1;
  border-radius: $content-radius;
  background-color: var(--content-color-1);
  box-shadow: 0px 2em 3em var(--app-shadow-color);

  @include small() {
    border-radius: 0;
  }
}
</style>
