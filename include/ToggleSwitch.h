#ifndef SWITCH_H
#define SWITCH_H

#include <Arduino.h>
#include <globals.h>

// Declar ToggleSwitch class
// *************************
class ToggleSwitch {
private:
  int pin, defaultP, on, locked;
  String id, name, dscr;

public:
  ToggleSwitch(int p_pin, String p_id, String p_name, String p_dscr, bool p_defaultP);

  // => get ID of switch
  String getID() { return id; }

  // => Get HTML String of switch
  String getHTML();

  // => Get JSON string of switch for sending over websocket
  String getJSON();

  // => Save switch position to file
  void savePosition();

  // => Load switch position from file
  void loadPosition();

  // => Initialise switches with hard coded defaults
  //    Load saved positions but don't apply yet
  void init();

  // => Turn on and apply
  void turnOn(bool save = true);

  // => Turn off and apply
  void turnOff(bool save = true);

  // => Set default position and apply
  void setDefault(bool save = true);

  // Apply virtual position to I/O pin
  void apply();
};

// Functions working on global array of ToggleSwitches
// ***************************************************
// Initialise switches
extern ToggleSwitch switches[n_switches];

// => Initialise status for all switches (pinMode, digitalWrite to default)
void initSwitches();

// => Set switches I/O pins to loaded positions (digitalWrite)
void switchesApply();

// => Set a switch to on or off
void setSwitch(const String id, const bool position);

// => Turn off all switches without saving position
void switchesOffTemporary();

// => Turn all switches to default position
void switchesDefault(bool save = true);

// => Reset switches to saved positions
void switchesReset();

// => Create switches list-items for HTML
String switchesListHTML();

// => Create JSON string for switches
String getSwitchesJSON();

#endif // SWITCH_H
