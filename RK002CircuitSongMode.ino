#include <RK002.h>
//#define DEBUG

// App name, Author, Version, GUID (generate your own at https://www.guidgenerator.com/)
RK002_DECLARE_INFO("Song Mode for Novation Circuit", "zubkovyuriy@gmail.com", "1.0", "c1825f6a-45fd-4ccc-b470-e3064ee4f94f");

// Constants
enum SequencerActionType {
  // 0 - means empty sequencer slot
  SessionChange = 1,
  StopSequence = 2,
  LoopSessions = 3,
  LoopActions = 4,
  JumpToAction = 5
                 // 6,7 - reserved
};

enum SequencerState {
  Disabled,
  Play,
  Record
};

enum SequencerSubstate {
  Idle,
  ConfirmationSoundPlaying,
  Recording,
  Playing
};

const byte SESSION_CHANNEL = 15; // Session channel is always 16
const byte CONFIRMATION_NOTE = 62; // Circuit's Drum 2, we can place it to the config parameters later
const byte CONFIRMATION_CHANNEL = 9; // Default channel 10, we can place it to the config parameters later

const int  CONTROL_BUTTON_LONG_PRESS_TIME = 1000; // ms
const byte SHORT_NOTE_INTERVAL = 100; // ms
const byte LONG_NOTE_INTERVAL = 150; // ms

// Not sure how to do this better on C, feel free to fix
typedef struct {
  byte action; // Action Type Id
  byte address1; // SessionChange - session index; JumpToAction - action index; LoopSessions - start session index
  byte address2; // LoopSessions - end session index
  byte counter; // SessionChange - beats; JumpToAction - beats; LoopSessions - repeats
} SequenceEntry;

SequenceEntry sequence[30];

// Vars
byte currentState = SequencerState::Disabled;
byte currentSubState = SequencerSubstate::Idle;
byte shouldRepeatSoundTimes = 0;
byte intervalBetweenNotes = LONG_NOTE_INTERVAL;
int controlButtonLastTimeDown = 0;
bool controlButtonDown = false;
int clockTicks = 0;
byte beatsSinceLastAction = 1;
byte beatsUntilNextAction = 4;
byte patternLengthReminder = 0;
byte currentSequencerAction = 0;
byte lastSessionIndex = 0;
bool loopingSessions = false;
byte loopingSessionsRepeats = 0; //0...7 = 8 repeats = 3 bits

// Timer
int timer = 0;
// void function type for passing it as an argument to another functions
typedef void (*TimerCallback)();
TimerCallback pTimerCallback;

// Arduino setup function
void setup() {
  RK002_clockSetMode(0); // 0 - Auto, 1 - internal, 2 - external clock
  loadSequence();
#ifdef DEBUG
  printSequence();
#endif
}

// Arduino loop function
void loop() {
  if (controlButtonDown && (currentSubState == SequencerSubstate::Idle)) {
    int timePressed = millis() - controlButtonLastTimeDown;
    // Long press
    if (timePressed >= CONTROL_BUTTON_LONG_PRESS_TIME) {
      controlButtonDown = false;
      setCurrentState(SequencerState::Record);
    }
  }
}

void setTimer(int timerMs, TimerCallback clb) {
  timer = timerMs / RK002_HEARTBEAT_IN_MS; // RK002_HEARTBEAT_IN_MS = 10ms
  pTimerCallback = clb;
}

void RK002_onHeartBeat() {
  // Is timer running ?
  if (timer != 0) {
    // -10 ms tick
    timer--;

    // Timeout - action time!
    if (timer == 0 && pTimerCallback) {
      pTimerCallback();
    }
  }
}

// === Timer callbacks ===

void sendConfirmationNoteOff() {
  RK002_sendNoteOff(CONFIRMATION_CHANNEL , CONFIRMATION_NOTE, 127);
  shouldRepeatSoundTimes = shouldRepeatSoundTimes - 1;

  if (shouldRepeatSoundTimes > 0) {
    sendConfirmationNoteOn();
  } else {
    currentSubState = SequencerSubstate::Idle;
  }
}

void sendConfirmationNoteOn() {
  RK002_sendNoteOn(CONFIRMATION_CHANNEL , CONFIRMATION_NOTE, 127);
  setTimer(intervalBetweenNotes, sendConfirmationNoteOff);
}

// === Main Logic functions      ===

void setControlButtonState(bool down) {
  controlButtonDown = down;

  if (down) {
    controlButtonLastTimeDown = millis();
  }
}

void setCurrentState(byte newState) {
#ifdef DEBUG
  RK002_printf("State %d", newState);
#endif
  if (newState == SequencerState::Record) {
    clearSequencerMemory();
  }

  currentState = newState;
  playConfirmSound();
}

