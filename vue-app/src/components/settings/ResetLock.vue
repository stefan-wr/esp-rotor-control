<template>
  <SettingCard :title="$t('lock.title')">
    <template #icon>
      <Icon v-if="settingsStore.lock.isLocked" icon="fa-solid fa-lock"></Icon>
      <Icon v-else icon="fa-solid fa-lock-open"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold flex-cc"
        :title="$t('settings.lock.btnDscr')"
        @click="resetLock()"
      >
        {{ $t('commons.reset') }}
      </button>
    </template>

    <template #content>
      <p class="txt-dark">{{ $t('settings.lock.dscr') }}</p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import { useSettingsStore } from '@/stores/settings';
import { useI18n } from "vue-i18n";

const { t } = useI18n();
const settingsStore = useSettingsStore();

function resetLock() {
  settingsStore.resetLock();
  alert(t('settings.lock.alert'));
}
</script>
