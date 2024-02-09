<template>
  <SettingCard title="Firmware Version">
    <template #icon>
      <Icon icon="fa-solid fa-microchip"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold flex-cc"
        title="Suche nach verfügbaren Firmware Version."
        ref="searchBtn"
        @click="searchForUpdate()"
      >
        <span v-if="!isSearching">Suchen</span>
        <Icon icon="fa-solid fa-spinner" class="spin" v-else></Icon>
      </button>
    </template>

    <template #content>
      <SmoothHeightWrap ref="contentWrap">
        <div class="flex-vst gap-one">
          <p class="txt-dark">Suche nach vefügbaren Firmware Versionen.</p>

          <!-- Current Firmware -->
          <div class="border-box flex-csp gap-one">
            <span class="txt-dark">Installierte Version:</span>
            <span class="version c-align">{{ settingsStore.settings.version }}</span>
          </div>

          <!-- Available Firmwares -->
          <TransitionSlideIn
            @after-leave="contentWrap.applyHeight()"
            @enter="contentWrap.applyHeight()"
            :toggle="!!firmwares.length"
            ref="toggleCard"
          >
            <div class="firmwares-box">
              <div class="flex-vst gap-one border-box">
                <!-- Header - More info -->
                <div class="flex-csp gap-one">
                  <p class="">Verfügbare Firmware:</p>
                  <a
                    class="flex-cc gap-half link-with-icon txt-dark"
                    href="https://www.wraase.de/rotorcontrol/#firmware"
                    title="Besucht die RotorCotrol Webseite, um mehr Informationen über verfübare Firmware Versionen zu erhalten."
                    target="_blank"
                  >
                    <span>Mehr Info</span>
                    <Icon icon="fa-solid fa-external-link"></Icon>
                  </a>
                </div>

                <!-- List of Firmwares -->
                <div v-for="fw in firmwares" :key="fw.hash" class="border-box flex-vst gap-one">
                  <!-- Version -->
                  <div class="flex-csp gap-one">
                    <span class="txt-dark">Version:</span>
                    <div class="flex-cr gap-half" style="flex-wrap: wrap">
                      <span class="version c-align">{{ fw.version }}</span>

                      <span
                        class="version c-align"
                        v-if="compareVersionWithCurrent(fw.version, settingsStore.settings.version) === 0"
                        >Installiert&nbsp;
                        <Icon icon="fa fa-solid fa-check"></Icon>
                      </span>

                      <span
                        class="version c-align"
                        v-if="compareVersionWithCurrent(fw.version, settingsStore.settings.version) === -1"
                        >Alt
                      </span>

                      <span
                        class="version c-align"
                        v-if="compareVersionWithCurrent(fw.version, settingsStore.settings.version) === 1"
                        >Neu&nbsp;
                        <Icon icon="fa fa-solid fa-exclamation"></Icon>
                      </span>
                    </div>
                  </div>

                  <hr />

                  <!-- File -->
                  <div class="flex-csp gap-one">
                    <span class="txt-dark" style="align-self: start">Datei:</span>
                    <div class="firmware-name gap-half">
                      <a
                        class="no-wrap-ellip link-with-icon r-align"
                        title="Firmware-Datei herunterladen."
                        :href="fw.url"
                        style="width: 100%"
                        download
                      >
                        <Icon icon="fa-solid fa-download"></Icon>&nbsp; <span>{{ fw.name }}</span>
                      </a>
                      <span class="small txt-dark wrd-break" :title="'MD5-Hash: ' + fw.hash">
                        #{{ fw.hash }}
                      </span>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </TransitionSlideIn>

          <!-- Error box -->
          <TransitionSlideIn
            @after-leave="contentWrap.applyHeight()"
            @enter="contentWrap.applyHeight()"
            @after-enter="errorBox.shake()"
            :toggle="!!errorTxt"
          >
            <ShakeOnToggle ref="errorBox">
              <div class="border-box flex-hc gap-one">
                <Icon icon="fa-solid fa-circle-exclamation" class="large" />
                <span>{{ errorTxt }}</span>
              </div>
            </ShakeOnToggle>
          </TransitionSlideIn>
        </div>
      </SmoothHeightWrap>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import ShakeOnToggle from '@/components/ShakeOnToggle.vue';
import TransitionSlideIn from '@/components/TransitionSlideIn.vue';
import SmoothHeightWrap from '@/components/SmoothHeightWrap.vue';

import { useSettingsStore } from '@/stores/settings';
import { compareVersions } from 'compare-versions';
import { ref, shallowRef } from 'vue';

const settingsStore = useSettingsStore();

const searchBtn = ref(null);
const contentWrap = ref(null);
const errorBox = ref(null);

const isSearching = ref(false);
const firmwares = shallowRef([]);
const errorTxt = ref('');

// Compare semver with current vesion if available.
// Return -2 if current version not available.
function compareVersionWithCurrent(version) {
  if (settingsStore.settings.version === '--') {
    return -2
  } else {
    return compareVersions(version, settingsStore.settings.version);
  }
}

// Search for new firmware versions
async function searchForUpdate() {
  searchBtn.value.style.width = String(searchBtn.value.offsetWidth) + 'px';
  isSearching.value = true;

  try {
    // Get json of firmwares from server
    const response = await fetch('https://www.wraase.de/rotorcontrol/firmwares.json');
    let responseJson = await response.json();

    // Check for array and not-emptyness
    if (!Array.isArray(responseJson.firmwares) || responseJson.firmwares.length === 0) {
      throw 'No firmwares found ("firmwares.json" is empty).';
    }

    // Check every firmware
    for (let fw of responseJson.firmwares) {
      // Check if it is an object
      if (typeof responseJson.firmwares[0] != 'object') {
        throw 'No firmware found (first entry in "firmwares.json" is not an object).';
      }

      // Check all keys in firmware, throws error if key is not allowed or a duplicate.
      let allowedKeys = ['version', 'name', 'url', 'hash'];
      for (let key in fw) {
        if (fw.hasOwnProperty(key)) {
          // Check if key is allow
          if (allowedKeys.includes(key)) {
            // Remove found key from allowed keys
            allowedKeys.splice(allowedKeys.indexOf(key), 1);
          } else {
            throw 'No firmware found (key from "firmwares.json" is either a duplicate or not allowed).';
          }
        }
      }

      // Check if all necessary keys were found
      if (allowedKeys.length !== 0) {
        throw 'No firmware found ("firmwares.json" is missing essential keys).';
      }
    }

    // At this point, all checks OK
    errorTxt.value = '';
    firmwares.value = responseJson.firmwares;
  } catch (error) {
    firmwares.value = [];

    // Mnanually shake error box if it is already shown
    if (errorTxt.value) {
      errorBox.value.shake();
    }

    // Set error
    errorTxt.value = 'Die Suche nach verfügbaren Firmwares ist fehlgeschagen.';
    console.error(error);
  } finally {
    isSearching.value = false;
  }
}
</script>

<style lang="scss" scoped>
.firmwares-box {
  width: 100%;
}

.firmware-name {
  display: grid;
  justify-items: end;
}

.link-with-icon {
  &:hover {
    color: var(--text-color);

    span {
      text-decoration: underline;
    }
  }
}

.version {
  font-variant-numeric: tabular-nums;
  padding: 0.2em 0.5em;
  border-radius: 0.5em;
  border: 0.1em solid var(--text-color);
}
</style>
