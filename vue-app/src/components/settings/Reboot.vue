<template>
  <SettingCard title="Neustart">
    <template #icon>
      <Icon icon="fa-solid fa-power-off"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold flex-cc"
        title="Startet den Rotor Controller neu."
        @click="reboot($event)"
      >
        Neu starten
      </button>
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
import { useRouter } from 'vue-router';

const router = useRouter();

function reboot() {
  fetch('/reboot');
  router.push({ name: 'reboot' });
}
</script>
