<template>
  <Card id="favorites" title="Favoriten">
    <div class="trans-wrap" ref="transWrap">
      <Transition name="dscr-trans" @enter="applyCurrentWrapHeight">
        <!-- Description and add favorite button -->
        <div id="fav-dscr" class="flex-cc" ref="favDscr" v-show="!isAddingFavorite">
          <p>Lege bis zu 10 Favoriten an, die per Tastendruck automatisch angefahren werden können.</p>
          <button
            @click="toggleAddingFavorite"
            title="Neuen Favoriten anlegen."
            :disabled="settingsStore.settings.favorites.length >= settingsStore.maxFavorites"
          >
            <Icon icon="fa-solid fa-plus" />
          </button>
        </div>
      </Transition>

      <!-- New favorite form -->
      <Transition name="form-trans" @enter="applyCurrentWrapHeight">
        <div
          id="fav-form"
          ref="favForm"
          v-show="isAddingFavorite"
          :class="{ 'form-shake': formFailed }"
        >
          <!-- Input Labels -->
          <label for="fav-name" class="small">Favoriten-Bezeichner</label>
          <label for="fav-angle" class="small">Azimuth (0° - 450°)</label>

          <!-- Name -->
          <input
            id="fav-name-input"
            type="text"
            name="fav-name"
            placeholder="Bezeichner"
            ref="nameInput"
            v-model="newName"
            @keypress="restrictNameInput($event)"
            @paste="restrictNamePaste($event)"
          />
          <!-- Angle-->
          <input
            id="fav-angle-input"
            type="Number"
            name="fav-angle"
            min="0"
            max="360"
            pattern="\d*"
            placeholder="Winkel"
            ref="angleInput"
            v-model="newAngle"
            @keyup.enter="addFavorite"
            @keypress="restrictAngleInput($event)"
            @paste.prevent=""
          />

          <!-- Buttons -->
          <!-- Cancel Button -->
          <button class="bold" title="Abbrechen" @click="toggleAddingFavorite">
            <Icon icon="fa-solid fa-xmark" />&nbsp;Abbrechen
          </button>
          <!-- Add Button -->
          <button class="bold" title="Favoriten anlegen." @click="addFavorite">
            <Icon icon="fa-solid fa-check"></Icon>&nbsp;Anlegen
          </button>
        </div>
      </Transition>
    </div>

    <!-- Favorites List Head-->
    <ul v-if="settingsStore.settings.favorites.length" id="favorites-list">
      <li class="small">
        <span
          class="fav-index fav-head fav-head-sort"
          @click="settingsStore.sortFavoritesById()"
          title="Favoriten nach ID sortieren."
          >#</span
        >
        <span
          class="fav-name fav-head fav-head-sort"
          @click="settingsStore.sortFavoritesByName()"
          title="Favoriten nach Namen sortieren."
          >Name</span
        >
        <span
          class="fav-angle fav-head fav-head-sort"
          @click="settingsStore.sortFavoritesByAngle()"
          title="Favoriten nach Winkel sortieren."
        >
          Azimuth
        </span>
        <span class="fav-rot fav-head">Anfahr.</span>
        <span class="fav-del fav-head">Lösch.</span>
        <hr />
      </li>

      <!-- Favorites List Items-->
      <li v-for="(fav, index) in settingsStore.settings.favorites" class="flex-csp">
        <span class="fav-index fav-head small">{{ fav.id }}</span>
        <span class="fav-name wrd-break">{{ fav.name }}</span>
        <span class="fav-angle">{{ fav.angle }}°</span>
        <button class="fav-rot flex-cc" title="Favoriten anfahren.">
          <span>
            <Icon icon="fa-solid fa-play" />
          </span>
        </button>
        <button
          class="fav-del flex-cc"
          @click="settingsStore.remFavorite(index)"
          title="Favoriten löschen."
        >
          <Icon icon="fa-solid fa-xmark" />
        </button>
        <hr v-if="index !== settingsStore.settings.favorites.length - 1" />
      </li>
    </ul>
  </Card>
</template>

<script setup>
import Card from '@/components/Card.vue';

import { ref, reactive, computed, onMounted } from 'vue';

import { useUmbrellaStore } from '@/stores/umbrella';
import { useRotorStore } from '@/stores/rotor';
import { useSettingsStore } from '@/stores/settings';
import { useUIStore } from '@/stores/ui';

const umbrellaStore = useUmbrellaStore();
const rotorStore = useRotorStore();
const settingsStore = useSettingsStore();
const uiStore = useUIStore();

// Toggle between description and 'add new favorite form'
// ------------------------------------------------------
const isAddingFavorite = ref(false);
const nameInput = ref(null);
const angleInput = ref(null);

function toggleAddingFavorite() {
  if (isAddingFavorite.value) {
    // When form leaves -> clear inputs, loose focus
    newName.value = '';
    newAngle.value = '';
    nameInput.value.value = '';
    angleInput.value.value = '';
    nameInput.value.blur();
    isAddingFavorite.value = false;
  } else {
    isAddingFavorite.value = true;
    // When form enters -> get focus AFTER form entered, otherwise the layout shifts on Edge.
    // Focus does not work on iOS Safari :(
    window.setTimeout(() => nameInput.value.focus(), 200);
  }
}

// Add new favorite
// ----------------
const newName = ref('');
const newAngle = ref('');

const maxNameLength = 30;

