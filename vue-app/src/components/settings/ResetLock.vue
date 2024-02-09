<template>
  <SettingCard title="Rotorsperre">
    <template #icon>
      <Icon v-if="settingsStore.lock.isLocked" icon="fa-solid fa-lock"></Icon>
      <Icon v-else icon="fa-solid fa-lock-open"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold flex-cc"
        title="Setzt die Rotorsperre zurück."
        @click="resetLock()"
      >
        Zurücksetzen
      </button>
    </template>

    <template #content>
      <p class="txt-dark">Setzt die Sperrung des Rotors zurück und gibt diesen wieder frei.</p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import { useSettingsStore } from '@/stores/settings';

const settingsStore = useSettingsStore();

function resetLock() {
  settingsStore.resetLock();
  alert('Die Rotorsperre wurde zurückgesetzt.');
}
</script>
