UI = {
    kbsc_enabled: true,
    cardinals: ["N", "NNO", "NO", "ONO",
                "O", "OSO", "SO", "SSO",
                "S", "SSW", "SW", "WSW",
                "W", "WNW", "NW", "NNW" ],

    // Current rotation
    setRotation: function() {
      var element = document.getElementById("direction");
      switch (rotor.rotation) {
        case 0:   // Case 0: not rotating
            if (element.innerHTML != '-') {
                element.innerHTML = '-';
            }
            break;
        case -1:  // Case -1: rotating CCW
            element.innerHTML = "<-N";
            break;
        case 1:   // Case 1: rotating CW
            element.innerHTML = "N->";
            break;
      }
    },

    // Rotor position ADC voltage
    setAdcVoltage: function() {
        var element = document.getElementById("adc_v");
        element.innerHTML = rotor.adc_v.toFixed(3);
    },

    // Rotor angle
    setAngle: function() {
        // Angle
        var element = document.getElementById("cmp_angle");
        element.textContent = (rotor.angle.toFixed(2));
        
        // Compass
        var needle = document.getElementById("cmp_needle");
        needle.style.transform = `rotate(${rotor.angle.toFixed(2)}deg)`;

        // Cardinal direction
        var cmp_label = document.getElementById("cmp_cardinal");
        var ci = Math.round((rotor.angle / (360 / this.cardinals.length)));
        cmp_label.textContent = this.cardinals[ci % this.cardinals.length];
    },

    // Set angle request needle
    setRequestNeedle: function(angle) {
        var needle = document.getElementById("cmp_req_needle");
        needle.style.transform = `rotate(${angle.toFixed(1)}deg)`;
    },

    // Set requested angle to label
    setRequestAngle: function(angle) {
        var label = document.getElementById("cmp_req_angle");
        label.textContent = angle.toFixed(0);
    },

    // Rotor speed
    setSpeed: function() {
        var spd_slider = document.getElementById("spd_slider");
        spd_slider.value = rotor.speed;
    
        var spd_label = document.getElementById("spd_val");
        if (rotor.speed === 0) {
        spd_label.textContent = "1";
        } else {
        spd_label.textContent = rotor.speed;
        }
    },

    // Keyboard shortcuts switch
    setKbsc: function() {
        document.getElementById("kbsc_switch").checked = this.kbsc_enabled;
    },

    setCalU1() {
        document.getElementById("cal_a1_lbl").textContent = (settings.cal.a1).toFixed(0);
        document.getElementById("cal_u1_lbl").textContent = (settings.cal.u1 / 1.5).toFixed(3);
    },

    setCalU2() {
        document.getElementById("cal_a2_lbl").textContent = (settings.cal.a2).toFixed(0);
        document.getElementById("cal_u2_lbl").textContent = (settings.cal.u2 / 1.5).toFixed(3);
    },

    enableCalSubmitBtn() {
        document.getElementById("cal_submit").disabled = false;
    },

    disableCalSubmitBtn() {
        document.getElementById("cal_submit").disabled = true;
    },

    // Update all UI elements by calling all UI setters
    update: function() {
        this.setRotation();
        this.setAdcVoltage();
        this.setAngle();
        this.setSpeed();
        this.setKbsc();
    }
};
