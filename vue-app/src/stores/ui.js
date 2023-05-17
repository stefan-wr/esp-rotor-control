import { ref, computed, reactive } from 'vue';
import { defineStore } from 'pinia';
import { useStorage } from '@vueuse/core';
import useColors from '@/stores/colors';

export const useUIStore = defineStore('ui', () => {
    const fontSizeRange = [12, 16];

    const colorThemes = useColors();

    // **********************
    //    State, Reactives
    // **********************

    const ui = reactive({
        // useStorgae saves these properties in localStorage
        kbscEnabled: useStorage('kbsc-enabled', true),
        fontSize: useStorage('font-size', 14),
        activeColorTheme: useStorage('active-color-theme', 'default'),

        cards: useStorage('cards', {}),

        hasDegreeLabels: useStorage('has-degree-labels', true),
        hasCardinalLabels: useStorage('has-cardinal-labels', true),
        hasFavoritesDots: useStorage('has-favorites-dots', true),
        requestAngle: 0,
        targetAngle: 0,
        useOverlap: useStorage('use-overlap', true),
        isMouseInCompass: false
    });

    // *************
    //    Getters
    // *************

    // Font size is maximum
    const isFontSizeMax = computed(() => {
        return ui.fontSize >= fontSizeRange[1];
    });

    // Font size is minimum
    const isFontSizeMin = computed(() => {
        return ui.fontSize <= fontSizeRange[0];
    });

    // Is given color the currently active color?
    function isActiveColorTheme(theme) {
        return theme === ui.activeColorTheme;
    }

    // *************
    //    Actions
    // *************

    // Make font bigger
    function fontSizePlus() {
        if (!this.isFontSizeMax) {
            ui.fontSize++;
            applyFontSize();
        }
    }

    // Make font smaller
    function fontSizeMinus() {
        if (!this.isFontSizeMin) {
            ui.fontSize--;
            applyFontSize();
        }
    }

    // Apply font size
    function applyFontSize() {
        document.documentElement.style.setProperty('--font-size', ui.fontSize + 'px');
    }

    // Set color theme
    function setColorTheme(theme) {
        ui.activeColorTheme = theme;
        applyColorTheme();
    }

    // Apply color theme
    function applyColorTheme() {
        for (const [variable, color] of Object.entries(colorThemes[ui.activeColorTheme].vars)) {
            document.documentElement.style.setProperty('--' + variable, color);
        }
    }

    // **********
    return {
        ui,
        fontSizePlus,
        fontSizeMinus,
        applyFontSize,
        isFontSizeMax,
        isFontSizeMin,
        colorThemes,
        setColorTheme,
        applyColorTheme,
        isActiveColorTheme
    };
});
