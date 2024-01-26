<template>
  <main class="flex-vst gap-one">
    <h2 class="medium">Firmware Udpate</h2>
    <p>
      Bitte warte, während das Firmware Update hochgeladen wird.<br />Nach erfolgreichem Update wird
      RotorControl neugestartet.
    </p>

    <!-- Upload Icon -->
    <div class="massive">
      <Icon icon="fa-solid fa-arrows-rotate" :class="{ spin: !uploadFinished }"></Icon>
    </div>

    <!-- Success Box-->
    <!---------------->
    <TransitionSlideIn :toggle="!!successTxt">
      <div class="flex-vc gap-one">
        <div class="border-box flex-hc l-align gap-one error">
          <Icon icon="fa-solid fa-circle-check" class="large" />
          <span>{{ successTxt }}</span>
        </div>
        <!-- Button-->
        <button class="btn-std-resp bold" @click="returnToSettings()" :disabled="rebootPending">
          <span v-if="rebootPending">Warte auf Neustart</span>
          <span v-else>Update abschließen</span>
          &nbsp;
          <Icon icon="fa-solid fa-spinner" class="spin" v-if="rebootPending"></Icon>
          <Icon icon="fa-solid fa-arrow-right" v-else></Icon>
        </button>
      </div>
    </TransitionSlideIn>

    <!-- Error Box -->
    <!--------------->
    <TransitionSlideIn :toggle="!!errorTxt" @after-enter="errorBox.shake()">
      <div class="flex-vc gap-one">
        <ShakeOnToggle ref="errorBox" class="border-box flex-hc l-align gap-one error">
          <Icon icon="fa-solid fa-circle-exclamation" class="large" />
          <span>{{ errorTxt }}</span>
        </ShakeOnToggle>
        <!-- Button -->
        <button class="btn-std-resp bold no-wrap-ellip" @click="returnToSettings()">
          <Icon icon="fa-solid fa-arrow-left" />&nbsp;Zurück
        </button>
      </div>
    </TransitionSlideIn>

    <!-- Progress Bar-->
    <div class="border-box flex-cst gap-half progress-bar">
      <div class="border-box flex-hl pb-bar">
        <div :style="'width:' + uploadProgress + '%;'"></div>
      </div>
      <span class="large pb-label r-align">{{ uploadProgress }}%</span>
    </div>
  </main>
</template>

<script setup>
import TransitionSlideIn from '@/components/TransitionSlideIn.vue';
import ShakeOnToggle from '@/components/ShakeOnToggle.vue';

import axios from 'axios';

import { ref, onMounted, computed } from 'vue';
import { useRouter } from 'vue-router';
import { useSettingsStore } from '@/stores/settings';
import { useUmbrellaStore } from '@/stores/umbrella';

const router = useRouter();
const settingsStore = useSettingsStore();
const umbrellaStore = useUmbrellaStore();

const errorBox = ref(null);

const uploadProgress = ref(1);
const uploadFinished = ref(false);
const rebootTimerActive = ref(false);

const errorTxt = ref('');
const successTxt = ref('');

// Is reboot pending after update
const rebootPending = computed(() => {
  return rebootTimerActive.value || umbrellaStore.hasLostConnection;
});

// Update upload progress up to 94%
function updateProgress(e) {
  let progress = ((e.loaded / e.total) * 100).toFixed(0);
  if (progress <= 94) {
    uploadProgress.value = progress;
  }
}

// Request approval to upload firmware update from ESP
// ---------------------------------------------------
async function requestUpdateApproval() {
  let formData = new FormData();
  formData.append('md5', settingsStore.firmware.md5);
  formData.append('size', settingsStore.firmware.size);

  // Axios config
  let config = {
    timeout: 1000 * 60,
    withCredentials: true,
    headers: {
      'Content-Type': 'multipart/form-data'
    }
  };

  try {
    var response = await axios.post('/request-update', formData, config);
  } catch (err) {
    throw new Error('Verbindungsfehler bei der Update-Anfrage', { cause: err });
  }

  if (response.data === 'denied') {
    throw new Error('Update-Anfrage von RotorControl abgelehnt');
  }

  if (response.data === '') {
    throw new Error('Leeren Token von RotorControl erhalten');
  }

  return response.data;
}

// Upload new firmware file
// ------------------------
async function uploadFirmware() {
  uploadFinished.value = false;

  // Get approval from server to upload firmware
  try {
    var token = await requestUpdateApproval();
    console.log('Token: ' + token);
  } catch (err) {
    uploadFinished.value = true;
    errorTxt.value = 'Update fehlgeschlagen! (' + err + ')';
    return;
  }

  // Create form data object
  let formData = new FormData();
  formData.append('firmware', settingsStore.firmware.file);

  // Axios config
  let config = {
    timeout: 1000 * 60 * 2,
    withCredentials: true,
    onUploadProgress: updateProgress,
    headers: {
      'Content-Type': 'multipart/form-data',
      Token: token
    }
  };

  // --------------------
  try {
    const response = await axios.post('/update', formData, config);
    uploadFinished.value = true;

    if (response.data === 'success') {
      // Update succesful
      uploadProgress.value = 100;
      rebootTimerActive.value = true;
      successTxt.value = 'Update erfolgreich durchgeführt. RotorControl wird neu gestartet.';
      setTimeout(() => {
        rebootTimerActive.value = false;
      }, 10000);
    } else {
      // Update failed on ESP
      errorTxt.value =
        'Update fehlgeschlagen! Die Firmware-Datei wurde nicht akzeptiert. (' + response.data + ')';
      console.log(response);
    }
  } catch (err) {
    // Upload failed
    uploadFinished.value = true;
    errorTxt.value = 'Update fehlgeschlagen! Die Firmware-Datei konnte nicht hochgeladen werden.';
    console.log(err);
  }
}
// --------------------

// Return to settings page
function returnToSettings() {
  router.push({ name: 'settings' });
}

// Start upload when component mounts
onMounted(() => {
  uploadFirmware();
});
</script>

<style lang="scss" scoped>
main {
  overflow: hidden;
}

.progress-bar {
  .pb-bar {
    width: 100%;
    border-radius: 100em;
    padding: 0.5em !important;

    div {
      height: 1em;
      background-color: var(--accent-color);
      border-radius: 100em;
      transition: width 0.1s;
    }
  }

  .pb-label {
    width: 3em;
  }
}

.error,
.success {
  align-self: center;
}
</style>
