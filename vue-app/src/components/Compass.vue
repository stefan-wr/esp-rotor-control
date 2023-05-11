<template>
  <div class="compass border-box">
    <div id="compass-svg">
      <!--- Corner Labels -->
      <div v-if="rotorStore.rotor.rotation === -1" id="rotation" class="corner-lbl bold medium">
        <div class="rotate-ccw"><Icon icon="fa-solid fa-rotate-left"></Icon></div>
      </div>
      <div v-if="rotorStore.rotor.rotation === 0" id="rotation" class="corner-lbl bold medium">
        STOP
      </div>
      <div v-if="rotorStore.rotor.rotation === 1" id="rotation" class="corner-lbl bold medium">
        <div class="rotate-cw"><Icon icon="fa-solid fa-rotate-right"></Icon></div>
      </div>

      <div v-if="rotorStore.isOverlap" id="overlap" class="corner-lbl medium bold">OL</div>
      <div v-if="settingsStore.lock.isLocked" id="lock" class="corner-lbl bold medium">
        <Icon icon="fa-solid fa-lock"></Icon>
      </div>

      <!-- Compass-->
      <svg
        id="compass"
        ref="compass"
        :class="{ 'mouse-inside': uiStore.ui.isMouseInCompass }"
        viewBox="0 0 1000 1000"
        style="fill-rule: evenodd"
        @click="requestAngle"
      >
        <path
          d="M500,0C775.958,0 1000,224.042 1000,500C1000,775.958 775.958,1000 500,1000C224.042,1000 0,775.958 0,500C0,224.042 224.042,0 500,0ZM500,50C748.362,50 950,251.638 950,500C950,748.362 748.362,950 500,950C251.638,950 50,748.362 50,500C50,251.638 251.638,50 500,50Z"
          :style="ringColor"
        />
        <g id="cmp-req-needle" :style="{ transform: 'rotate(' + uiStore.ui.requestAngle + 'deg)' }">
          <path
            d="M500,15L515,40L515,700L485,700L485,40L"
            style="fill: var(--compass-req-needle-color)"
          />
        </g>
        <g
          v-if="rotorStore.hasTarget"
          id="cmp-target-needle"
          :style="{ transform: 'rotate(' + rotorStore.rotor.target + 'deg)' }"
        >
          <path d="M500,200L550,500L500,650L450,500L500,200" :style="ringColor" />
          >
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
      <span id="cmp_req_angle" class="bold">{{ uiStore.ui.requestAngle.toFixed(1) }}°</span>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, watch } from 'vue';
import { useEventListener } from '@vueuse/core';

import { useUmbrellaStore } from '@/stores/umbrella';
import { useRotorStore } from '@/stores/rotor';
import { useSettingsStore } from '@/stores/settings';
import { useUIStore } from '@/stores/ui';

const umbrellaStore = useUmbrellaStore();
const rotorStore = useRotorStore();
const settingsStore = useSettingsStore();
const uiStore = useUIStore();

const compass = ref(null);

// Get and set request angle from mouse position in compass
// --------------------------------------------------------
function setRequestAngle(event) {
  // Transform mouse position from insde compass
  // bounding-rectangle to polar coordinates.
  const rect = compass.value.getBoundingClientRect();
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
    uiStore.ui.requestAngle = angle;
  } else {
    uiStore.ui.isMouseInCompass = false;
  }
}

// Add event listener to set request angle
useEventListener(compass, 'mousemove', (event) => {
  setRequestAngle(event);
});

watch(
  () => rotorStore.rotor.rotation,
  (newRotation, oldRotation) => {
    if (!oldRotation && !newRotation) {
      uiStore.ui.targetAngleLocked = false;
    }
  }
);

// Change ring color if rotor is locked by someone else
// ----------------------------------------------------
const ringColor = computed(() => {
  if (settingsStore.isLockedByElse) {
    return 'fill: var(--alert-color)';
  } else {
    return 'fill: var(--compass-color)';
  }
});

// Request auto rotation to given angle
// ------------------------------------
function requestAngle() {
  if (uiStore.ui.isMouseInCompass) {
    uiStore.ui.targetAngle = uiStore.ui.requestAngle;
    uiStore.ui.targetAngleLocked = true;
    umbrellaStore.sendTarget(uiStore.ui.requestAngle.toFixed(1));
  }
}
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

#rotation {
  top: 0;
  left: 0;
  width: 4em;

  .rotate-cw {
    animation: 5s linear infinite rotate-cw;
  }

  .rotate-ccw {
    animation: 5s linear reverse infinite rotate-cw;
  }
}

@keyframes rotate-cw {
  0% {
    transform: rotate(0deg);
  }
  100% {
    transform: rotate(360deg);
  }
}

#overlap {
  top: 0;
  right: 0;
  width: 2.3em;
  color: var(--alert-color);
  border-color: var(--alert-color);
}

#lock {
  bottom: 0;
  right: 0;
  width: 2em;
  color: var(--alert-color);
  border-color: var(--alert-color);
}

#cmp-req-needle,
#cmp-target-needle {
  transform-origin: center;
  transform: rotate(0deg);
}

#cmp-needle {
  transform-origin: center;
  transform: rotate(0deg);
  transition: transform 0.1s;
}

#cmp-target-needle {
  opacity: 0.3;
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
