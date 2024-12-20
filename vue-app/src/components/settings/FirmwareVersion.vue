<template>
  <SettingCard :title="$t('settings.firmware.title')">
    <template #icon>
      <Icon icon="fa-solid fa-microchip"></Icon>
    </template>

    <template #action>
      <button
        class="btn-std-resp bold flex-cc"
        :title="$t('settings.firmware.dscr')"
        ref="searchBtn"
        @click="searchForUpdate()"
      >
        <span v-if="!isSearching">{{ $t('settings.firmware.btn') }}</span>
        <Icon icon="fa-solid fa-spinner" class="spin" v-else></Icon>
      </button>
    </template>

    <template #content>
      <SmoothHeightWrap ref="contentWrap">
        <div class="flex-vst gap-one">
          <p class="txt-dark">{{ $t('settings.firmware.dscr') }}</p>

          <!-- Current Firmware -->
          <div class="border-box flex-vst gap-one">
            <div class="flex-csp gap-one">
              <span class="txt-dark">{{ $t('settings.firmware.installedVersion') }}</span>
              <span class="version c-align">{{ settingsStore.settings.version }}</span>
            </div>

            <div class="flex-csp gap-one small">
              <span class="txt-dark" style="align-self: start">MD5-Hash:</span>
              <div class="firmware-name gap-half">
                <span class="small txt-dark wrd-break" :title="'MD5-Hash: ' + settingsStore.settings.md5">
                  #{{ settingsStore.settings.md5 }}
                </span>
              </div>
            </div>
          </div>

          <!-- Available Firmwares -->
          <TransitionSlideIn
            @after-leave="contentWrap.applyHeight()"
            @enter="contentWrap.applyHeight()"
            @after-enter="contentWrap.applyHeight()"
            :toggle="!!firmwares.length"
            ref="toggleCard"
          >
            <div class="firmwares-box">
              <div class="flex-vst gap-one border-box">
                <!-- Header - More info -->
                <div class="flex-csp gap-one">
                  <p class="">{{ $t('settings.firmware.availableFirmware') }}</p>
                  <a
                    class="flex-cc gap-half link-with-icon txt-dark"
                    href="https://www.wraase.de/rotorcontrol/#firmware"
                    :title="$t('settings.firmware.moreInfoDscr')"
                    target="_blank"
                  >
                    <span>{{ $t('settings.firmware.moreInfo') }}</span>
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
                        v-if="
                          compareVersionWithCurrent(fw.version, settingsStore.settings.version) ===
                          0
                        "
                        >{{ $t('settings.firmware.installed') }}&nbsp;
                        <Icon icon="fa fa-solid fa-check"></Icon>
                      </span>

                      <span
                        class="version c-align"
                        v-if="
                          compareVersionWithCurrent(fw.version, settingsStore.settings.version) ===
                          -1
                        "
                        >{{ $t('settings.firmware.old') }}
                      </span>

                      <span
                        class="version c-align"
                        v-if="
                          compareVersionWithCurrent(fw.version, settingsStore.settings.version) ===
                          1
                        "
                        >{{ $t('settings.firmware.new') }}&nbsp;
                        <Icon icon="fa fa-solid fa-exclamation"></Icon>
                      </span>
                    </div>
                  </div>

                  <hr />

                  <!-- File -->
                  <div class="flex-csp gap-one">
                    <span class="txt-dark" style="align-self: start">{{ $t('commons.file') }}:</span>
                    <div class="firmware-name gap-half">
                      <a
                        class="no-wrap-ellip link-with-icon r-align"
                        :title="$t('settings.firmware.downloadDscr')"
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
            :toggle="hasError"
          >
            <ShakeOnToggle ref="errorBox">
              <div class="border-box flex-hc gap-one">
                <Icon icon="fa-solid fa-circle-exclamation" class="large" />
                <span>{{ getErrorTxt() }}</span>
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

import { useI18n } from "vue-i18n";
const { t } = useI18n();

const settingsStore = useSettingsStore();

const searchBtn = ref(null);
const contentWrap = ref(null);
const errorBox = ref(null);

const isSearching = ref(false);
const firmwares = shallowRef([]);
const hasError = ref(false);

// Compare semver with current vesion if available.
// Return -2 if current version not available.
function compareVersionWithCurrent(version) {
  if (settingsStore.settings.version === '--') {
    return -2;
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
    const response = await fetch('https://www.wraase.de/rotorcontrol/firmwares.json', {
      headers: {
        "Cache-Control": "no-store"
      }
    });
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
    hasError.value = false;
    firmwares.value = responseJson.firmwares;
  } catch (error) {
    firmwares.value = [];

    // Manually shake error box if it is already shown
    if (hasError.value) {
      errorBox.value.shake();
    }

    // Set error
    hasError.value = true;
    console.error(error);
  } finally {
    isSearching.value = false;
  }
}

// Get translated error message
function getErrorTxt() {
  if (hasError.value) {
    return t('settings.firmware.error');
  } else {
    return '';
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
