import { ref, computed, reactive } from 'vue';
import { defineStore } from 'pinia';
import { useRotorStore } from './rotor.js';
import { useUIStore } from './ui.js';

export const useSettingsStore = defineStore('settings', () => {
    // Other stores
    const rotor = useRotorStore();
    const ui = useUIStore();

    // **********************
    //    State, Reactives
    // **********************

    const maxFavorites = 10;

    const settings = reactive({
        offset: 0.0,
        cal: {
            u1: 0.0,
            u2: 0.0,
            a1: 0.0,
            a2: 0.0
        },
        favorites: [
            {   
                id: 1,
                name: 'Osten',
                angle: 90
            },
            {
                id: 2,
                name: 'Superlanger Bezeichner für einen Favoriten',
                angle: 180
            },
            {
                id: 3,
                name: 'Megalangeswortohneuntebrechung',
                angle: 360
            }
        ]
    });

    // *************
    //    Getters
    // *************
    const getSettingsMsg = computed(() => {
        return `SETTINGS|${JSON.stringify(settings)}`;
    });

    // *************
    //    Actions
    // *************

    function addFavorite(newName, newAngle) {
    const newFavorite = {
        id: settings.favorites.length + 1,
        name: newName,
        angle: newAngle
      };
      settings.favorites.push(newFavorite);
    }

    function remFavorite(index) {
        settings.favorites.splice(index, 1);
        reorderFavoritesIds();
    }
    
    function reorderFavoritesIds() {
        for (let i = 0; i < settings.favorites.length; i++) {
            settings.favorites[i].id = i + 1;
        }
    }

    function sortFavoritesByName() {
        settings.favorites.sort((a, b) => {
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

    function sortFavoritesByAngle() {
        settings.favorites.sort((a, b) => {
            return a.angle - b.angle;
          });
    }

    function sortFavoritesById() {
        settings.favorites.sort((a, b) => {
            return a.id - b.id;
          });
    }

    return {
        settings,
        maxFavorites,
        getSettingsMsg,
        addFavorite,
        remFavorite,
        sortFavoritesByName,
        sortFavoritesByAngle,
        sortFavoritesById,
    };
});
