<template>
  <SettingCard title="Netzwerk">
    <template #icon>
      <Icon icon="fa-solid fa-wifi" style="margin-right: 0.5em"></Icon>
    </template>

    <template #action>
      <a
        class="button btn-std-resp bold flex-cc"
        href="%DISCONNECT_URL%"
        @click="disconnect($event)"
        title="Trennt und setzt die Netzwerverbindung des Rotor Controllers zurück."
        >Trennen</a
      >
    </template>

    <template #content>
      <div class="flex-hc gap-one" style="width: 100%">
        <span class="ssid no-wrap-ellip larger">Mumpitz</span>
        <span class="smaller">RSSI:&nbsp;-29</span>
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

function disconnect(event) {
  let msg = 'ACHTUNG!\nWirklich die Netzwerkverbindung trennen?\n\n';
  msg += 'Ein Fernzugriff ist dann nicht mehr möglich.\n';
  msg += 'Der Rotor Controller muss danach lokal neu eingerichtet werden.';
  if (!confirm(msg)) event.preventDefault();
}
</script>

<style lang="scss" scoped>
.ssid {
  flex-shrink: 1;
  flex-grow: 1;
}
</style>
