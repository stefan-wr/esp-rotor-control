<template>
  <main class="gap-one">
    <Compass />
    <aside class="flex-vst gap-one">
      <ManualRotation></ManualRotation>
      <SpeedControl></SpeedControl>
      <Favorites></Favorites>
    </aside>
  </main>
</template>

<script setup>
import Compass from '@/components/Compass.vue';
import SpeedControl from '@/components/SpeedControl.vue';
import ManualRotation from '@/components/ManualRotation.vue'
import Favorites from '@/components/Favorites.vue'

import { ref, computed } from 'vue';

import { useRotorStore } from '@/stores/rotor';
import { useUmbrellaStore } from '@/stores/umbrella';

const rotorStore = useRotorStore();
const umbrellaStore = useUmbrellaStore();
</script>

<style lang="scss" scoped>
main {
  display: grid;
  align-items: start;
  grid-template-columns: minmax(0, 2fr) minmax(0, 1.5fr);

  @include large {
    grid-template-columns: repeat(2, minmax(0, 2fr));
  }

  @include medium {
    display: flex;
    align-items: stretch;
    flex-direction: column;
  }
}
</style>
