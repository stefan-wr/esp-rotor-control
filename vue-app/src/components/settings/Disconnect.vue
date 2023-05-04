<template>
  <SettingCard title="Netzwerk">
    <template #icon>
      <Icon icon="fa-solid fa-wifi"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold flex-cc"
        @click="disconnect($event)"
        title="Trennt und setzt die Netzwerverbindung des Rotor Controllers zurück."
      >
        Trennen
      </button>
    </template>

    <template #content>
      <div class="flex-hc gap-one" style="width: 100%">
        <span class="ssid no-wrap-ellip larger">{{ settingsStore.settings.ssid }}</span>
        <span class="smaller">RSSI:&nbsp;{{ settingsStore.settings.rssi }}</span>
      </div>
      <hr />
      <p class="txt-dark" style="">
        <b>ACHTUNG!</b><br />
        Wird die Verbindung getrennt, ist ein Fernzugriff nicht mehr möglich! Die gespeicherten
        Zugangsdaten werden gelöscht.
      </p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import { useSettingsStore } from '../../stores/settings';
import { useRouter } from 'vue-router';

const settingsStore = useSettingsStore();
const router = useRouter();

// Request conmirmation before sending disconnect request
function disconnect(event) {
  let msg = 'ACHTUNG!\nWirklich die Netzwerkverbindung trennen?\n\n';
  msg += 'Ein Fernzugriff ist dann nicht mehr möglich.\n';
  msg += 'Der Rotor Controller muss danach lokal neu eingerichtet werden.';
  if (confirm(msg)) {
    fetch('/disconnect');
    router.push({ name: 'disconnect' });
  }
}
</script>

<style lang="scss" scoped>
.ssid {
  flex-shrink: 1;
  flex-grow: 1;
}
</style>