void playConfirmSound() {
  if (currentSubState != SequencerSubstate::Idle) {
    return;
  }

  currentSubState = SequencerSubstate::ConfirmationSoundPlaying;

  switch (currentState) {
    case SequencerState::Disabled: {
        shouldRepeatSoundTimes = 1;
        intervalBetweenNotes = SHORT_NOTE_INTERVAL;
        sendConfirmationNoteOn();
        break;
      }

    case SequencerState::Play: {
        shouldRepeatSoundTimes = 2;
        intervalBetweenNotes = LONG_NOTE_INTERVAL;
        sendConfirmationNoteOn();
        break;
      }

    case SequencerState::Record: {
        shouldRepeatSoundTimes = 5;
        intervalBetweenNotes = SHORT_NOTE_INTERVAL;
        sendConfirmationNoteOn();
        break;
      }
  }
}

void clearSequencerMemory() {
  // Erasing first 30 parameters, except last 2 configuration parameters
  word clearValue = 0; // 16 bit
  for (byte paramNr = 0; paramNr < 30; paramNr++) {
    sequence[paramNr] = { 0, 0, 0, 0 };
    RK002_paramSet(paramNr, clearValue);
  }
}

void processControllButtonState() {
  if ((currentSubState != SequencerSubstate::Idle) && (controlButtonDown == false)) {
    return;
  }
  int timeBetweenPressDownAndUp = millis() - controlButtonLastTimeDown;

  // Short press
  if (timeBetweenPressDownAndUp < CONTROL_BUTTON_LONG_PRESS_TIME) {
    switch (currentState) {
      case SequencerState::Disabled: {
          setCurrentState(SequencerState::Play);
#ifdef DEBUG
          printSequence();
#endif
          break;
        }

      case SequencerState::Play: {
          setCurrentState(SequencerState::Disabled);
          break;
        }

      case SequencerState::Record: {
          setCurrentState(SequencerState::Disabled);
          break;
        }
    }
  }

  // Long press handler located in the Arduino loop
}

void resetCounters() {
  clockTicks = 0;
  beatsSinceLastAction = 1;
  beatsUntilNextAction = 4;
  loopingSessions = false;
  loopingSessionsRepeats = 0;
  patternLengthReminder = 0;
  currentSequencerAction = 0;
}

SequenceEntry unpackEntryFrom16BitParamVal(word paramVal) {
  byte byte2 = paramVal >> 8; // action
  byte byte1 = paramVal & 0xFF;
  SequenceEntry entry = { 0, 0, 0, 0 };
  entry.action = (byte2 & 0b11100000) >> 5;

  // stop action - it has only action type, that was already parsed above
  switch (entry.action) {
    case SequencerActionType::SessionChange: {
        entry.address1 = byte2 & 0b00011111; // session index
        entry.counter = byte1; // beats counter
        break;
      }

    case SequencerActionType::LoopSessions: {
        entry.address1 = byte2 & 0b00011111; // start session index
        entry.address2 = byte1 & 0b00011111; // end session index
        entry.counter = (byte1 & 0b11100000) >> 5; // repeats
        break;
      }

    case SequencerActionType::LoopActions: {
        // not implemented
        break;
      }

    case SequencerActionType::JumpToAction: {
        entry.address1 = byte2 & 0b00011111; // action index
        entry.counter = byte1; // beats counter
        break;
      }
  }

  return entry;
}

word packEntryTo16BitParamVal(SequenceEntry entry) {
  word paramVal = 0;

  switch (entry.action) {
    case SequencerActionType::SessionChange: {
        paramVal = entry.action << 5;
        paramVal = (paramVal | entry.address1) << 8; // session index
        paramVal = paramVal | entry.counter;
        break;
      }

    case SequencerActionType::LoopSessions: {
        paramVal = entry.action << 5;
        paramVal = (paramVal | entry.address1) << 8; // start session index
        paramVal = paramVal | (entry.counter << 5); // repeats
        paramVal = paramVal | entry.address2; // end session index
        break;
      }

    case SequencerActionType::LoopActions: {
        // not implemented
        break;
      }

    case SequencerActionType::JumpToAction: {
        paramVal = entry.action << 5;
        // 0 - 29, 2 last parameters reserved for config
        paramVal = (paramVal | entry.address1) << 8; // to action index
        break;
      }

    case SequencerActionType::StopSequence: {
        paramVal = entry.action << 13; // 8 + 5
        break;
      }
  }

  return paramVal;
}

