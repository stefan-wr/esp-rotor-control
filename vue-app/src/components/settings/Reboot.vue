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
import { ref } from 'vue';
import { useRouter } from 'vue-router';

const router = useRouter();

const rebootBtn = ref(null);

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

function reboot() {
  dots(rebootBtn.value);
  fetch('/reboot')
    .then(() => {
      router.push({ name: 'reboot' });
    })
    .catch(() => {
      router.push({ name: 'reboot-failed' });
    });
}
</script>
