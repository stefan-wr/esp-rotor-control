<template>
  <div class="content">
    <Header />
    <RouterView />
  </div>
</template>

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

// Fire resize event after locale was applied to readjust heights.
// Set lang attribute in html document to new locale.
watch(
  () => uiStore.ui.localeApplied,
  () => {
    window.dispatchEvent(new Event('resize'));
    document.documentElement.setAttribute("lang", uiStore.ui.locale);
  },
  { flush: 'post' }
);

// Apply UI settings from local storage
onBeforeMount(() => {
  uiStore.applyColorTheme();
  uiStore.applyFontSize();
});
</script>

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