// Prevent input of wrong angles
// -> restrict to full numbers between 0-450
// -> prevent leading zeros
function restrictAngleInput(event) {
  if (!/\d/.test(event.key)) {
    event.preventDefault();
  } else if (event.key === '0' && !newAngle.value) {
    event.preventDefault();
  } else if (Number(newAngle.value + event.key) > 450) {
    event.preventDefault();
  }
}

// Restrict typing-in more than 30 characters as a favorite name
function restrictNameInput(event) {
  if (newName.value.length >= maxNameLength) {
    event.preventDefault();
  }
}

// Restrict name length to 30 characters AFTER paste
function restrictNamePaste(event) {
  setTimeout(() => {
    if (newName.value.length > maxNameLength) {
      newName.value = newName.value.substring(0, maxNameLength);
    }
  }, 0);
}

// Shake form
const formFailed = ref(false);
function shakeForm() {
  formFailed.value = true;
  setTimeout(() => {
    formFailed.value = false;
  }, 300);
}

// Add favorite
function addFavorite() {
  // Inputs should not be empty and angle should be a number
  if (!newName.value.trim() || !newAngle.value || Number.isNaN(newAngle.value)) {
    shakeForm();
    return false;
  }
  // Angle should be between 0° and 450°
  const newAngleValue = Number(newAngle.value);
  if (newAngleValue < 0.0 || newAngleValue > 450.0) {
    shakeForm();
    return false;
  }
  // Name gets cut to maxNameLength
  let newNameValue = newName.value.trim();
  if (newNameValue.length > maxNameLength) {
    newNameValue = newNameValue.substring(0, maxNameLength);
  }
  // Add favorite
  settingsStore.addFavorite(newName.value, newAngle.value);
  toggleAddingFavorite();
  return true;
}

// Transition between description and adding new favorites form.
// For a smooth transition, adjust the height of the wrapping
// div to the height of the new child when it starts to fly in.
// -------------------------------------------------------------
const transWrap = ref(null);
const favDscr = ref(null);
const favForm = ref(null);

// Initially, set wrapper height to height of initial child and
// reapply height when th browser window resizes
onMounted(() => {
  applyCurrentWrapHeight();
  window.addEventListener('resize', applyCurrentWrapHeight);
});

// Adjust height of wrapper to height of currently shown child
function applyCurrentWrapHeight() {
  if (isAddingFavorite.value) {
    transWrap.value.style.height = String(favForm.value.offsetHeight) + 'px';
  } else {
    transWrap.value.style.height = String(favDscr.value.offsetHeight) + 'px';
  }
}

</script>

<style lang="scss" scoped>
.card-dscr {
  margin-bottom: 1em;
}

#favorites {
  button {
    height: 2em;
    padding: 0 1em;

    @media (hover: none) {
      font-size: 1.25rem;
    }
  }
}

#fav-dscr {
  gap: 1em;
}

/* Add-Favorite Dialog */
#fav-form {
  display: grid;
  grid-template-rows: auto 1fr auto;
  grid-template-columns: 1fr 1fr;
  justify-items: stretch;
  align-items: stretch;
  column-gap: 1em;
  width: 100%;

  label {
    opacity: 0.6;
  }

  button {
    padding-left: 0;
    padding-right: 0;
    width: 100%;
  }

  input {
    margin-top: 0.3em;
    margin-bottom: 1em;
    @media (hover: none) {
      font-size: 1.25rem;
    }
  }
}

/* Favorites List */
#favorites-list {
  display: grid;
  grid-template-columns: auto 2fr auto auto auto;
  align-items: center;
  row-gap: 0.5em;
  column-gap: 1em;
  margin-top: 1em;

  li {
    display: contents;
    width: 100%;
    gap: 1em;
  }

  hr {
    grid-column: 1 / span 5;
    border: none;
    border-top: 1px solid var(--content-color-1);
  }

  .fav-head {
    opacity: 0.6;
  }

  .fav-head-sort {
    cursor: pointer;
    justify-self: start;

    &:hover {
      opacity: 0.9;
    }
  }

  .fav-index {
    margin-top: 0.2em;
  }

  .fav-name {
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
  }

  .fav-angle {
    justify-self: end;
    margin-right: 0.5em;
  }

  .fav-del,
  .fav-rot {
    justify-self: center;
  }

  button.fav-rot {
    justify-self: end;
  }
}

/* Form shake animation*/
.form-shake {
  animation: shake 0.3s;
}

@keyframes shake {
  0% {
    transform: translateX(0%);
  }
  20% {
    transform: translateX(-8%);
  }
  40% {
    transform: translateX(5%);
  }
  60% {
    transform: translateX(-3%);
  }
  80% {
    transform: translateX(2%);
  }
  100% {
    transform: translateX(0%);
  }
}

/* Vue Transition animation, sliding in
   and out the add favorite form */
$speed: 0.2s;
.trans-wrap {
  transition: height $speed;
}

/* Description */
.dscr-trans-leave-active,
.dscr-trans-enter-active {
  transition: transform $speed, opacity $speed;
  position: absolute;
  top: 0;
  left: 0;
}
.dscr-trans-leave-to,
.dscr-trans-enter-from {
  opacity: 0;
  transform: translateX(-80%) scaleY(70%);
}

/* Add favorite form */
.form-trans-leave-active,
.form-trans-enter-active {
  transition: transform $speed, opacity $speed;
  position: absolute;
  top: 0;
  left: 0;
}
.form-trans-leave-to,
.form-trans-enter-from {
  opacity: 0;
  transform: translateX(80%) scaleY(70%);
}
</style>
