import { ref, computed, reactive } from 'vue';
import { defineStore } from 'pinia';
import useColors from '@/stores/colors';

export const useUIStore = defineStore('ui', () => {
    const fontSizeRange = [12, 16];

    const colorThemes = useColors();

    // **********************
    //    State, Reactives
    // **********************

    const ui = reactive({
        kbscEnabled: true,
        showSettings: false,
        fontSize: 14,
        activeColorTheme: 'default',

        cards: {},

        reqAngle: 0,
        reqAngleLocked: false,
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
            applyFontsize();
        }
    }

    // Make font smaller
    function fontSizeMinus() {
        if (!this.isFontSizeMin) {
            ui.fontSize--;
            applyFontsize();
        }
    }

    // Apply font size
    function applyFontsize() {
        document.documentElement.style.setProperty('--font-size', ui.fontSize + 'px');
    }

    // Apply color theme
    function applyColorTheme(theme) {
        console.log("A")
        colorThemes[theme].isActive = true;
        ui.activeColorTheme = theme;
        for (const [variable, color] of Object.entries(colorThemes[theme].vars)) {
            document.documentElement.style.setProperty('--' + variable, color);
        }
    }

    return {
        ui,
        fontSizePlus,
        fontSizeMinus,
        isFontSizeMax,
        isFontSizeMin,
        colorThemes,
        applyColorTheme,
        isActiveColorTheme
    };
});
