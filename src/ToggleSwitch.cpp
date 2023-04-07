#include <ToggleSwitch.h>
#include <SimpleSPIFFS.h>
#include <globals.h>

// Define ToggleSwitch members
// ***************************
ToggleSwitch::ToggleSwitch(int p_pin, String p_id, String p_name, String p_dscr, bool p_defaultP) :
                            pin(p_pin), id(p_id), name(p_name), dscr(p_dscr), defaultP(p_defaultP),
                            on(false), locked(false) {}

// => Get HTML String of switch
String ToggleSwitch::getHTML() {
  String s = "";
  s += "<li>";
  s += "<div class=\"reboot\">";
  s += "<p class=\"l-align\">" + name + "<br>";
  s += "<span class=\"small\">" + dscr + "</span></p>";
  s += "<div class=\"switch small\">";
  s += "<input id=\"" + id + "\" type=\"checkbox\" onchange=\"toggleSwitch(this)\"" + ((on) ? " autocomplete=\"off\" checked" : "") + ">";
  s += "<span class=\"slider\"></span>";
  s += "<span class=\"on\">AN</span><span class=\"off\">AUS</span>";
  s += "</div></div></li>";
  return s;
}

// => get JSON string of switch for sending over websocket
String ToggleSwitch::getJSON() {
  return (String) "\"" + id + "\":\"" + (String)on + "\"";
}

// => Save switch position to file
void ToggleSwitch::savePosition() {
  while (locked) {}
  Serial.println("Saving position [" + (String)on + "] for switch [" + id + "].");
  locked = true;
  writeToSPIFFS(("/" + id + ".txt").c_str(), (String)on);
  locked = false;
}

// => Load switch position from file
void ToggleSwitch::loadPosition() {
  const String status = readFromSPIFFS(("/" + id + ".txt").c_str());
  Serial.println("Loaded position [" + status + "] for switch [" + id + "].");
  if (status == "") {
    on = false;
    savePosition();
  } else {
    on = (status == "1");
  }
}

// => Initialise switches with hard coded defaults
//    Load saved positions but don't apply yet
void ToggleSwitch::init() {
  pinMode(pin, OUTPUT);
  setDefault(false);
  loadPosition();
}

// => Turn on and apply
void ToggleSwitch::turnOn(bool save) {
  on = true;
  apply();
  if (save) savePosition();
}

// => Turn off and apply
void ToggleSwitch::turnOff(bool save) {
  on = false;
  apply();
  if (save) savePosition();
}

// => Set default position and apply
void ToggleSwitch::setDefault(bool save) {
  on = defaultP;
  apply();
  if (save) savePosition();
}

// Apply virtual position to I/O pin
void ToggleSwitch::apply() {
  if (on) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }
}

// Functions working on global array of ToggleSwitches
// ***************************************************
// => Initialise status for all switches (pinMode, digitalWrite to default)
void initSwitches() {
  for (int i = 0; i < n_switches; i++) {
    switches[i].init();
  }
}

// => Set switches I/O pins to loaded positions (digitalWrite)
void switchesApply() {
  for (int i = 0; i < n_switches; i++) {
    switches[i].apply();
  }
}

// => Set a switch to on or off
void setSwitch(const String id, const bool position) {
  for (int i = 0; i < n_switches; i++) {
    if (switches[i].getID() == id) {
      if (position) {
        switches[i].turnOn();
      } else {
        switches[i].turnOff();
      }
      break;
    }
  }
}

// => Turn off all switches without saving position
void switchesOffTemporary() {
  for (int i = 0; i < n_switches; i++) {
    switches[i].turnOff(false);
  }
}

// => Turn all switches to default position
void switchesDefault(bool save) {
  for (int i = 0; i < n_switches; i++) {
    switches[i].setDefault(save);
  }
}

// => Reset switches to saved positions
void switchesReset() {
  for (int i = 0; i < n_switches; i++) {
    switches[i].loadPosition();
    switches[i].apply();
  }
}

// => Create switches list-items for HTML
String switchesListHTML() {
  String s = "";
  for (int i = 0; i < n_switches; i++) {
    s += switches[i].getHTML();
  }
  return s;
}

// => Create JSON string for switches
String getSwitchesJSON() {
  String json = "{";
  for (int i = 0; i < n_switches; i++) {
    json += switches[i].getJSON() + ",";
  }
  json.remove(json.length() - 1);
  json += "}";
  return json;
}