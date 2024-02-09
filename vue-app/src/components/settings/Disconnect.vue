<template>
  <SettingCard title="Netzwerk">
    <template #icon>
      <Icon icon="fa-solid fa-wifi"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold flex-cc"
        title="Trennt und setzt die Netzwerverbindung des Rotor Controllers zurück."
        ref="disconnectBtn"
        @click="disconnect($event)"
      >
      <span v-if="!waitingForDisconnect">Trennen</span>
       <Icon icon="fa-solid fa-spinner" class="spin" v-else></Icon>
      </button>
    </template>

    <template #content>
      <div class="flex-hc gap-one" style="width: 100%">
        <span class="flex-grow no-wrap-ellip larger">{{ settingsStore.settings.ssid }}</span>
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

import { useSettingsStore } from '@/stores/settings';
import { useRouter } from 'vue-router';
import { ref } from 'vue';

const settingsStore = useSettingsStore();
const router = useRouter();

const disconnectBtn = ref(null);
const waitingForDisconnect = ref(false);

// Request conmirmation before sending disconnect request
function disconnect(event) {
  let msg = 'ACHTUNG!\nWirklich die Netzwerkverbindung trennen?\n\n';
  msg += 'Ein Fernzugriff ist dann nicht mehr möglich.\n';
  msg += 'Der RotorControl muss danach lokal neu eingerichtet werden.';
  if (confirm(msg)) {
    disconnectBtn.value.style.width = String(disconnectBtn.value.offsetWidth) + 'px';
    waitingForDisconnect.value = true;
    fetch('/disconnect')
      .then(() => {
        router.push({ name: 'disconnect' });
      })
      .catch(() => {
        router.push({ name: 'disconnect-failed' });
      });
  }
}
</script>

<style lang="scss" scoped>

</style>
