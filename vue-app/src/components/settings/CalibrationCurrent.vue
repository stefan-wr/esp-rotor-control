<template>
  <SettingCard :title="$t('calibration.current.title')" class="l-align">
    <template #icon>
      <Icon icon="fa-solid fa-compass"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold"
        :title="$t('calibration.current.btnDscr')"
        @click="resetCalibration($event)"
      >
        {{ $t('commons.reset') }}
      </button>
    </template>

    <template #content>
      <p class="txt-dark">{{ $t('calibration.current.dscr') }}</p>

      <div class="flex-cst gap-half current-cal">
        <div class="border-box flex-csp gap-one flex-grow current-cal-pos">
          <span class="small txt-dark no-wrap">Position 1:</span>
          <span class="no-wrap r-align" :title="'Pos 1. ' + $t('commons.azimut')"
            >{{ settingsStore.calibration.a1 }}°</span
          >
          <span class="no-wrap r-align" :title="'Pos 1. ' + $t('commons.voltage')"
            >{{ settingsStore.calibration.u1 }} V</span
          >
        </div>

        <div class="border-box flex-csp gap-one flex-grow current-cal-pos">
          <span class="small txt-dark no-wrap">Position 2:</span>
          <span class="no-wrap r-align" :title="'Pos 2. ' + $t('commons.azimut')"
            >{{ settingsStore.calibration.a2 }}°</span
          >
          <span class="no-wrap r-align" :title="'Pos 2. ' + $t('commons.voltage')"
            >{{ settingsStore.calibration.u2 }} V</span
          >
        </div>
      </div>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';

import { useI18n } from "vue-i18n";
const { t } = useI18n();

import { useUmbrellaStore } from '@/stores/umbrella';
import { useSettingsStore } from '@/stores/settings';
const umbrellaStore = useUmbrellaStore();
const settingsStore = useSettingsStore();

function resetCalibration(event) {
  if (confirm(t('calibration.current.confirmMessage'))) {
    umbrellaStore.sendCalibration(0.0, 0.05, 450, 4.1);
  }
}
</script>

<style lang="scss" scoped>
/* Current Calibration */
.current-cal {
  flex-wrap: wrap;
  flex-shrink: 1;
}

.current-cal-pos {
  flex-basis: 48%;
  flex-shrink: 1;

  span {
    flex-grow: 1;
    flex-basis: 0;
    cursor: help;
  }

  span:first-child {
    flex-grow: 0;
    flex-basis: 5em;
    cursor: default;
  }
}
</style>
