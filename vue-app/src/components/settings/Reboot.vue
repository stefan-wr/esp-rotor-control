<template>
  <SettingCard title="Neustart">
    <template #icon>
      <Icon icon="fa-solid fa-power-off"></Icon>
    </template>

    <template #action>
      <a
        class="button btn-std-resp bold flex-cc"
        href="/reboot"
        title="Startet den Rotor Controller neu."
        >Neu starten</a
      >
    </template>

    <template #content>
      <p class="txt-dark">
        Startet den Rotor Controller neu. Die Netzwerkeinstellungen bleiben erhalten.
      </p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
</script>
