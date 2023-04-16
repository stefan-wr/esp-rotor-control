<template>
  <main>
    <div class="compass-wrapper">
      <Compass />
    </div>
    <aside>
      <ManualRotation></ManualRotation>
      <SpeedControl></SpeedControl>
      <Favorites></Favorites>
    </aside>
  </main>
</template>

<script setup>
import Compass from '@/components/Compass.vue';
import Card from '@/components/Card.vue';
import SpeedControl from '@/components/SpeedControl.vue';
import ManualRotation from '@/components/ManualRotation.vue'
import Favorites from '@/components/Favorites.vue'

import { ref, computed } from 'vue';

import { useRotorStore } from '@/stores/rotor';
import { useUmbrellaStore } from '@/stores/umbrella';

const rotorStore = useRotorStore();
const umbrellaStore = useUmbrellaStore();

const rotation = computed(() => {
  const dirs = ['<-N', '-', 'N->'];
  return dirs[rotorStore.rotor.rotation + 1];
});
</script>

<style lang="scss" scoped>
main {
  display: grid;
  grid-template-columns: 2fr 1.4fr;
  gap: 1em;

  @include large {
    grid-template-columns: 2fr 2fr;
  }

  @include medium {
    display: flex;
    flex-direction: column;
    gap: 1em;
  }
}

.compass-wrapper {
}

aside {
  display: flex;
  flex-direction: column;
  gap: 1em;
}
</style>
