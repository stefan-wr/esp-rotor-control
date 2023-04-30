<template>
  <SettingCard title="Aktuelle Kalibrierung" class="l-align">
    <template #icon>
      <Icon icon="fa-solid fa-compass" style="margin-right: 0.5em"></Icon>
    </template>
    <template #content>
      <p class="txt-dark">Die aktuell verwendeten Kalibrations-Parameter.</p>

      <div class="flex-cst gap-half current-cal">
        <div class="border-box flex-csp gap-one flex-grow current-cal-pos">
          <span class="small txt-dark no-wrap">Position 1:</span>
          <span class="no-wrap r-align">{{ settingsStore.settings.cal.a1 }}°</span>
          <span class="no-wrap r-align">{{ settingsStore.settings.cal.u1 }} V</span>
        </div>

        <div class="border-box flex-csp gap-one flex-grow current-cal-pos">
          <span class="small txt-dark no-wrap">Position 2:</span>
          <span class="no-wrap r-align">{{ settingsStore.settings.cal.a2 }}°</span>
          <span class="no-wrap r-align">{{ settingsStore.settings.cal.u2 }} V</span>
        </div>
      </div>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';

import { useSettingsStore } from '@/stores/settings';

const settingsStore = useSettingsStore();
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
  }

  span:first-child {
    flex-grow: 0;
    flex-basis: 5em;
  }
}
</style>
