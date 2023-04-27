<template>
  <SettingCard title="ADC-Spannung">
    <template #icon>
      <Icon icon="fa-solid fa-bolt"></Icon>
    </template>

    <template #action>
      <span class="large">{{ rotorStore.rotor.adc_v.toFixed(3) }}&nbsp;V</span>
    </template>

    <template #content>
      <p class="txt-dark">
        Zeigt die gemessene Positions-Spannung des Rotors. Mithilfe der Kalibrierung wird aus dieser Spannung die Rotor-Position berechnet.
      </p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';

import { useRotorStore } from '@/stores/rotor';

const rotorStore = useRotorStore();

</script>
