<template>
  <SettingCard :title="$t('settings.reboot.title')">
    <template #icon>
      <Icon icon="fa-solid fa-power-off"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold flex-cc"
        :title="$t('settings.reboot.btnDscr')"
        ref="rebootBtn"
        @click="reboot($event)"
      >
        <span v-if="!waitingForReboot">{{ $t('settings.reboot.btn') }}</span>
        <Icon icon="fa-solid fa-spinner" class="spin" v-else></Icon>
      </button>
    </template>

    <template #content>
      <p class="txt-dark">
        {{ $t('settings.reboot.dscr') }}
      </p>
      <div class="border-box flex-cl gap-half txt-dark">
        <span>{{ $t('settings.reboot.lastBoot') }}:</span>
        <span>{{ lastBootHumanized }}</span>
      </div>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import { computed, ref } from 'vue';
import { useRouter } from 'vue-router';
import { useSettingsStore } from '@/stores/settings';
import { useI18n } from 'vue-i18n';

const router = useRouter();
const settingsStore = useSettingsStore();
const { t } = useI18n();

const rebootBtn = ref(null);
const waitingForReboot = ref(false);

// Send reboot command to ESP
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

// Get human-friendly time string showing on-time
const lastBootHumanized = computed(() => {
  // Value of on-time is still the placeholder
  if (settingsStore.getOnTimeHumanized === null) {
    return '--';
  }

  // Filter: Remove time unit entries that are zero -> we only want to display what is needed
  // Slice: Only display the two biggest time units
  // Map: Get translated time string, entry: ['timeUnit': value]
  const timeString = Object.entries(settingsStore.getOnTimeHumanized)
    .filter((entry) => entry[1] > 0)
    .slice(0, 2)
    .map((entry) => t('settings.reboot.' + entry[0], entry[1]))
    .join(' ' + t('settings.reboot.and') + ' ');
  
  if (timeString === "") {
    return t('settings.reboot.justNow');
  } else {
    return t('settings.reboot.lastBootWrap', { lastBootString: timeString });
  }
});
</script>
