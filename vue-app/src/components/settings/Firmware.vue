<template>
  <SettingCard title="Firmware Version">
    <template #icon>
      <Icon icon="fa-solid fa-microchip"></Icon>
    </template>

    <template #action>
      <span class="large">v{{ settingsStore.settings.version }}</span>
    </template>

    <template #content>
      <p class="txt-dark">Suche nach neuen Firmware Versionen.</p>
      <button class="btn-std-resp bold flex-cc">Nach Update suchen</button>

    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import CardToggleContentTransition from '@/components/CardToggleContentTransition.vue';

import { useRouter } from 'vue-router';
import { useSettingsStore } from '@/stores/settings';
import { useUmbrellaStore } from '@/stores/umbrella';

import { ref } from 'vue';

const settingsStore = useSettingsStore();


</script>

<style lang="scss" scoped>

</style>
