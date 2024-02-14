<template>
  <SettingCard :title="$t('settings.screen.title')">
    <template #icon>
      <Icon icon="fa-solid fa-display"></Icon>
    </template>

    <template #action>
      <ToggleSwitch
        :toggle="settingsStore.settings.useScreen"
        @toggle="settingsStore.toggleScreen()"
      ></ToggleSwitch>
    </template>

    <template #content>
      <p class="txt-dark" style="">
        {{ $t('settings.screen.dscr') }}
      </p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import ToggleSwitch from '@/components/ToggleSwitch.vue';
import { useSettingsStore } from '@/stores/settings';
const settingsStore = useSettingsStore();
</script>
