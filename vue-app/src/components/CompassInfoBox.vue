<template>
    <div class="compass-labels border-box">
      <div class="flex-vc gap-half">
        <span class="small txt-dark">{{ $t('compass.position') }}</span>
        <span class="label-value large bold monospace">{{ rotorStore.angle1D }}°</span>
      </div>
      <div class="flex-vc gap-half">
        <span class="small txt-dark">{{ $t('compass.targetPosition') }}</span>
        <span class="label-value large bold monospace">{{ target }}</span>
      </div>
      <div class="flex-vc gap-half">
        <span class="small txt-dark">{{ $t('compass.time') }}</span>
        <span
          class="label-value medium bold monospace flex-cc no-select"
          :title="$t('compass.timeDscr')"
        >
          {{ currentTime }}
        </span>
      </div>
    </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue';
import { useRotorStore } from '@/stores/rotor';
const rotorStore = useRotorStore();

// Target position, null replaced with dashes
// ------------------------------------------
const target = computed(() => {
  if (rotorStore.rotor.target === null) {
    return '--';
  }
  return String(rotorStore.rotor.target) + '°';
});

// Current time label
// ------------------
const today = ref(new Date());
const currentTime = computed(() => {
  let time = String(today.value.getHours()).padStart(2, '0') + ':';
  time += String(today.value.getMinutes()).padStart(2, '0') + ':';
  time += String(today.value.getSeconds()).padStart(2, '0');
  return time;
});

// Enable update time with mount
let intervalId;
onMounted(() => {
  intervalId = setInterval(() => {
    today.value = new Date();
  }, 100);
});

// Disable update time on unmount
onUnmounted(() => {
  clearInterval(intervalId);
});
</script>

<style lang="scss" scoped>
.compass-labels {
  display: flex;
  flex-wrap: wrap;
  justify-content: space-around;
  align-items: center;
  gap: 1em;

  .label-value {
    min-height: 2rem;
    min-width: 5.5rem;
    align-self: center;
  }
}
</style>