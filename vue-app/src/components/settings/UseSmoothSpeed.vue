<template>
  <SettingCard :title="$t('settings.smoothSpeed.title')">
    <template #icon>
      <Icon icon="fa-solid fa-gauge-high"></Icon>
    </template>

    <template #action>
      <ToggleSwitch
        :toggle="uiStore.ui.useSmoothSpeed"
        @toggle="uiStore.ui.useSmoothSpeed = !uiStore.ui.useSmoothSpeed"
      ></ToggleSwitch>
    </template>

    <template #content>
      <p class="txt-dark" style="">
        {{ $t('settings.smoothSpeed.dscr') }}
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
