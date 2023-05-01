import { computed, reactive } from 'vue';
import { defineStore } from 'pinia';

import { useUmbrellaStore } from './umbrella.js';

export const useSettingsStore = defineStore('settings', () => {
    const umbrellaStore = useUmbrellaStore();

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

    // Array of favorites
    const favorites = reactive([
        {
            id: 1,
            name: 'Norden',
            angle: 0
        },
        {
            id: 2,
            name: 'Osten',
            angle: 90
        },
        {
            id: 3,
            name: 'Süden',
            angle: 180
        },
        {
            id: 4,
            name: 'Westen',
            angle: 270
        }
    ]);

    const maxFavorites = 10;
    const hasMaxFavorites = computed(() => {
        return favorites.length >= maxFavorites;
    });

    // *************
    //    Getters
    // *************

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

    // Get favorites JSON message
    const getFavoritesMsg = computed(() => {
        return JSON.stringify(favorites);
    });

    // *************
    //    Actions
    // *************

    // Add a new favorite if number of favorites < maxFavorites
    function addFavorite(newName, newAngle) {
        if (hasMaxFavorites.value) {
            return false;
        }
        const newFavorite = {
            id: favorites.length + 1,
            name: newName,
            angle: newAngle
        };
        favorites.push(newFavorite);
        umbrellaStore.sendFavorites();
        return true;
    }

    // Remove a favorite
    function remFavorite(index) {
        favorites.splice(index, 1);
        reorderFavoritesIds();
        umbrellaStore.sendFavorites();
    }

    // Reset IDs to resemble the current order of the favorites array
    function reorderFavoritesIds() {
        for (let i = 0; i < favorites.length; i++) {
            favorites[i].id = i + 1;
        }
    }

    // Sort favorites array by IDs
    function sortFavoritesById() {
        favorites.sort((a, b) => {
            return a.id - b.id;
        });
    }

    // Sort favorites array by name
    function sortFavoritesByName() {
        favorites.sort((a, b) => {
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
    }

    // Sort favorites array by angle
    function sortFavoritesByAngle() {
        favorites.sort((a, b) => {
            return a.angle - b.angle;
        });
    }

    // *************
    return {
        calibration,
        favorites,
        maxFavorites,
        hasMaxFavorites,
        getCalibrationMsg,
        getFavoritesMsg,
        addFavorite,
        remFavorite,
        sortFavoritesByName,
        sortFavoritesByAngle,
        sortFavoritesById
    };
});
