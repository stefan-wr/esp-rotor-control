// **********
// WebSockets
// **********
var ws = {
    sockets2: [
        {
            gateway: `ws://${window.location.host}/ws`,
            socket: null
        }
    ],

    sockets: [
        {
            gateway: `ws://home.wraase.de:1339/ws`,
            socket: null
        }
    ],

    store: new Object,

    // Initialise a WebSocket
    // ----------------------
    initWebSocket: (i) => {
        console.log('[' + this.sockets[i].gateway + '] Open connection...');
        this.sockets[i].socket = new WebSocket(this.sockets[i].gateway);
        // Open
        this.sockets[i].socket.onopen = function (event) {
            console.log('[' + this.sockets[i].gateway + '] Connected.');
        };
        // Close
        this.sockets[i].socket.onclose = function (event) {
            console.log('[' + this.sockets[i].gateway + '] Connection closed. (' + event.reason + ')');
            setTimeout(this.initWebSocket, 1000, i);
        };
        // Error
        this.sockets[i].socket.onerror = function (event) {
            console.log('[' + this.sockets[i].gateway + '] Error.');
            this.sockets[i].socket.close();
        };
        // Receive message
        this.sockets[i].socket.onmessage = this.receiveData;
    },

    // Receive data from WebSocket
    // ---------------------------
    receiveData: (event) => {
        // Split type-identifier from JSON data
        var [identifier, data] = event.data.split("|");

        // ROTOR DATA
        // ..........
        if (identifier === "ROTOR") {
            console.log('[' + event.origin + '] ' + event.data);
            var rotor_data = JSON.parse(data);

            // Go through rotor data
            for (key in rotor_data) {

                // Rotation & Direction
                if (key === "rotation") {
                    this.store.rotor[key] = rotor_data[key];
                }
            }
        }

        // SETTINGS
        // ........
        if (identifier === "SETTINGS") {
            console.log('[' + event.origin + '] ' + event.data);
            settings_data = JSON.parse(data);

            // Go through settings data  
            for (key in settigs_data) {
                if (key === "cal_u1") {
                    this.store.settings.u1 = settings_data[key];
                }
            }
        }
    },

    // Send data over socket[i]
    // ------------------------
    sendData: (i, data) => {
        this.sockets[i].socket.send(data);
    },

    // Init sockets
    initSockets: () => {
        for (let i = 0; i < this.sockets.length; i++) {
            this.initWebSocket(i);
        }
    }
}

export { ws };
