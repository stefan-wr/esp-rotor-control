<template>
  <SettingCard :title="$t('settings.update.title')">
    <template #icon>
      <Icon icon="fa-solid fa-arrows-rotate"></Icon>
    </template>

    <template #content>
      <p class="txt-dark">{{ $t('settings.update.dscr') }}</p>

      <div class="">
        <CardToggleContentTransition :toggle="isFileSelected" style="width: 100%" ref="toggleCard">
          <!-- CHILD A - Choose File-->
          <template #childA>
            <!-- File drop zone -->
            <div class="flex-vst gap-one update-form">
              <ShakeOnToggle
                class="border-box flex-hc txt-dark drop-zone-wrap"
                tabindex="0"
                ref="dropZone"
                @click="firmwareInput.click()"
              >
                <div
                  class="flex-vc flex-cc gap-one flex-grow drop-zone"
                  :class="{ 'drop-hover': isOverDropZone }"
                >
                  <Icon icon="fa-solid fa-file-import" class="large" />
                  <span class="c-align">{{ $t('settings.update.dropzone') }}</span>
                </div>
              </ShakeOnToggle>

              <!-- Error box -->
              <TransitionSlideIn
                @after-leave="toggleCard.applyCurrentWrapHeight()"
                @enter="toggleCard.applyCurrentWrapHeight()"
                @after-enter="dropZone.shake()"
                :toggle="!!hasError"
              >
                <div class="border-box flex-hc gap-one">
                  <Icon icon="fa-solid fa-circle-exclamation" class="large" />
                  <span>{{ getErrorTxt() }}</span>
                </div>
              </TransitionSlideIn>
            </div>
          </template>

          <!-- CHILD B - Start update-->
          <template #childB>
            <div class="flex-vst gap-one flex-grow update-form">
              <!-- Selected file info-->
              <div class="flex-vst gap-one border-box">
                <!-- Name -->
                <div class="flex-csp gap-one">
                  <span class="txt-dark" style="align-self: start">{{ $t('commons.file') }}:</span>
                  <div class="firmware-name gap-half r-align">
                    <span
                      class="no-wrap-ellip"
                      style="width: 100%"
                      :title="$t('commons.file') + ': ' + settingsStore.firmware.name"
                    >
                      {{ settingsStore.firmware.name }}
                    </span>
                    <span
                      class="small txt-dark wrd-break"
                      :title="'MD5-Hash: ' + settingsStore.firmware.md5"
                    >
                      #{{ settingsStore.firmware.md5 }}
                    </span>
                  </div>
                </div>

                <!-- Size -->
                <div class="flex-csp gap-one">
                  <span class="txt-dark">{{ $t('commons.size') }}:</span>
                  <span>{{ settingsStore.getFirmwareKBytes }} KB</span>
                </div>
              </div>

              <!-- Buttons -->
              <div class="flex-cst gap-one flex-grow">
                <!-- Cancel BTN-->
                <button
                  class="btn-std-resp bold no-wrap-ellip flex-grow update-btn"
                  :title="$t('commons.cancel')"
                  @click="cancelUpdate"
                >
                  <Icon icon="fa-solid btn-std-resp fa-xmark" />&nbsp;{{ $t('commons.cancel') }}
                </button>
                <!-- Update BTN-->
                <button
                  class="btn-std-resp bold no-wrap-ellip flex-grow update-btn"
                  :title="$t('settings.update.updateBtnDscr')"
                  @click="startUpdate"
                >
                  <Icon icon="fa-solid fa-check"></Icon>&nbsp;{{ $t('settings.update.updateBtn') }}
                </button>
              </div>
            </div>
          </template>
        </CardToggleContentTransition>
      </div>

      <!-- Hidden form for file input -->
      <form class="hide" method="post" action="/update" enctype="multipart/form-data">
        <input
          id="firmware-input"
          ref="firmwareInput"
          type="file"
          name="firmware"
          accept=".bin"
          @change="onFileChange"
          hidden
        />
      </form>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import TransitionSlideIn from '@/components/TransitionSlideIn.vue';
import ShakeOnToggle from '@/components/ShakeOnToggle.vue';
import CardToggleContentTransition from '@/components/CardToggleContentTransition.vue';

