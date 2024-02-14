<template>
  <SettingCard :title="$t('settings.keyboard.title')">
    <template #icon>
      <Icon icon="fa-solid fa-keyboard"></Icon>
    </template>

    <template #action>
      <ToggleSwitch
        :toggle="uiStore.ui.kbscEnabled"
        @toggle="uiStore.ui.kbscEnabled = !uiStore.ui.kbscEnabled"
      ></ToggleSwitch>
    </template>

    <template #content>
      <p class="txt-dark" style="">
        {{ $t('settings.keyboard.dscr') }}
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
