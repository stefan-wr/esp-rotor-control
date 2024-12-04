<template>
  <SettingCard :title="$t('calibration.voltage.title')">
    <template #icon>
      <Icon icon="fa-solid fa-bolt"></Icon>
    </template>

    <template #action>
      <span class="large monospace">{{ rotorStore.rotor.adc_v.toFixed(3) }}&nbsp;V</span>
    </template>

    <template #content>
      <p class="txt-dark">
        {{ $t('calibration.voltage.dscr') }}
      </p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import { useRotorStore } from '@/stores/rotor';
const rotorStore = useRotorStore();
</script>
