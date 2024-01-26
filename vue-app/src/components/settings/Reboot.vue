<template>
  <SettingCard title="Neustart">
    <template #icon>
      <Icon icon="fa-solid fa-power-off"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold flex-cc"
        title="Startet den Rotor Controller neu."
        ref="rebootBtn"
        @click="reboot($event)"
      >
       <span v-if="!waitingForReboot">Neu starten</span>
       <Icon icon="fa-solid fa-spinner" class="spin" v-else></Icon>
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
import { ref } from 'vue';
import { useRouter } from 'vue-router';

const router = useRouter();

const rebootBtn = ref(null);
const waitingForReboot = ref(false);

function reboot() {
  rebootBtn.value.style.width = String(rebootBtn.value.offsetWidth) + 'px';
  waitingForReboot.value = true;
  fetch('/reboot')
    .then(() => {
      router.push({ name: 'reboot' });
    })
    .catch(() => {
      router.push({ name: 'reboot-failed' });
    });
}
</script>
