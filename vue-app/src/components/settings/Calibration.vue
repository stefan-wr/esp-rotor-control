<template>
  <div class="flex-vst gap-one">
    <h2 class="medium">Kalibrierung</h2>
    <ADC></ADC>
    <CalibrationCurrent></CalibrationCurrent>

    <SettingCard title="Rotor-Kalibrierung" class="l-align">
      <template #icon>
        <Icon icon="fa-solid fa-wrench"></Icon>
      </template>

      <template #content>
        <!-- Description -->
        <p class="txt-dark">
          Damit der Rotor-Controller die richtige Position anzeigt, muss er kalibiert werden. Wähle
          entweder die geführte Kalibrierung, oder gebe die Parameter manuell ein.
        </p>

        <!-- Tabs Navigation -->
        <div id="calibration">
          <nav class="flex-cl">
            <button
              class="cal-tab normal"
              :class="{ 'cal-tab-active': !isManualTabActive }"
              @click="setTabActive(0)"
            >
              <span class="txt-dark">Geführt</span>
            </button>
            <button
              class="cal-tab normal"
              :class="{ 'cal-tab-active': isManualTabActive }"
              @click="setTabActive(1)"
            >
              <span class="txt-dark">Manuell</span>
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
  </div>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import CardToggleContentTransition from '@/components/CardToggleContentTransition.vue';
import CalibrationAutomatic from '@/components/settings/CalibrationAutomatic.vue';
import CalibrationManual from '@/components/settings/CalibrationManual.vue';
import CalibrationCurrent from '@/components/settings/CalibrationCurrent.vue';
import ADC from '@/components/settings/ADC.vue';

import { useSettingsStore } from '@/stores/settings';
import { useUmbrellaStore } from '@/stores/umbrella';

import { ref } from 'vue';

const settingsStore = useSettingsStore();
const umbrellaStore = useUmbrellaStore();

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
  color: var(--text-color);
  border: none;
  border-radius: calc($border-box-bradius / 2) calc($border-box-bradius / 2) 0 0;
  transition: background-color 0.1s, opacity 0.1s;
  min-height: $button-height;
  padding: 0.5em 1em;

  span {
    display: inline-block;
    transform: scale(85%);
    transition: transform 0.1s, opacity 0.1s;
  }

  &:hover {
    filter: none;
    transform: scale(100%);
    background-color: var(--content-color-3);

    span {
      opacity: 1;
      transform: scale(92.5%);
    }
  }
}

.cal-tab-active {
  background-color: var(--content-color-3);

  span {
    transform: scale(100%);
    color: var(--text-color);
  }

  &:hover {
    span {
      transform: scale(100%);
    }
  }
}

.cal-page {
  width: 100%;
  border-top-left-radius: 0;
}
</style>
