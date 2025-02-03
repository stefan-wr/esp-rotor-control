<template>
  <SettingCard :title="$t('calibration.title')" class="l-align">
    <template #icon>
      <Icon icon="fa-solid fa-wrench"></Icon>
    </template>

    <template #action>
      <span class="monospace large">{{rotorStore.rotor.angle.toFixed(1)}}°</span>
    </template>

    <template #content>
      <!-- Description -->
      <p class="txt-dark">
        {{ $t('calibration.dscr') }}
      </p>

      <!-- Tabs Navigation -->
      <div id="calibration">
        <nav class="flex-cl">
          <button
            class="cal-tab normal"
            :class="{ 'cal-tab-active': !isManualTabActive }"
            @click="setTabActive(0)"
          >
            <span class="txt-dark">{{ $t('calibration.guidedTab') }}</span>
          </button>
          <button
            class="cal-tab normal"
            :class="{ 'cal-tab-active': isManualTabActive }"
            @click="setTabActive(1)"
          >
            <span class="txt-dark">{{ $t('calibration.manualTab') }}</span>
          </button>
        </nav>

        <!-- Tabs Content-->
        <div class="border-box cal-page">
          <CardToggleContentTransition :toggle="isManualTabActive" style="width: 100%">
            <!-- Automatic Calibration-->
            <template #childA>
              <CalibrationAutomatic ref="automaticCalibration"></CalibrationAutomatic>
            </template>

            <!-- Manual Calibration -->
            <template #childB>
              <CalibrationManual ref="manualCalibration"></CalibrationManual>
            </template>
          </CardToggleContentTransition>
        </div>
      </div>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import CardToggleContentTransition from '@/components/CardToggleContentTransition.vue';
import CalibrationAutomatic from '@/components/settings/CalibrationAutomatic.vue';
import CalibrationManual from '@/components/settings/CalibrationManual.vue';

import { useRotorStore } from '@/stores/rotor';
import { ref } from 'vue';

const rotorStore = useRotorStore();

// Toggle between manual- and auto calibration tabs
// ------------------------------------------------
const automaticCalibration = ref(null);
const manualCalibration = ref(null);

const isManualTabActive = ref(false);

function setTabActive(tab) {
  manualCalibration.value.reset();
  automaticCalibration.value.reset();
  isManualTabActive.value = tab ? true : false;
}
</script>

<style lang="scss" scoped>
#calibration {
  width: 100%;
  align-self: center;

  nav {
    gap: 0.3em;
  }
}

/* Tabs */
.cal-tab {
  background-color: var(--content-color-1);
  border: none;
  border-radius: calc($border-box-bradius / 2) calc($border-box-bradius / 2) 0 0;
  min-height: $button-height;
  padding: 0.5em 1em;
  flex-grow: 0;
  transition:
    background-color 0.1s,
    flex-grow 0.2s;

  span {
    display: inline-block;
    transform: scale(85%);
    transition: transform 0.1s;
  }

  &:hover {
    filter: none;
    transform: none;
    background-color: var(--content-color-3);
  }

  &:not(.cal-tab-active):hover,
  &:not(.cal-tab-active):focus-visible {
    span {
      transform: scale(92.5%);
    }
  }
}

.cal-tab-active {
  background-color: var(--content-color-3);
  cursor: default;
  flex-grow: 0.1;

  span {
    transform: scale(100%);
    color: var(--text-color);
    font-weight: bold;
  }
}

.cal-page {
  width: 100%;
  border-top-left-radius: 0;
}
</style>
