<template>
  <SettingCard :title="$t('settings.network.title')">
    <template #icon>
      <Icon icon="fa-solid fa-wifi"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold flex-cc"
        :title="$t('settings.network.disconnectDscr')"
        ref="disconnectBtn"
        @click="disconnect($event)"
        :disabled="demo"
      >
      <span v-if="!waitingForDisconnect">{{ $t('settings.network.disconnect') }}</span>
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
        <b>{{ $t('settings.network.attention') }}!</b><br />
        {{ $t('settings.network.dscr') }}
      </p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import { useSettingsStore } from '@/stores/settings';
import { useRouter } from 'vue-router';
import { ref } from 'vue';
import { useI18n } from "vue-i18n";

const { t } = useI18n();
const settingsStore = useSettingsStore();
const router = useRouter();

const disconnectBtn = ref(null);
const waitingForDisconnect = ref(false);

const demo = (import.meta.env.MODE === 'demo') ? true : false;

// Request confirmation before sending disconnect request
function disconnect(event) {
  if (demo) return;
  if (confirm(t('settings.network.alert'))) {
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
