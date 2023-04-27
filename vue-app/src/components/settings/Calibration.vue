<template>
  <SettingCard title="Rotor-Kalibrierung" class="l-align">
    <template #icon>
      <Icon icon="fa-solid fa-wrench" style="margin-right: 0.5em"></Icon>
    </template>

    <template #content>
      <p class="txt-dark">
        Damit der Rotor-Controller die richtige Position anzeigt, muss er kalibiert werden. Wähle
        entweder die geführte Kalibrierung, oder gebe die Parameter manuell ein.
      </p>

      <!-- Tabs Navigation -->
      <div id="calibration">
        <nav class="flex-cl">
          <button
            class="cal-tab normal"
            :class="{ 'cal-tab-active': !isManualTabActive }"
            @click="setTabActive(0)"
          >
            <span class="txt-dark">Geführt</span>
          </button>
          <button
            class="cal-tab normal"
            :class="{ 'cal-tab-active': isManualTabActive }"
            @click="setTabActive(1)"
          >
            <span class="txt-dark">Manuell</span>
          </button>
        </nav>

        <!-- Tabs Content-->
        <div class="border-box cal-page">
          <CardToggleContentTransition :toggle="isManualTabActive" style="width: 100%">
            <!-- Automatic Calibration-->
            <!-- |||||||||||||||||||||-->
            <template #childA>
              <div class="flex-vst gap-one" v-show="!isManualTabActive">
                <ol class="steps flex-vl gap" type="1">
                  <li>
                    Drehe den Rotor über das Original-Steuergerät auf eine Position zwischen 20° und
                    40°.
                  </li>
                  <li>
                    Trage den genauen Winkel, möglichst exakt am Steuergerät abgelesen, bei Position
                    1 ein und drücke den Haken darunter.
                  </li>
                  <li>Drehe den Rotor nun auf eine zweite Position zwischen 430° und 450°.</li>
                  <li>
                    Trage den genauen Winkel diesmal bei Position 2 ein und drücke auf den Haken
                    darunter.
                  </li>
                  <li>Danach drücke auf "Bestätigen".</li>
                </ol>

                <hr />

                <!-- Position 1 -->
                <div class="auto-cal-inputs flex-cst gap-one">
                  <div
                    class="flex-grow border-box flex-vst gap-one auto-cal-pos"
                    :class="{ 'form-shake': pos1FormFailed }"
                  >
                    <label for="azimuth-1" class="flex gap-half">
                      <span>Position&nbsp;1</span>
                      <span class="small no-wrap txt-dark">(20° - 40°)</span>
                    </label>

                    <input
                      class="input-std-resp"
                      type="number"
                      name="azimuth-1"
                      ref="autoCalPos1Input"
                      min="20"
                      max="40"
                      placeholder="Pos. 1"
                      v-model="newPos1Angle"
                      @keyup.enter="confirmPos1(20, 40)"
                      @keypress="restrictAngleInput($event, 20, 40)"
                      @paste.prevent=""
                    />

                    <button
                      class="btn-std-resp bold"
                      title="Position 1 bestätigen."
                      @click="confirmPos1(20, 40)"
                    >
                      <Icon icon="fa-solid fa-check"></Icon>
                    </button>
                  </div>

                  <!-- Position 2 -->
                  <div
                    class="flex-grow border-box flex-vst gap-one auto-cal-pos"
                    :class="{ 'form-shake': pos2FormFailed }"
                  >
                    <label for="azimuth-2" class="flex gap-half">
                      <span>Position&nbsp;2</span>
                      <span class="small no-wrap txt-dark">(430° - 450°)</span>
                    </label>

                    <input
                      class="input-std-resp"
                      type="number"
                      name="azimuth-2"
                      ref="autoCalPos2Input"
                      min="430"
                      max="450"
                      placeholder="Pos. 2"
                      v-model="newPos2Angle"
                      @keyup.enter="confirmPos2(430, 450)"
                      @keypress="restrictAngleInput($event, 430, 450)"
                      @paste.prevent=""
                    />

                    <button
                      class="btn-std-resp bold"
                      title="Position 2 bestätigen."
                      @click="confirmPos2(430, 450)"
                    >
                      <Icon icon="fa-solid fa-check"></Icon>
                    </button>
                  </div>
                </div>

                <!-- Arrows -->
                <div class="auto-cal-arrows flex-cst gap-one medium">
                  <div class="flex-grow flex-cc">
                    <Icon icon="fa-solid fa-arrow-down"></Icon>
                  </div>
                  <div class="flex-grow flex-cc">
                    <Icon icon="fa-solid fa-arrow-down"></Icon>
                  </div>
                </div>

                <!-- Confirm -->
                <div class="auto-cal-confirm flex-vst gap-one">
                  <div class="flex-cst gap-one">
                    <div class="flex-grow flex-vc border-box auto-cal-confirm-lbl">
                      <span class="small txt-dark">Position 1</span>
                      <span>{{ dashedInitial(pos1Angle) }}°</span>
                      <span>{{ dashedInitial(pos1Adc) }} V</span>
                    </div>
                    <div class="flex-grow flex-vc border-box auto-cal-confirm-lbl">
                      <span class="small txt-dark">Position 2</span>
                      <span>{{ dashedInitial(pos2Angle) }}°</span>
                      <span>{{ dashedInitial(pos2Adc) }} V</span>
                    </div>
                  </div>
                  <button
                    class="btn-std-resp bold no-wrap-ellip"
                    ref="confirmBtn"
                    :disabled="!isConfirmEnabled"
                  >
                    <Icon icon="fa-solid fa-check"></Icon>&nbsp;Bestätigen
                  </button>
                </div>
              </div>
            </template>

            <!-- Manual Calibration -->
            <!-- |||||||||||||||||| -->
            <template #childB>
              <div class="flex-vst gap-one" v-show="isManualTabActive">
                <p>
                  Hier können die Kalibrations-Parameter manuell eingegeben werden. Dabei muss
                  gelten: <span class="no-wrap">Pos. 1 &lt; Pos. 2</span>.
                </p>

                <div class="man-cal-inputs border-box">
                  <span></span>
                  <label for="azimuth-1" class="small txt-dark">Azimuth in °</label>
                  <label for="adc-1" class="small txt-dark">Spannung in V</label>

                  <!-- Position 1 -->
                  <span>Pos.&nbsp;1</span>

                  <input
                    class="input-std-resp"
                    :class="{ 'form-shake': pos1FormFailed }"
                    type="number"
                    name="azimuth-1"
                    ref="manCalPos1AngleInput"
                    placeholder="Azimuth"
                    v-model="newPos1Angle"
                  />

                  <input
                    class="input-std-resp"
                    :class="{ 'form-shake': pos1FormFailed }"
                    type="number"
                    step="0.001"
                    name="adc-1"
                    ref="manCalPos1AdcInput"
                    placeholder="Spannung"
                    v-model="newPos1Adc"
                  />

                  <!-- Position 2 -->
                  <span></span>
                  <label for="azimuth-2" class="small txt-dark">Azimuth in °</label>
                  <label for="adc-2" class="small txt-dark">Spannung in V</label>

                  <span>Pos.&nbsp;2</span>
                  <input
                    class="input-std-resp"
                    :class="{ 'form-shake': pos1FormFailed }"
                    type="number"
                    name="azimuth-2"
                    ref="manCalPos2AngleInput"
                    placeholder="Azimuth"
                    v-model="newPos2Angle"
                  />

                  <input
                    class="input-std-resp"
                    :class="{ 'form-shake': pos1FormFailed }"
                    type="number"
                    step="0.001"
                    name="adc-2"
                    ref="manCalPos2AdcInput"
                    placeholder="Spannung"
                    v-model="newPos2Adc"
                  />
                </div>

                <button
                  class="btn-std-resp bold no-wrap-ellip"
                  ref="confirmBtn"
                  :disabled="!isConfirmEnabled"
                >
                  <Icon icon="fa-solid fa-check"></Icon>&nbsp;Bestätigen
                </button>
              </div>
            </template>
          </CardToggleContentTransition>
        </div>
      </div>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import CardToggleContentTransition from '@/components/CardToggleContentTransition.vue';

