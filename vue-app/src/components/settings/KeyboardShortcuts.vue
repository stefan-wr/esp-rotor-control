<template>
  <SettingCard title="Tastatursteuerung">
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
        Erlaubt im Controller-Tab die manuelle Steuerung des Rotors über die Pfeiltasten.
      </p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import ToggleSwitch from '@/components/ToggleSwitch.vue';

import { ref } from 'vue';
import { useUIStore } from '@/stores/ui';

const uiStore = useUIStore();
const toggle = ref(true);
</script>
