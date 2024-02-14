<template>
  <CardCollapsable id="favorites" :title="$t('favorites.title')">
    <template #icon>
      <Icon icon="fa-solid fa-bookmark"></Icon>
    </template>

    <CardToggleContentTransition :toggle="isFormOpen">
      <!-- CHILD A -->
      <!-- Description and add favorite button -->
      <template #childA>
        <div class="flex-cc gap" ref="favDscr">
          <p>
            {{ $t('favorites.dscr') }}
          </p>
          <button
            class="btn-std-resp"
            :title="$t('favorites.addBtnDscr')"
            @click="toggleForm"
            :disabled="settingsStore.hasMaxFavorites"
          >
            <Icon icon="fa-solid fa-plus" />
          </button>
        </div>
      </template>

      <!-- CHILD B -->
      <!-- New favorite form -->
      <template #childB>
        <div id="fav-form" ref="favForm" :class="{ 'form-shake': formFailed }">
          <!-- Input Labels -->
          <label for="fav-name" class="small txt-dark">{{ $t('favorites.nameLabel') }}</label>
          <label for="fav-angle" class="small txt-dark"
            >{{ $t('commons.azimut') }} (0° - 450°)</label
          >

          <!-- Name -->
          <input
            id="fav-name-input"
            class="input-std-resp"
            type="text"
            name="fav-name"
            :placeholder="$t('favorites.namePlaceholder')"
            ref="nameInput"
            v-model="newName"
            @keypress="restrictNameInput($event)"
            @paste="restrictNamePaste($event)"
            @keyup.enter="angleInput.focus()"
          />
          <!-- Angle-->
          <input
            id="fav-angle-input"
            class="input-std-resp"
            :class="{ 'red-outline': isAngleWrong }"
            type="Number"
            name="fav-angle"
            min="0"
            :max="maxAngle"
            pattern="\d*"
            :placeholder="$t('commons.angle')"
            ref="angleInput"
            v-model="newAngle"
            @keyup.enter="addFavorite"
            @keypress="restrictAngleInput($event)"
            @paste.prevent=""
          />

          <!-- Buttons -->
          <!-- Cancel Button -->
          <button
            class="btn-std-resp bold no-wrap-ellip"
            :title="$t('commons.cancel')"
            @click="toggleForm"
          >
            <Icon icon="fa-solid fa-xmark" style="vertical-align:; font-size: 1em" />&nbsp;{{ $t('commons.cancel') }}
          </button>
          <!-- Add Button -->
          <button
            class="btn-std-resp bold no-wrap-ellip"
            :title="$t('favorites.createBtnDscr')"
            @click="addFavorite"
            :disabled="settingsStore.hasMaxFavorites"
          >
            <Icon icon="fa-solid fa-check"></Icon>&nbsp;{{ $t('favorites.createBtn') }}
          </button>
        </div>
      </template>
    </CardToggleContentTransition>

    <!-- Favorites List Head-->
    <ul v-if="settingsStore.favorites.array.length" id="favorites-list">
      <li class="small txt-dark">
        <span
          class="fav-index fav-head-sort-btn"
          @click="settingsStore.sortFavoritesBy.id"
          @keyup.enter="settingsStore.sortFavoritesBy.id"
          :title="$t('favorites.sortByID')"
          tabindex="0"
          >#</span
        >
        <span
          class="fav-name fav-head-sort-btn"
          @click="settingsStore.sortFavoritesBy.name"
          @keyup.enter="settingsStore.sortFavoritesBy.name"
          :title="$t('favorites.sortByName')"

          tabindex="0"
          >Name</span
        >
        <span
          class="fav-angle fav-head-sort-btn"
          @click="settingsStore.sortFavoritesBy.angle"
          @keyup.enter="settingsStore.sortFavoritesBy.angle"
          :title="$t('favorites.sortByAngle')"

          tabindex="0"
        >
          {{ $t('commons.azimut') }}
        </span>
        <span class="fav-rot">{{ $t('favorites.moveTo') }}</span>
        <span class="fav-del">{{ $t('favorites.delete') }}</span>
        <hr />
      </li>

      <!-- Favorites List Items-->
      <li v-for="(fav, index) in settingsStore.favorites.array" :key="fav" class="flex-csp">
        <span class="fav-index fav-head small">{{ fav.id }}</span>
        <span class="fav-name no-wrap-ellip">{{ fav.name }}</span>
        <span class="fav-angle">{{ fav.angle }}°</span>
        <button
          class="fav-rot btn-std-resp flex-cc"
          :title="$t('favorites.moveToBtn')"
          :disabled="settingsStore.isLockedByElse"
          @click="requestAngle(fav.angle)"
        >
          <Icon icon="fa-solid fa-play" />
        </button>
        <button
          class="fav-del btn-std-resp flex-cc"
          @click="settingsStore.remFavorite(index)"
          :title="$t('favorites.deleteBtn')"
        >
          <Icon icon="fa-solid fa-xmark" />
        </button>
        <hr v-if="index !== settingsStore.favorites.array.length - 1" />
      </li>
    </ul>
  </CardCollapsable>
