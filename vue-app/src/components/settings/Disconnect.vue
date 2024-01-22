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
        Trennen
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
import { useSettingsStore } from '../../stores/settings';
import { useRouter } from 'vue-router';
import { ref } from 'vue';

const settingsStore = useSettingsStore();
const router = useRouter();

const disconnectBtn = ref(null);

function animatedDots(el) {
  el.textContent = ' ...';
  setTimeout(() => {
    el.textContent = '. ..';
  }, 500);
  setTimeout(() => {
    el.textContent = '.. .';
  }, 1000);
}

function dots(el) {
  el.style.width = String(el.offsetWidth) + 'px';
  animatedDots(el);
  setInterval(() => {
    animatedDots(el);
  }, 1500);
}

// Request conmirmation before sending disconnect request
function disconnect(event) {
  let msg = 'ACHTUNG!\nWirklich die Netzwerkverbindung trennen?\n\n';
  msg += 'Ein Fernzugriff ist dann nicht mehr möglich.\n';
  msg += 'Der Rotor Controller muss danach lokal neu eingerichtet werden.';
  if (confirm(msg)) {
    dots(disconnectBtn.value);
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
