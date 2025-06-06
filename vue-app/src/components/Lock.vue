<template>
  <CardCollapsable id="lock" :title="$t('lock.title')">
    <template #icon>
      <Icon v-if="settingsStore.lock.isLocked" icon="fa-solid fa-lock"></Icon>
      <Icon v-else icon="fa-solid fa-lock-open"></Icon>
    </template>
    <div class="flex-vst gap-one">
      <p>
        {{ $t('lock.dscr') }}
      </p>
      <div class="flex-cst gap-one">
        <input
          class="input-std-resp flex-grow"
          type="text"
          :placeholder="$t('lock.namePlaceholder')"
          v-model="settingsStore.lock.name"
          :disabled="settingsStore.lock.isLocked"
          @keypress="restrictInput($event)"
          @paste="restrictPaste($event)"
          @keyup.enter="toggleLock"
        />

        <Icon
          icon="fa-solid fa-arrow-right"
          class="medium"
          style="color: var(--content-color-1)"
        ></Icon>

        <ToggleSwitch
          :toggle="settingsStore.lock.isLocked"
          :disabled="settingsStore.isLockedByElse"
          @toggle="toggleLock"
        >
          <template #on>
            <Icon icon="fa-solid fa-lock switch-lbl"></Icon>
          </template>
          <template #off>
            <Icon icon="fa-solid fa-lock-open switch-lbl"></Icon>
          </template>
        </ToggleSwitch>
      </div>
    </div>

    <div class="normal">
      <CardToggleContentTransition :toggle="!settingsStore.lock.isLocked">
        <template #childA>
          <div class="lock-lbl-wrap">
            <div class="border-box flex-cl gap-half">
              <Icon icon="fa-solid fa-lock"></Icon>
              <span>
                <i18n-t keypath="lock.lockedInfo" scope="global">
                  <template #locked>
                    <b>{{ $t('lock.locked') }}</b>
                  </template>
                </i18n-t>
                <b>{{ settingsStore.lock.by }}</b>
              </span>
            </div>
          </div>
        </template>

        <template #childB>
          <div style="margin-top: -1em; height: 0px"></div>
        </template>
      </CardToggleContentTransition>
    </div>
  </CardCollapsable>
</template>

<script setup>
import CardCollapsable from '@/components/CardCollapsable.vue';
import ToggleSwitch from '@/components/ToggleSwitch.vue';
import CardToggleContentTransition from '@/components/CardToggleContentTransition.vue';

import { useSettingsStore } from '@/stores/settings';

const settingsStore = useSettingsStore();
const maxNameLength = 15;

// Toggle lock status
function toggleLock() {
  if (settingsStore.lock.isLocked) {
    // Unlock
    if (settingsStore.lock.name === settingsStore.lock.by) {
      settingsStore.openLock();
    }
  } else {
    // Lock
    settingsStore.closeLock();
  }
}

// Restrict typing-in more than 15 characters
function restrictInput(event) {
  if (settingsStore.lock.name.length >= maxNameLength) {
    event.preventDefault();
  }
}

// Restrict name length to 15 characters AFTER paste
function restrictPaste(event) {
  setTimeout(() => {
    if (settingsStore.lock.name.length > maxNameLength) {
      settingsStore.lock.name = settingsStore.lock.name.substring(0, maxNameLength);
    }
  }, 0);
}
</script>

<style lang="scss" scoped>
.switch-lbl {
  margin-left: 0.6em;
}

.lock-lbl-wrap {
  padding-top: 1em;
  width: 100%;
}
</style>