</template>

<script setup>
import CardCollapsable from '@/components/CardCollapsable.vue';
import CardToggleContentTransition from '@/components/CardToggleContentTransition.vue';

import { ref, computed } from 'vue';

import { useSettingsStore } from '@/stores/settings';
import { useUmbrellaStore } from '../stores/umbrella';

const settingsStore = useSettingsStore();
const umbrellaStore = useUmbrellaStore();

// Toggle between dscr and 'add new favorite form'
// -----------------------------------------------
const isFormOpen = ref(false);
const nameInput = ref(null);
const angleInput = ref(null);

function toggleForm() {
  if (isFormOpen.value) {
    // When form leaves -> clear inputs, loose focus
    newName.value = '';
    newAngle.value = '';
    nameInput.value.blur();
    isFormOpen.value = false;
  } else {
    isFormOpen.value = true;
    // When form enters -> get focus AFTER form entered, otherwise the layout shifts on Edge.
    // Focus does not work on iOS Safari :(
    window.setTimeout(() => nameInput.value.focus(), 200);
  }
}

// Favorite parameters
// -------------------
const maxNameLength = 30;
const maxAngle = 450;

// V-Models
const newName = ref('');
const newAngle = ref('');

// Used to add outline to input if value is not allowed
const isAngleWrong = computed(() => {
  if (newAngle.value > 450 || newAngle.value < 0) {
    return true;
  } else {
    return false;
  }
});

// Prevent input of wrong angles
// -> restrict to full numbers
// -> prevent leading zeros
// -> prevent input if max angle already reached
function restrictAngleInput(event) {
  if (!/\d/.test(event.key)) {
    //event.preventDefault();
  } else if (newAngle.value > maxAngle) {
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
// ----------
const formFailed = ref(false);

function shakeForm() {
  formFailed.value = true;
  setTimeout(() => {
    formFailed.value = false;
  }, 300);
}

// Add favorite
// ------------
function addFavorite() {
  // Inputs should not be empty and angle should be a number
  if (!newName.value.trim() || !newAngle.value || Number.isNaN(newAngle.value)) {
    shakeForm();
    return false;
  }

  // Angle should be between 0° and 450° and not have decimals
  const newAngleValue = Math.round(Number(newAngle.value));
  if (newAngleValue < 0.0 || newAngleValue > maxAngle) {
    shakeForm();
    return false;
  }

  // Name gets trimmed and cut to maxNameLength
  let newNameValue = newName.value.trim();
  if (newNameValue.length > maxNameLength) {
    newNameValue = newNameValue.substring(0, maxNameLength);
  }

  // Add favorite
  if (settingsStore.addFavorite(newNameValue, newAngleValue)) {
    toggleForm();
    return true;
  } else {
    shakeForm();
    return false;
  }
}

// Move to favorite
// ----------------
function requestAngle(angle) {
  if (!settingsStore.isLockedByElse) {
    if (angle > 360) {
      angle -= 360;
    }
    umbrellaStore.sendTarget(angle);
  }
}
</script>

<style lang="scss" scoped>
#favorites {
  min-width: 100%;
}
.card-dscr {
  margin-bottom: 1em;
}

/* Add-Favorite Dialog */
#fav-form {
  display: grid;
  grid-template-rows: auto auto auto;
  grid-template-columns: 1fr 1fr;
  justify-items: stretch;
  align-items: stretch;
  column-gap: 1em;
  width: 100%;

  input {
    margin-top: 0.3em;
    margin-bottom: 1em;
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
  }

  hr {
    grid-column: 1 / span 5;
  }

  .fav-head-sort-btn {
    cursor: pointer;
    justify-self: start;

    &:hover,
    &:focus-visible {
      color: var(--text-color);
    }
  }

  .fav-index {
    margin-top: 0.2em;
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
</style>
