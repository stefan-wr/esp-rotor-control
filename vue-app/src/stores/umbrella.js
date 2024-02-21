import { ref } from 'vue';
import { defineStore } from 'pinia';

import { useRotorStore } from './rotor.js';
import { useSettingsStore } from './settings.js';
import { useUIStore } from './ui.js';

/**
 * Main store handling all sockets sends and receives.
 * Distributes received messages down to all other stores.
 */
export const useUmbrellaStore = defineStore('umbrella', () => {
    // Other stores
    const rotorStore = useRotorStore();
    const settingsStore = useSettingsStore();
    const uiStore = useUIStore();

    // Check Connection
    // ****************
    const connectionTimeout = 5000;
    const hasLostConnection = ref(false);
    let timeOfLastMsg = new Date().getTime();

    function updateTimeOfLastMsg() {
        const date = new Date();
        timeOfLastMsg = date.getTime();
    }

    // Check for loss of connection by checking for incoming messages
    // Close socket if connection lost -> this will trigger initWebSocket
    function checkConnection() {
        const date = new Date();
        const now = date.getTime();
        if (now - timeOfLastMsg >= connectionTimeout) {
            hasLostConnection.value = true;
            console.log('Connection lost. Closing WebSocket.');
            socket.socket.close();
        }
    }

    // Start continuous connection test
    if (import.meta.env.PROD || true) {
        setInterval(checkConnection, connectionTimeout);
    }

    // Socket
    // ******
    const socket = {
        gateway: import.meta.env.DEV
            ? import.meta.env.VITE_SOCKET_GATEWAY
            : `ws://${window.location.host}/ws`,
        socket: null
    };

    if (import.meta.env.DEV) {
        console.log('Window.Location.Host: ', window.location.host);
        console.log('Socket Gateway: ', socket.gateway);
    }

    // Initialise Socket Events
    // ------------------------
    function initWebSocket() {
        console.log('[' + socket.gateway + '] Open connection...');
        updateTimeOfLastMsg();
        socket.socket = new WebSocket(socket.gateway);
        // Open
        socket.socket.onopen = function (event) {
            console.log('[' + socket.gateway + '] Connected.');
        };
        // Close
        socket.socket.onclose = function (event) {
            console.log('[' + socket.gateway + '] Connection closed. (' + event.reason + ')');
            setTimeout(initWebSocket, 1000);
        };
        // Error
        socket.socket.onerror = function (event) {
            console.error('[' + socket.gateway + '] Error.');
            socket.socket.close();
        };
        // Receive message
        socket.socket.onmessage = receiveData;
    }

    // Socket main send function
    // -------------------------
    function sendData(data) {
        socket.socket.send(data);
    }

    // Init socket
    // -----------
    initWebSocket();

    // ***************
    //    Receivers
    // ***************
    const identifiers = {
        rotor: 'ROTOR',
        settings: 'SETTINGS',
        ui: 'UI',
        calibration: 'CALIBRATION',
        favorites: 'FAVORITES',
        lock: 'LOCK'
    };

    // Receivers collection
    const receivers = {
        [identifiers.rotor]: receiveRotorMsg,
        [identifiers.settings]: receiveSettingsMsg,
        [identifiers.ui]: () => {},
        [identifiers.calibration]: receiveCalibrationMsg,
        [identifiers.favorites]: receiveFavoritesMsg,
        [identifiers.lock]: receiveLockMsg
    };

    // Receive websocket message
    function receiveData(event) {
        hasLostConnection.value = false;
        updateTimeOfLastMsg();

        // Split msg-type-identifier from JSON data
        var [identifier, msg] = event.data.split('|');

        // Receive JSON message
        if (receivers.hasOwnProperty(identifier)) {
            receivers[identifier](msg);
            //console.log('[' + event.origin + '] ' + event.data);
        } else {
            console.warn(`ERROR: unknown message identifier '${identifier}'`);
        }
    }

    // Receiver for rotor message
    function receiveRotorMsg(msg) {
        let rotorMsg = JSON.parse(msg);
        for (let key in rotorMsg) {
            if (key in rotorStore.rotor) {
                rotorStore.rotor[key] = rotorMsg[key];
            } else {
                console.warn(`ERROR: key '${key}' not in rotor.`);
            }
        }
    }

    // Receiver for calibration message
    function receiveCalibrationMsg(msg) {
        let calibrationMsg = JSON.parse(msg);
        for (let key in calibrationMsg) {
            if (key in settingsStore.calibration) {
                settingsStore.calibration[key] = calibrationMsg[key];
            } else {
                console.warn(`ERROR: key '${key}' not in calibration.`);
            }
        }
    }

    // Receiver for favorites message
    function receiveFavoritesMsg(msg) {
        const parsedMsg = JSON.parse(msg);
        if (settingsStore.isValidFavoritesArray(parsedMsg)) {
            settingsStore.favorites.array = parsedMsg;
            settingsStore.reapplyFavoriteSorting();
        } else {
            // Received favorites are not valid -> reset them in UI and on controller
            console.error(`ERROR: received favorites are not valid: ${msg}`);
            settingsStore.favorites.array = [];
            sendFavorites('[]');
        }
    }

    // Receiver for settings message
    function receiveSettingsMsg(msg) {
        let settingsMsg = JSON.parse(msg);
        for (let key in settingsMsg) {
            if (key in settingsStore.settings) {
                settingsStore.settings[key] = settingsMsg[key];
            } else {
                console.warn(`ERROR: key '${key}' not in settings.`);
            }
        }
    }

    // Receiver for lock message
    function receiveLockMsg(msg) {
        let lockMsg = JSON.parse(msg);
        for (let key in lockMsg) {
            if (key in settingsStore.lock) {
                settingsStore.lock[key] = lockMsg[key];
            } else {
                console.warn(`ERROR: key '${key}' not in lock.`);
            }
        }
    }

    // *************
    //    Senders
    // *************

    // General message sender
    // ----------------------
    function sendMsg(identifier, msg) {
        sendData(`${identifier}|${msg}`);
    }

    // Specific senders
    // ----------------
    function sendRotation(dir) {
        sendMsg(identifiers.rotor, rotorStore.getRotationMsg(dir));
    }

    function sendSpeed() {
        sendMsg(identifiers.rotor, rotorStore.getSpeedMsg);
    }

    function sendTarget(angle) {
        sendMsg(identifiers.rotor, rotorStore.getTargetMessage(angle, uiStore.ui.useOverlap, uiStore.ui.useSmoothSpeed));
    }

    function sendCalibration(a1, u1, a2, u2) {
        sendMsg(identifiers.calibration, settingsStore.getCalibrationMsg(a1, u1, a2, u2));
    }

    function sendFavorites() {
        sendMsg(identifiers.favorites, settingsStore.getFavoritesMsg);
    }

    function resetFavorites() {
        sendMsg(identifiers.favorites, '[]');
    }

    function sendLock() {
        sendMsg(identifiers.lock, settingsStore.getLockMsg);
    }

    function sendScreen() {
        sendMsg(identifiers.settings, settingsStore.getScreenMsg);
    }

    function sendRotorSettings() {
        sendMsg(identifiers.settings, settingsStore.getRotorSettingsMsg);
    }

    // *************
    return {
        hasLostConnection,
        sendRotation,
        sendSpeed,
        sendTarget,
        sendCalibration,
        sendFavorites,
        resetFavorites,
        sendLock,
        sendScreen,
        sendRotorSettings
    };
});