import { ref, reactive, computed, onMounted } from 'vue';

import { useUmbrellaStore } from '@/stores/umbrella';
import { useRotorStore } from '@/stores/rotor';
import { useSettingsStore } from '@/stores/settings';
import { useUIStore } from '@/stores/ui';

const umbrellaStore = useUmbrellaStore();
const rotorStore = useRotorStore();
const settingsStore = useSettingsStore();
const uiStore = useUIStore();

// Toggle between manual- and auto calibration tabs
// ------------------------------------------------
const isManualTabActive = ref(false);

function setTabActive(tab) {
  isManualTabActive.value = tab ? true : false;
  console.log(isManualTabActive.value);
  resetInputs();
}

// Calibration parameters
// ----------------------
const newPos1Angle = ref(null);
const newPos2Angle = ref(null);

const pos1Angle = ref(0);
const pos2Angle = ref(0);
const pos1Adc = ref(0);
const pos2Adc = ref(0);

// Change initial values to dashes for labels
function dashedInitial(value) {
  if (value) {
    return value;
  } else {
    return '--';
  }
}

// Enable / Disable confirm button
const isConfirmEnabled = computed(() => {
  if (
    pos1Angle.value >= 20 &&
    pos1Angle.value <= 40 &&
    pos2Angle.value >= 420 &&
    pos2Angle.value <= 450
  ) {
    return true;
  } else {
    return false;
  }
});

// Element refs
// ------------
const autoCalPos1Input = ref(null);
const autoCalPos2Input = ref(null);
const confirmBtn = ref(null);

