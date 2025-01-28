<template>
  <CardCollapsable id="speed-control" :title="$t('speed.title')">
    <template #icon><Icon icon="fa-solid fa-gauge-high"></Icon></template>
    <div class="range-wrap">
      <input
        v-model.number="rotorStore.rotor.speed"
        id="speed"
        type="range"
        name="speed"
        min="0"
        max="100"
        step="10"
        @change="umbrellaStore.sendSpeed()"
        @keydown="preventRepeat($event)"
        :disabled="settingsStore.isLockedByElse"
      />
      <div class="ticks flex-csp no-select">
        <div v-for="n in 11" class="no-select"></div>
      </div>
      <label class="xxlarge range-lbl r-align" for="speed">
        {{ speedLabel }}<span class="smaller">&nbsp;%</span>
      </label>
    </div>
  </CardCollapsable>
</template>

<script setup>
import CardCollapsable from '@/components/CardCollapsable.vue';

import { computed } from 'vue';
import { useEventListener } from '@vueuse/core';

import { useUmbrellaStore } from '@/stores/umbrella';
import { useSettingsStore } from '@/stores/settings';
import { useRotorStore } from '@/stores/rotor';
import { useUIStore } from '@/stores/ui';

const umbrellaStore = useUmbrellaStore();
const settingsStore = useSettingsStore();
const rotorStore = useRotorStore();
const uiStore = useUIStore();

// Speed label, display 0 as 1%
const speedLabel = computed(() => {
  if (rotorStore.rotor.speed == 0) {
    return 1;
  } else {
    return rotorStore.rotor.speed;
  }
});

// Event listener for setting speed with up/down arrow keys
function speedKeyEventListener(event) {
  if (!event.repeat && uiStore.ui.kbscEnabled && event.target.tagName !== 'INPUT' && !settingsStore.isLockedByElse) {
    switch (event.key) {
      case 'ArrowUp':
        if (rotorStore.rotor.speed < 100) {
          rotorStore.rotor.speed += 10;
          umbrellaStore.sendSpeed();
        }
        event.preventDefault();
        break;
      case 'ArrowDown':
        if (rotorStore.rotor.speed > 0) {
          rotorStore.rotor.speed -= 10;
          umbrellaStore.sendSpeed();
        }
        event.preventDefault();
        break;
    }
  }
}

// Block key-hold on slider
function preventRepeat(event) {
  if (event.repeat) {
    event.preventDefault();
  }
}

// Register arrow keys event listener
useEventListener(document, 'keydown', speedKeyEventListener);
</script>

<style lang="scss">
/* Range Input */
/* ----------- */
:root {
  --range-total-height: 32px;
  --range-total-padding: 4px;
  --range-total-bradius: 6px;
  --range-track-height: 4px;
  --range-thumb-width: 24px;
  --range-thumb-bradius: 4px;
  --range-thumb-height: calc(var(--range-total-height) - 2 * var(--range-total-padding));
  --range-track-margin: calc(var(--range-thumb-width) / 2);
  --range-tick-width: 2px;
  --range-tick-height: 6px;
  --range-tick-margin: 2px;

  @media (hover: none) {
    --range-total-height: 44px;
    --range-total-padding: 6px;
    --range-thumb-width: 44px;
  }
}

.range-wrap {
  display: grid;
  grid-template-columns: 1fr 7em;
  justify-items: stretch;
  align-items: center;
  column-gap: 1em;
  row-gap: var(--range-tick-margin);

  input[type='range'] {
    -webkit-appearance: none;
    appearance: none;
    background: var(--content-color-1);
    height: var(--range-total-height);
    padding: var(--range-total-padding);
    border-radius: var(--range-total-bradius);
    margin: 0;
    width: 100%;
    min-width: 100px;
  }

  input[type='range']:hover {
    cursor: pointer;
    background: var(--content-color-0);
  }

  input[type='range']:disabled:hover {
    background: var(--content-color-1);
  }

  /* Range-Track */
  input[type='range']::-moz-range-track {
    margin: 0 var(--range-track-margin);
    height: var(--range-track-height);
    border-radius: var(--range-track-height);
    background-color: var(--content-color-2);
  }

  input[type='range']::-webkit-slider-runnable-track {
    height: var(--range-track-height);
    border-radius: var(--range-track-height);
    background-color: var(--content-color-2);
  }

  /* Range-Thumb */
  input[type='range']::-moz-range-thumb {
    cursor: pointer;
    height: var(--range-thumb-height);
    width: var(--range-thumb-width);
    border-radius: var(--range-thumb-bradius);
    border: none;
    background: var(--text-color);
    box-shadow: none;
  }

  input[type='range']::-webkit-slider-thumb {
    appearance: none;
    margin-top: calc(
      -1 * (var(--range-total-height) / 2 - var(--range-total-padding) - var(--range-track-height) /
            2)
    );
    cursor: pointer;
    height: var(--range-thumb-height);
    width: var(--range-thumb-width);
    border-radius: var(--range-thumb-bradius);
    border: none;
    background: var(--text-color);
    box-shadow: none;
  }

  input[type='range']:disabled::-moz-range-thumb {
    cursor: not-allowed;
  }

  input[type='range']:disabled::-webkit-slider-thumb {
    cursor: not-allowed;
  }

  /* Thumb Hover */
  input[type='range']::-moz-range-thumb:hover {
    filter: brightness(0.9);
  }

  input[type='range']::-webkit-slider-thumb:hover {
    filter: brightness(0.9);
  }

  input[type='range']:disabled::-moz-range-thumb:hover {
    filter: brightness(1.0);
  }

  input[type='range']:disabled::-webkit-slider-thumb:hover {
    filter: brightness(1.0);
  }


  /* Ticks */
  .ticks {
    padding: 0
      calc(var(--range-thumb-width) / 2 + var(--range-total-padding) - var(--range-tick-width) / 2);
  }

  .ticks div {
    width: var(--range-tick-width);
    height: var(--range-tick-height);
    border-radius: var(--range-tick-height);
    background: var(--content-color-0);
    opacity: 0.8;
  }

  /* Label */
  .range-lbl {
    pointer-events:none;
    grid-column: 2;
    grid-row: 1;
  }
}
</style>
