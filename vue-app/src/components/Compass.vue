<template>
  <div class="compass border-box">
    <div id="compass-svg">
      <div v-if="rotorStore.isOverlap" id="overlap" class="corner-lbl medium bold">OL</div>
      <div id="rotation" class="corner-lbl bold medium">{{ rotation }}</div>
      <svg
        id="compass"
        :class="{ 'mouse-inside': uiStore.ui.isMouseInCompass }"
        viewBox="0 0 1000 1000"
        style="fill-rule: evenodd"
      >
        <path
          d="M500,0C775.958,0 1000,224.042 1000,500C1000,775.958 775.958,1000 500,1000C224.042,1000 0,775.958 0,500C0,224.042 224.042,0 500,0ZM500,50C748.362,50 950,251.638 950,500C950,748.362 748.362,950 500,950C251.638,950 50,748.362 50,500C50,251.638 251.638,50 500,50Z"
          :style="ringColor"
        />
        <g id="cmp-req-needle" :style="{ transform: 'rotate(' + uiStore.ui.reqAngle + 'deg)' }">
          <path d="M500,15L515,40L515,700L485,700L485,40L" style="fill: var(--text-color-accent)" />
        </g>
        <g id="cmp-needle" :style="{ transform: 'rotate(' + rotorStore.angle1D + 'deg)' }">
          <path d="M500,200L550,500L500,650L450,500L500,200" :style="ringColor" />
        </g>
      </svg>
    </div>
    <div class="compass-label border-box">
      <span class="small">Position</span>
      <span class="small">Richtung</span>
      <span class="small">Autom. Rotation</span>
      <span id="cmp_angle" class="large bold">{{ rotorStore.angle1D }}°</span>
      <span id="cmp_cardinal" class="large bold">{{ rotorStore.cardinal }}</span>
      <span id="cmp_req_angle" class="bold">{{ uiStore.ui.reqAngle.toFixed(1) }}°</span>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue';

import { useUmbrellaStore } from '@/stores/umbrella';
import { useRotorStore } from '@/stores/rotor';
import { useSettingsStore } from '@/stores/settings';
import { useUIStore } from '@/stores/ui';

const umbrellaStore = useUmbrellaStore();
const rotorStore = useRotorStore();
const settingsStore = useSettingsStore();
const uiStore = useUIStore();

const rotation = computed(() => {
  const dirs = ['<-N', 'STOP', 'N->'];
  return dirs[rotorStore.rotor.rotation + 1];
});

function setRequestAngle(event) {
  // Transform mouse position from insde compass
  // bounding-rectangle to polar coordinates.
  const rect = document.getElementById('compass').getBoundingClientRect();
  const x = event.offsetX - rect.width / 2;
  const y = (event.offsetY - rect.height / 2) * -1;
  const radius = Math.sqrt(x * x + y * y);

  // Check wether mouse is outside of compass circle
  if (radius < rect.width / 2) {
    uiStore.ui.isMouseInCompass = true;
    let angle = (Math.atan2(x, y) * 180) / Math.PI;
    if (angle < 0) {
      angle = 360 + angle;
    }
    uiStore.ui.reqAngle = angle;
  } else {
    uiStore.ui.isMouseInCompass = false;
  }
}

const ringColor = computed(() => {
  if (settingsStore.isLockedByElse) {
    return 'fill: var(--alert-color)';
  } else {
    return 'fill: var(--text-color)';
  }
});

onMounted(() => {
  const compass = document.getElementById('compass');
  compass.addEventListener('mousemove', (event) => {
    if (!uiStore.ui.reqAngleLocked) {
      setRequestAngle(event);
    }
  });
});
</script>

<style lang="scss" scoped>
#compass-svg {
  margin-bottom: 1em;
  position: relative;
  z-index: 1;
}

.mouse-inside {
  cursor: pointer;
}

.corner-lbl {
  position: absolute;
  border-radius: 0.5em;
  padding: 0.25em;
  z-index: -1;
  border: 0.2em solid var(--text-color);

  @include large {
    font-size: 1rem;
  }
}

#overlap {
  top: 0;
  right: 0;
  color: var(--alert-color);
  border-color: var(--alert-color);
}

#rotation {
  top: 0;
  left: 0;
  width: 4em;
}

#cmp-req-needle {
  transform-origin: center;
  transform: rotate(0deg);
}

#cmp-needle {
  transform-origin: center;
  transform: rotate(0deg);
  transition: transform 0.1s;
}

.compass-label {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr;
  align-items: center;
  justify-content: center;
  column-gap: 1em;
  row-gap: 0.5em;
}
</style>