import { ref, watch } from 'vue';
import { useDropZone } from '@vueuse/core';
import md5 from 'js-md5';

import { useRouter } from 'vue-router';
import { useSettingsStore } from '@/stores/settings';

import { useI18n } from "vue-i18n";
const { t } = useI18n();

const settingsStore = useSettingsStore();
const router = useRouter();

// Refs
const toggleCard = ref(null);
const firmwareInput = ref(null);
const dropZone = ref(null);

const isFileSelected = ref(false);
const hasError = ref(0);

// Drop files area
function onDrop(files) {
  onFileChange(null, files);
}

const { isOverDropZone } = useDropZone(dropZone, {
  onDrop,
  dataTypes: ['application/octet-stream']
});

// Handler fore change-event of file input, can also be called with arguments instead.
// Performs checks wether given file is a valid firmware file
async function onFileChange(event, files = []) {
  resetFirmwareValues();
  let fileList = [];

  if (files.length !== 0) {
    fileList = files;
  } else {
    fileList = firmwareInput.value.files;
  }
  let error = 0;

  // Make sure just one file was selected
  if (fileList.length !== 1) {
    error = 1;
  }

  // Make sure file is a .bin file
  if (!error && fileList[0].name.split('.').pop() !== 'bin') {
    error = 2;
  }

  // Make sure firmware is not empty
  if (!error && fileList[0].size === 0) {
    error = 3;
  }

  // Make sure firmware fits into ESP partition
  if (!error && fileList[0].size > 1638400) {
    error = 4;
  }

  // Get hash and make sure it has correct length
  let hash;
  if (!error) {
    hash = await getMD5(fileList[0]);
    if (hash.length !== 32) {
      error = 5;
    }
  }

  // All checks passed at this point.
  // Save selected file parameters in store.
  if (!error) {
    settingsStore.firmware.file = fileList[0];
    settingsStore.firmware.name = fileList[0].name;
    settingsStore.firmware.size = fileList[0].size;
    settingsStore.firmware.md5 = hash;
    // Toggle card
    hasError.value = 0;
    isFileSelected.value = true;
  }

  // An error occured, set error text, shake form
  hasError.value = 0;
  if (error) {
    hasError.value = error;
  }
  dropZone.value.shake();
}

// Get translated error message
function getErrorTxt() {
  if (hasError.value) {
    return t('settings.update.error' + hasError.value);
  } else {
    return 'errorTxt.value;'
  }
}

// Get MD5 hash from selected file.
// Need to read file first.
function getMD5(file) {
  // Async file reader
  const reader = new FileReader();

  return new Promise((resolve, reject) => {
    // On event, file read succesfully
    reader.onload = (e) => {
      resolve(md5(reader.result));
    };

    // On event, file read failed
    reader.onerror = (e) => {
      console.log(reader.error);
      reject('--');
    };

    // Start reading file
    reader.readAsArrayBuffer(file);
  });
}

// Cancel update
function cancelUpdate() {
  isFileSelected.value = false;
}

function resetFirmwareValues() {
  settingsStore.firmware.name = '--';
  settingsStore.firmware.size = 0;
  settingsStore.firmware.md5 = '';
}

// Start update
function startUpdate() {
  router.push({ name: 'update' });
}

// Reapply height for toggleCard after error changes
watch(
  hasError,
  () => {
    toggleCard.value.applyCurrentWrapHeight();
  },
  { flush: 'post' }
);
</script>

<style lang="scss" scoped>
.drop-zone {
  min-height: 6em;
  cursor: pointer;
  transition: transform 0.1s;
  transform: scale(100%);

  &:hover {
    color: var(--text-color);
    transform: scale(105%);
  }

  &.drop-hover {
    color: var(--text-color);
    transform: scale(105%);
    cursor: grabbing;
  }
}

.drop-zone-wrap:focus-visible {
  outline: 2px solid var(--text-color);
  outline-offset: -0.25em;
  .drop-zone {
    color: var(--text-color);
    transform: scale(105%);
  }
}

.update-form {
  width: 100%;
}

.firmware-name {
  display: grid;
  justify-items: end;
  width: 100%;
}

.update-btn {
  flex-basis: 48%;
  flex-shrink: 1;
}
</style>
