import { computed, reactive, ref } from 'vue';
import { defineStore } from 'pinia';
import { useStorage } from '@vueuse/core';

import { useUmbrellaStore } from './umbrella.js';

export const useSettingsStore = defineStore('settings', () => {
    const umbrellaStore = useUmbrellaStore();

    // Max allowed number of favorites
    const maxFavorites = 10;

    // **********************
    //    State, Reactives
    // **********************

    // Calibration parameters
    const calibration = reactive({
        u1: 0.0,
        u2: 0.0,
        a1: 0.0,
        a2: 0.0,
        offset: 0.0
    });

    // Favorites
    const favorites = reactive({
        array: [],
        sortedBy: 'id'
    });

    // Other settings
    const settings = reactive({
        ssid: '--',
        rssi: '--'
    });

    // Rotor lock
    const lock = reactive({
        isLocked: false,
        by: '',
        name: useStorage('lock-name', '')
    });

    // *************
    //    Getters
    // *************

    // Max number of favorites reached?
    const hasMaxFavorites = computed(() => {
        return favorites.array.length >= maxFavorites;
    });

    // Is rotor locked by someone else?
    const isLockedByElse = computed(() => {
        if (lock.isLocked && lock.name !== lock.by) {
            return true;
        } else {
            return false;
        }
    });

    // Get calibration JSON message
    function getCalibrationMsg(a1_value, u1_value, a2_value, u2_value) {
        const cal = {
            a1: a1_value,
            a2: a2_value,
            u1: u1_value,
            u2: u2_value
        };
        return JSON.stringify(cal);
    }

    // Get favorites JSON message, use unreffed copy sorted by ID
    const getFavoritesMsg = computed(() => {
        let copy = JSON.parse(JSON.stringify(favorites.array));
        copy.sort((a, b) => {
            return a.id - b.id;
        });
        return JSON.stringify(copy);
    });

    // Get lock JSON message
    const getLockMsg = computed(() => {
        return JSON.stringify(lock, ['isLocked', 'by']);
    });

    // *************
    //    Actions
    // *************

    // Favorites
    // ---------

    // Test wether a given array is a valid array of favorites
    function isValidFavoritesArray(array) {
        if (Array.isArray(array)) {
            for (let item of array) {
                for (let prop of ['id', 'name', 'angle']) {
                    if (!Object.hasOwn(item, prop)) {
                        return false;
                    }
                }
            }
        } else {
            return false;
        }
        return true;
    }

    // Add a new favorite if max number of favorites has not been reached
    function addFavorite(newName, newAngle) {
        if (this.hasMaxFavorites) {
            return false;
        }
        const newFavorite = {
            id: favorites.array.length + 1,
            name: newName,
            angle: newAngle
        };
        favorites.array.push(newFavorite);
        reapplyFavoriteSorting();
        umbrellaStore.sendFavorites();
        return true;
    }

    // Remove a favorite
    function remFavorite(index) {
        favorites.array.splice(index, 1); // 1. Remove Favorite
        reorderFavoritesIds(); // 2. Reorder IDs
        reapplyFavoriteSorting(); // 3. Reapply previous sorting
        umbrellaStore.sendFavorites(); // 4. Send changes to controller
    }

    // Reset IDs to fill the gap that a removed favorite left behind
    // Works on copy of favorites first and replaces original array with result
    function reorderFavoritesIds() {
        let copy = JSON.parse(JSON.stringify(favorites.array));
        copy.sort((a, b) => {
            return a.id - b.id;
        });
        for (let i = 0; i < copy.length; i++) {
            copy[i].id = i + 1;
        }
        favorites.array = copy;
    }

    // Collection of favorites sorting functions
    const sortFavoritesBy = {
        id: function (save = true) {
            favorites.array.sort((a, b) => {
                return a.id - b.id;
            });
            favorites.sortedBy = save ? 'id' : favorites.sortedBy;
        },

        name: function (save = true) {
            favorites.array.sort((a, b) => {
                const nameA = a.name.toUpperCase();
                const nameB = b.name.toUpperCase();
                if (nameA < nameB) {
                    return -1;
                }
                if (nameA > nameB) {
                    return 1;
                }
                return 0;
            });
            favorites.sortedBy = save ? 'name' : favorites.sortedBy;
        },

        angle: function (save = true) {
            favorites.array.sort((a, b) => {
                return a.angle - b.angle;
            });
            favorites.sortedBy = save ? 'angle' : favorites.sortedBy;
        }
    };

    // Reapply previous favorites sorting
    function reapplyFavoriteSorting() {
        sortFavoritesBy[favorites.sortedBy](false);
    }

    // Lock
    // ----

    // Close lock
    function closeLock() {
        lock.by = lock.name;
        lock.isLocked = true;
        umbrellaStore.sendLock();
    }

    // Open lock
    function openLock() {
        lock.isLocked = false;
        lock.by = '';
        umbrellaStore.sendLock();
    }

    // Reset lock
    function resetLock() {
        lock.by = '';
        lock.isLocked = false;
        lock.name = '';
        umbrellaStore.sendLock();
    }

    // *************
    return {
        calibration,
        favorites,
        settings,
        lock,
        hasMaxFavorites,
        isLockedByElse,
        getCalibrationMsg,
        getFavoritesMsg,
        getLockMsg,
        isValidFavoritesArray,
        addFavorite,
        remFavorite,
        sortFavoritesBy,
        reapplyFavoriteSorting,
        closeLock,
        openLock,
        resetLock
    };
});