function restrictAngleInput(event, min, max) {
  let newAngle;
  if (event.target.name === 'azimuth-1') {
    newAngle = newPos1Angle.value;
  } else if (event.target.name === 'azimuth-2') {
    newAngle = newPos2Angle.value;
  }

  if (!/\d/.test(event.key)) {
    event.preventDefault();
  } else if (event.key === '0' && !newAngle) {
    event.preventDefault();
  } else if (newAngle > max) {
    event.preventDefault();
  }
}

function confirmPos1(min, max) {
  if (newPos1Angle.value >= min && newPos1Angle.value <= max) {
    pos1Angle.value = Number(newPos1Angle.value);
    pos1Adc.value = rotorStore.rotor.adc_v;
    autoCalPos2Input.value.focus();
  } else {
    shakePos1Form();
  }
  newPos1Angle.value = null;
}

function confirmPos2(min, max) {
  if (newPos2Angle.value >= min && newPos2Angle.value <= max) {
    pos2Angle.value = Number(newPos2Angle.value);
    pos2Adc.value = rotorStore.rotor.adc_v;
    autoCalPos2Input.value.blur();
  } else {
    shakePos2Form();
  }
  newPos2Angle.value = null;
}

function resetInputs() {
  newPos1Angle.value = null;
  newPos2Angle.value = null;
  pos1Angle.value = null;
  pos2Angle.value = null;
  pos1Adc.value = null;
  pos2Adc.value = null;
}

// Shake Forms
// -----------
const pos1FormFailed = ref(false);
const pos2FormFailed = ref(false);

function shakePos1Form() {
  pos1FormFailed.value = true;
  setTimeout(() => {
    pos1FormFailed.value = false;
  }, 300);
}

function shakePos2Form() {
  pos2FormFailed.value = true;
  setTimeout(() => {
    pos2FormFailed.value = false;
  }, 300);
}
</script>

<style lang="scss" scoped>
#calibration {
  width: 100%;
  align-self: center;

  nav {
    gap: 0.3em;
  }
}

/* Tabs */
.cal-tab {
  background-color: var(--content-color-1);
  color: var(--text-color);
  border: none;
  border-radius: calc($border-box-bradius / 2) calc($border-box-bradius / 2) 0 0;
  transition: background-color 0.1s, opacity 0.1s;
  min-height: $button-height;
  padding: 0.5em 1em;

  span {
    display: inline-block;
    transform: scale(85%);
    transition: transform 0.1s, opacity 0.1s;
  }

  &:hover {
    filter: none;
    transform: scale(100%);
    background-color: var(--content-color-3);

    span {
      opacity: 1;
      transform: scale(92.5%);
    }
  }
}

.cal-tab-active {
  background-color: var(--content-color-3);

  span {
    opacity: 1;
    transform: scale(100%);
  }

  &:hover {
    span {
      transform: scale(100%);
    }
  }
}

.cal-page {
  width: 100%;
  border-top-left-radius: 0;
}

/* Steps */
.steps {
  //margin-bottom: 1em;
  list-style-type: none;
  position: relative;
  counter-reset: item;

  $list-style-dim: 1.6em;
  $vl-width: 2px;

  &:before {
    content: '';
    width: $vl-width;
    height: calc(100% - $list-style-dim);
    border-radius: 100px;
    position: absolute;
    left: calc($list-style-dim / 2 - $vl-width / 2);
    top: 50%;
    transform: translateY(-50%);
    background-color: var(--content-color-2);
  }

  li {
    counter-increment: item;
    position: relative;

    &:before {
      content: counter(item);
      display: flex;
      align-items: center;
      justify-content: center;
      width: $list-style-dim;
      height: $list-style-dim;
      border-radius: 100px;
      position: absolute;
      left: -$ol-left-padding;
      background-color: var(--content-color-2);
      color: var(--text-color-darker);
    }
  }
}

/* Calibration */
/* ----------- */
.auto-cal-inputs {
  label {
    flex-wrap: wrap;
    align-items: baseline;
    justify-content: center;
    text-align: center;

    /* Makes sure labels wraps evenly on both positions */
    span:first-child {
      flex-basis: 5em;
    }

    span:nth-child(2) {
      flex-basis: 6.5em;
    }
  }
}

.auto-cal-pos {
  /* Both position boxes have same start width and grow evenly */
  flex-basis: 0;
}

.auto-cal-arrows {
  color: var(--content-color-2);
  margin: -0.5em 0;
}

.auto-cal-confirm-lbl {
  padding: 0.5em !important;
  flex-basis: 0;
}

.man-cal-inputs {
  display: grid;
  grid-template-columns: auto 1fr 1fr;
  grid-template-rows: auto 1fr auto 1fr;
  align-items: center;
  column-gap: 1em;

  label {
    margin-bottom: 0.4em;
  }

  label:nth-child(n + 8) {
    margin-top: 1em;
  }
}
</style>
