#include <RK002.h>
/*
  RK002_DECLARE_PARAM(name, flags, min, max, def)
    name = the name of the parameter
    flags = flags for the creation of the parameter: currently only the following values are supported: 
    (0=default, 1=store in EEPROM)
    min = minimal value of the parameter
    max = maximal value of the parameter
    def = default value of the parametername = the name of the parameter

  Matrix of the parameters for storing automation sequence.
  Each value is 16 bits, or 2 bytes.
  
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
         action session          beats 
                                 (how much beats until next action)
  
  Action 0b010 - Stop Sequencer.
         0b010   00000        0b00000000
          /   \                /        \
         action                  beats 
                                 (0 - there is no next action)

  Action 0b011 - Loop Sessions.
  From start session index to end session index 0-31.
         0b011   00000           0b000         00000
          /   \ /     \           /   \       /     \
        action  sessStartIndx   repeatCount   sessStopIndx 

  Action 0b100 - Loop Sequencer.
  From start action index to the end action index 0-29.
         0b100   00000           0b000         00000
          /   \ /     \           /   \       /     \
        action  actStartIndx   repeatCount   actStopIndx 

  Action 0b101 - Jump to Action.
  Jump to action index 0-29.
         0b101   00000         0b00000000
          /   \ /     \         /        \
        action  actionInd         beats 
                                  (how much beats until next action)
*/

RK002_DECLARE_PARAM(ACT1, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT2, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT3, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT4, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT5, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT6, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT7, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT8, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT9, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT10, 1, 0, 65535, 0);

RK002_DECLARE_PARAM(ACT11, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT12, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT13, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT14, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT15, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT16, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT17, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT18, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT19, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT20, 1, 0, 65535, 0);

RK002_DECLARE_PARAM(ACT21, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT22, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT23, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT24, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT25, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT26, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT27, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT28, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT29, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(ACT30, 1, 0, 65535, 0);

// Reserved for configuration, 4 bytes total
RK002_DECLARE_PARAM(CFG1, 1, 0, 65535, 0);
RK002_DECLARE_PARAM(CFG2, 1, 0, 65535, 0);
