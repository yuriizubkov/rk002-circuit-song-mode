# Song Mode for Novation Circuit with RK002

This firmware is for the [RK002 from Retrokits.com](https://www.retrokits.com/rk-002/). It implements the ability to use the Song Mode (chaining sessions) for the Novation Circuit.


The firmware can be used both in stand-alone mode and in conjunction with a [Web Editor](https://github.com/yuriizubkov/rk002-sequence-editor). In stand-alone mode, the sequencer can record up to 30 session changes with automatic counting and storage of the number of beats between events. The web editor allows you to add additional actions to the sequence, edit the sequence, or create a sequence from scratch and upload it into the RK002 memory. The recorded sequence is stored in the non-volatile memory of the RK002 and will be available after turning the device off and on.

## How to flash the firmware
How to flash this firmware into your RK002 is described [here on the retrokits.com](https://www.retrokits.com/rk002-duy/)

## Operating modes

### Stand-alone mode
- Connect your RK002 (with this firmware installed) to the Circuit as MIDI loopback cable. Yellow end goes to the input, black - to the output of the Circuit. 
- Make sure that you have <strong>Drum 2</strong> enabled in the mixer of your Circuit, RK002 will use this channel for the confirmation sounds. 
- Choose Synth 1 or Synth 2 and navigate to the lower octave. First note of the lower octave is used for switching between operation modes of the RK002. By default it is in disabled mode. By pressing once at the first note we will toggle between modes and you will hear confirmation sounds from the Drum 2 channel:

<strong>1 snare</strong> - disabled mode. </br>
<strong>2 snares</strong> - "play sequence mode" is armed and will be started after you press play on the Circuit.<br/>
<strong>Drum Roll</strong> - long press on the first note of the lower octave until you hear drum roll and the sequence data will be cleared and sequencer will be armed for recording. Recording will start after you press play on the Circuit, and will stop when you press stop. RK002 will be switched to the play mode, recorded sequence will be saved to the RK002 memory.

### Using with Web Editor
If you want to edit the sequence or create a new one in offline mode, you can use RK002 with [Web Editor](https://github.com/yuriizubkov/rk002-sequence-editor).</br>
Online version can be found [https://lensflare.dev/rk002-sequence-editor/](https://lensflare.dev/rk002-sequence-editor/).

You need to connect your RK002 to the PC as a MIDI loopback cable. Yellow end goes to the input, black - to the output of the MIDI interface.

More information about functionality of the web editor can be found on this repository: [https://github.com/yuriizubkov/rk002-sequence-editor](https://github.com/yuriizubkov/rk002-sequence-editor)

## How sequence data is storing in the RK002
RK002 has the ability to store 16 bit user parameters (32 parameters in total) in the non-volatile memory. This firmware using this functionality to pack sequencer data into these parameters as follows:
```
Each parameter value is 16 bits, or 2 bytes.
  
  2nd byte, 3 MSBs is an action command:
    - 0b000 (0) Empty - no action
    - 0b001 (1) Session Change
    - 0b010 (2) Stop
    - 0b011 (3) Loop Sessions
    - 0b100 (4) Loop Actions
    - 0b101 (5) Jump to Action
    
  2nd byte, 5 LSBs is usually a session index 0-31 session.
  1st byte depends on an action id:

  Action 0b001 - Session Change.
         0b001   00000        0b00000000
          /   \ /     \        /        \
         action session          beats until next action
  
  Action 0b010 - Stop Sequencer.
         0b010   00000        0b00000000
          /   \                /        \
         action                  beats, 0 - there is no next action

  Action 0b011 - Loop Sessions.
  From start session index to end session index 0-31. Repeat count 8 max.
         0b011   00000           0b000         00000
          /   \ /     \           /   \       /     \
         action sessStartIndx   repeatCount   sessStopIndx 

  Action 0b100 - Loop Sequencer.
  From start action index to the end action index 0-29. Repeat count 8 max.
         0b100   00000           0b000         00000
          /   \ /     \           /   \       /     \
         action actStartIndx   repeatCount   actStopIndx 

  Action 0b101 - Jump to Action.
  Jump to action index 0-29.
         0b101   00000         0b00000000
          /   \ /     \         /        \
         action actionInd         beats until next action
```
We are using first 30 parameters as a storage for the sequencer data. Last 2 parameters are reserved for configuration.

## Known issues and limitations
- Loop Actions - is not implemented.
- Beats counter is 8 bit, so it is 255 beats max between sequencer actions can be set.
- Loop Sessions can loop only within sequentially session numbers, for example 1,2,3,4,5... or 8,7,6,5... and repeats are limited to 8 (3 bits).
- Jump to Action should be used with caution (available on the web editor). There is almost no logic checks for the sequence. For example you can jump on a jump action that point to the first jump action, and you will get infinite loop and it'll freeze the RK002 :)

## Copyright
You can fork, use, modify this project as you wish.