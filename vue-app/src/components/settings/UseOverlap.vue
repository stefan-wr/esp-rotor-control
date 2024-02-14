<template>
  <SettingCard title="Overlap">
    <template #icon>
      <Icon icon="fa-solid fa-circle-half-stroke"></Icon>
    </template>

    <template #action>
      <ToggleSwitch
        :toggle="uiStore.ui.useOverlap"
        @toggle="uiStore.ui.useOverlap = !uiStore.ui.useOverlap"
      ></ToggleSwitch>
    </template>

    <template #content>
      <p class="txt-dark" style="">
        {{ $t('settings.overlap.dscr') }}
      </p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import ToggleSwitch from '@/components/ToggleSwitch.vue';
import { useUIStore } from '@/stores/ui';
const uiStore = useUIStore();
</script>