// We can save only session change in the standalone record mode (without web editor)
void saveNextSequencerEntry(byte action, int beats) {
#ifdef DEBUG
  RK002_printf("Save at beat %d", beats);
#endif
  if (currentSequencerAction > 29) {
    return;
  }

  if (currentSequencerAction > 0) {
    // saving beats to previous sequencer entry
    sequence[currentSequencerAction - 1].counter = beats;
  }

  switch (action) {
    case SequencerActionType::SessionChange: {
        sequence[currentSequencerAction] = { action, lastSessionIndex, 0, 0 };
        break;
      }
    case SequencerActionType::StopSequence: {
        sequence[currentSequencerAction] = { action, 0, 0, 0 };
        break;
      }
  }

  //RK002_paramSet(currentSequencerAction, paramVal);
  currentSequencerAction = currentSequencerAction + 1;
}

void processNextSequencerAction(bool incrementActionIndex = true, bool queueSessionChanges = true) {
  if (currentSequencerAction > 29) {
    stopPlayingSequence(true);
    return;
  }

  if (incrementActionIndex) {
    currentSequencerAction = currentSequencerAction + 1;
  }

  SequenceEntry sequenceEntry = sequence[currentSequencerAction];

  switch (sequenceEntry.action) {
    case SequencerActionType::StopSequence: { // stop
        stopPlayingSequence(true);
        break;
      }

    case SequencerActionType::SessionChange: {
        changeSession(queueSessionChanges, sequenceEntry.address1);

        lastSessionIndex = sequenceEntry.address1;
        beatsUntilNextAction = sequenceEntry.counter;
        beatsSinceLastAction = 1;
        break;
      }

    case SequencerActionType::JumpToAction: {
        currentSequencerAction = sequenceEntry.address1; // address1 represents action index in this case
        processNextSequencerAction(false); // recursive call without incrementing currentSequencerAction index
        break;
      }

    case SequencerActionType::LoopSessions: {
        if (!loopingSessions) {
          loopingSessions = true;
          loopingSessionsRepeats = sequenceEntry.counter + 1; // 0...7 = 8 repeats = 3 bits
          // change session on start index
          changeSession(queueSessionChanges, sequenceEntry.address1);
          lastSessionIndex = sequenceEntry.address1;
        } else {
          // if on the end session index of the loop
          if (lastSessionIndex == sequenceEntry.address2) {
            // change session to the start index, decrement repeats by 1
            loopingSessionsRepeats = loopingSessionsRepeats - 1;
            if (loopingSessionsRepeats == 0) {
              loopingSessions = false;
              processNextSequencerAction();
              return;  // exit looping mode
            } else {
              changeSession(queueSessionChanges, sequenceEntry.address1);
              lastSessionIndex = sequenceEntry.address1;
            }
          } else if (sequenceEntry.address1 <= sequenceEntry.address2) {
            // if start index is more than end index - increment current session index
            // increment session index and change session
            if(lastSessionIndex < 31) { lastSessionIndex = lastSessionIndex + 1; }
            changeSession(queueSessionChanges, lastSessionIndex);
          } else {
            // if start index is less than end index - decrement current session index
            // decrement session index and change session
            if (lastSessionIndex > 0) { lastSessionIndex = lastSessionIndex - 1; }
            changeSession(queueSessionChanges, lastSessionIndex);
          }
        }

        beatsUntilNextAction = 32; // one session length = 32 beats
        beatsSinceLastAction = 1;
        break;
      }

    default: { // 0 - empty
        stopPlayingSequence(true);
        break;
      }
  }
}

void changeSession(bool queued, byte sessionIndex) {
  if (queued) {
    RK002_sendProgramChange(SESSION_CHANNEL, sessionIndex + 64); // select session (queued)
    // We need to calculate pattern length reminder, because session changes are queued
    // Maybe there is more efficient way to do this
    int lengthReminder = beatsSinceLastAction % 4;
    if (lengthReminder > 0) {
      patternLengthReminder = 4 - lengthReminder;
    }
  } else {
    RK002_sendProgramChange(SESSION_CHANNEL, sessionIndex);
  }
}

#ifdef DEBUG
void printSequence() {
  for (byte i = 0; i < 30; i++) {
    RK002_printf("Entry %d, act:%d, adr1:%d, adr2:%d, beats:%d", i, sequence[i].action, sequence[i].address1, sequence[i].address2, sequence[i].beats);
  }
}
#endif

void saveSequence() {
  for (byte i = 0; i < 30; i++) {
    SequenceEntry currentEntry = sequence[i];
    word paramVal = packEntryTo16BitParamVal(currentEntry);
    RK002_paramSet(i, paramVal);
  }
}

