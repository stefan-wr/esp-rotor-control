<template>
  <main class="flex-vst gap-one">
    <h2 class="medium">Firmware Udpate</h2>
    <p>
      Bitte warte, während das Firmware Update hochgeladen wird.<br>Nach erfolgreichem Update wird der
      RotorControl neugestartet.
    </p>

    <!-- Upload Icon -->
    <div class="massive">
      <Icon icon="fa-solid fa-arrows-rotate" :class="{ spin: !uploadFinished }"></Icon>
    </div>

    <!-- Success Box-->
    <!---------------->
    <MessageBoxTransition :toggle="!!successTxt">
      <div class="flex-vc gap-one">
        <div class="border-box flex-hc l-lign gap-one error">
          <Icon icon="fa-solid fa-circle-info" class="large" />
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
    </MessageBoxTransition>

    <!-- Error Box -->
    <!--------------->
    <MessageBoxTransition :toggle="!!errorTxt" :shake="true" v-slot="{ isShaking }">
      <div class="flex-vc gap-one">
        <div class="border-box flex-hc l-align gap-one error" :class="{ 'form-shake': isShaking }">
          <Icon icon="fa-solid fa-circle-info" class="large" />
          <span>{{ errorTxt }}</span>
        </div>
        <!-- Button -->
        <button class="btn-std-resp bold no-wrap-ellip" @click="returnToSettings()">
          <Icon icon="fa-solid fa-arrow-left" />&nbsp;Zurück
        </button>
      </div>
    </MessageBoxTransition>

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
import MessageBoxTransition from '@/components/MessageBoxTransition.vue';

import axios from 'axios';

import { ref, onMounted, computed } from 'vue';
import { useRouter } from 'vue-router';
import { useSettingsStore } from '@/stores/settings';
import { useUmbrellaStore } from '@/stores/umbrella';

const router = useRouter();
const settingsStore = useSettingsStore();
const umbrellaStore = useUmbrellaStore();

const uploadProgress = ref(1);
const uploadFinished = ref(false);
const rebootTimerActive = ref(false);

const errorTxt = ref('');
const successTxt = ref('');
const errorShaking = ref(false);

// Update upload progress up to 94%
function updateProgress(e) {
  let progress = ((e.loaded / e.total) * 100).toFixed(0);
  if (progress <= 94) {
    uploadProgress.value = progress;
  }
}

const rebootPending = computed(() => {
  if (rebootTimerActive.value) {
    return true;
  } else if (umbrellaStore.hasLostConnection) {
    return true;
  } else {
    return false;
  }
});

// Upload new firmware file
// ------------------------
async function uploadFirmware() {
  uploadFinished.value = false;

  // Create data object
  let formData = new FormData();
  formData.append('firmware', settingsStore.firmware.file, settingsStore.firmware.name);

  // Create config object
  let config = {
    onUploadProgress: updateProgress,
    timeout: 1000 * 60 * 2,
    headers: {
      'Content-Type': 'multipart/form-data'
    }
  };

  // Configure Axios, cache common headers
  let commonHeaders = axios.defaults.headers.common;
  axios.defaults.withCredentials = true;
  axios.defaults.headers.common['Firmware-MD5'] = settingsStore.firmware.md5;
  axios.defaults.headers.common['Firmware-Size'] = settingsStore.firmware.size;

  // --------------------
  try {
    const response = await axios.post('http://rotor.local/update', formData, config);
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

  } catch (error) {
    // Upload failed
    uploadFinished.value = true;
    errorTxt.value = 'Update fehlgeschlagen! Die Firmware-Datei konnte nicht hochgeladen werden.';
    console.log(error);

  } finally {
    // Reset common headers
    axios.defaults.headers.common = commonHeaders;
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

/* Error text transition */
$speed: 0.2s;

.error-leave-active,
.error-enter-active {
  transition:
    transform $speed,
    opacity $speed;
}
.error-leave-to,
.error-enter-from {
  opacity: 0;
  transform: translateX(-80%) scaleY(70%);
}
</style>
