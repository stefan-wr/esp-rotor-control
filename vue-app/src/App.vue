<script setup>
import { RouterView } from 'vue-router';
import Header from '@/components/Header.vue';

import { watch } from 'vue';

import { useRotorStore } from '@/stores/rotor';

const rotorStore = useRotorStore();

// Show current rotor angle in browser tab title.
watch(rotorStore.rotor, () => {
  document.title = `RotorControl [ ${rotorStore.angle}°, ${rotorStore.cardinal} ]`;
})

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