void loadSequence() {
  for (byte i = 0; i < 30; i++) {
    word paramVal = RK002_paramGet(i);
    SequenceEntry entry = unpackEntryFrom16BitParamVal(paramVal);
    sequence[i] = entry;
  }
}

void startRecordingSequence() {
#ifdef DEBUG
  RK002_printf("StartRecording");
#endif
  resetCounters();
  currentSubState = SequencerSubstate::Recording;
  saveNextSequencerEntry(SequencerActionType::SessionChange, 0);
}

void stopRecordingSequence() {
#ifdef DEBUG
  RK002_printf("StopRecording");
#endif
  currentSubState = SequencerSubstate::Idle;
  currentState = SequencerState::Play;
  saveNextSequencerEntry(SequencerActionType::StopSequence, beatsSinceLastAction);
  saveSequence();
#ifdef DEBUG
  printSequence();
#endif
}

void startPlayingSequence() {
#ifdef DEBUG
  RK002_printf("StartPlaying");
#endif
  resetCounters();
  currentSubState = SequencerSubstate::Playing;
  processNextSequencerAction(false, false); // don't increment actionIndex here on start, don't use session change queue
}

void stopPlayingSequence(bool sendMidi) {
#ifdef DEBUG
  RK002_printf("StopPlaying");
#endif
  currentSubState = SequencerSubstate::Idle;
  //  if (sendMidi) { // we can't send stop in MIDI slave mode
  //    RK002_sendStop();
  //    //RK002_sendReset();
  //  }
}

// === MIDI processing functions ===

bool RK002_onProgramChange(byte channel, byte nr) {
  // Session control channel number is never changing on Circuit
  if (channel == SESSION_CHANNEL) {
    if(nr >= 64) {
      lastSessionIndex = nr - 64; // queued program change has indexes > 64
    } else {
      lastSessionIndex = nr;
    }

    if (currentSubState == SequencerSubstate::Recording) {
      saveNextSequencerEntry(SequencerActionType::SessionChange, beatsSinceLastAction);
      beatsSinceLastAction = 0;
    }
  }

  return false;
}

bool RK002_onNoteOn(byte channel, byte key, byte velocity) {
  // Very bottom key in the lowest octave on the Circuit (in any scale and synth number)
  if (key != 0) {
    return false;
  }
  if (currentSubState != SequencerSubstate::Idle || (controlButtonDown != false)) {
    return false;
  }

  setControlButtonState(true);
  return false;
}

bool RK002_onNoteOff(byte channel, byte key, byte velocity) {
  if (key != 0) {
    return false;
  }
  if ((currentSubState != SequencerSubstate::Idle) || (controlButtonDown != true)) {
    return false;
  }

  setControlButtonState(false);
  processControllButtonState();
  return false;
}

// Clock tick handler is called 24x per quarter note
boolean RK002_onClock() {
  if (currentSubState == SequencerSubstate::Recording
      || currentSubState == SequencerSubstate::Playing) {
    clockTicks = (clockTicks + 1) % 24;

    if (clockTicks == 0 && beatsSinceLastAction < 255) {
      switch (currentSubState) {
        case SequencerSubstate::Recording: {
            beatsSinceLastAction = beatsSinceLastAction + 1;
            break;
          }
        case SequencerSubstate::Playing: {
            if (patternLengthReminder == 0) {
              if (beatsSinceLastAction >= beatsUntilNextAction - 1) {
                if(loopingSessions) {
                  processNextSequencerAction(false); // stay in the same action handler
                } else {
                  processNextSequencerAction();
                }
              } else {
                beatsSinceLastAction = beatsSinceLastAction + 1;
              }

            } else {
              patternLengthReminder = patternLengthReminder - 1;
            }

            break;
          }
      }
    }
  }

  return false;
}

void RK002_onParamSet(unsigned param_nr, int val) {
  // In case if editor connected we need to update current loaded parameters
  if (param_nr < 30) {
    sequence[param_nr] = unpackEntryFrom16BitParamVal(val);
  } else {
    // 30-31 config params
  }
}

bool RK002_onStart() {
  switch (currentState) {
    case SequencerState::Play: {
        startPlayingSequence();
        break;
      }

    case SequencerState::Record: {
        startRecordingSequence();
        break;
      }
  }

  return false;
}

bool RK002_onContinue() {
  switch (currentState) {
    case SequencerState::Play: {
        startPlayingSequence();
        break;
      }

    case SequencerState::Record: {
        startRecordingSequence();
        break;
      }
  }

  return false;
}

bool RK002_onStop() {
  switch (currentState) {
    case SequencerState::Play: {
        stopPlayingSequence(false);
        break;
      }

    case SequencerState::Record: {
        stopRecordingSequence();
        break;
      }
  }

  return false;
}
