<template>
  <main>
    <div class="compass-wrapper">
      <Compass />
    </div>
    <aside>
      <h1>{{ rotation }}</h1>
      <button
        @mousedown.prevent="store.rotor[rot] = -1"
        @mouseup.prevent="store.setRotation(0, $event)"
      >
        &lt;-
      </button>
      <button
        @mousedown.prevent="store.setRotation(1, $event)"
        @mouseup.prevent="store.setRotation(0, $event)"
      >
        -&gt;
      </button>
    </aside>
  </main>
</template>

<script setup>
import Header from '@/components/Header.vue';
import Compass from '@/components/Compass.vue';

import { ref, computed } from 'vue';
import { useRotorStore } from '@/stores/rotor';

const store = useRotorStore();
const rot = 'rotation';

const rotation = computed(() => {
  var dirs = ['<-N', '-', 'N->'];
  return dirs[store.rotor.rotation + 1];
});
</script>

<style scoped>
main {
  display: grid;
  grid-template-columns: 2fr 1fr;
  gap: 1em;
}

aside {
  width: 30%;

}

</style>
