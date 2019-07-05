#include "Decoder.h"

Decoder* Decoder::m_pInstance = NULL; 

Decoder* Decoder::Instance()
{
   if (m_pInstance == NULL)   // Only allow one instance of class to be generated.
      m_pInstance = new Decoder();
      
   return m_pInstance;
}

/// Initialise Decoder Class
/** 
  allows the object to be created once and reused without for every 
  new instruction received by clearing old instruction content and filling in new content
*/
void Decoder::DecodeNewSerial(String s)
{
  ClearExistingInstructions();
  serial = s;
  if(ContainsSplitterChar()){
      DecodeInstructions();
  }
}

/// Clear Existing instruction
/** 
  Function clears 3 main variables used by Decoder class.
  serial, state, instruction
*/
void Decoder::ClearExistingInstructions()
{
    serial = "";
    state = "";
    instruction = "";
}

/// Set New instruction
/** 
 *  sets state and instruction variable by retreiveing state 
 *  part and instruction part from the full instruction/whole instruction
*/
void Decoder::DecodeInstructions()
{
    state = serial.substring(0, serial.indexOf(SPLITTER_CHAR));
    instruction = serial.substring(serial.indexOf(SPLITTER_CHAR) + 1);

    if(state == "ZUMO|AUTO"){
      isAutoMode = true;
    }else{
      isAutoMode = false;
    }

    if(state == "PLAYER-NAME"){
      player_name = instruction;
    }
}

/// Contains Splitter Char
/** 
  checks if the current whole instruction contains 
  a splitter character which is a colon (:)
*/
bool Decoder::ContainsSplitterChar()
{
    return serial.indexOf(SPLITTER_CHAR) != NPOS;
}

/// Get State
/** 
 *  
*/
char Decoder::GetInstruction()
{
    return instruction.charAt(0);
}

/// Get Message state
/** 
  getter for state variable
*/
String Decoder::GetState()
{
    return state;
}

int Decoder::GetSpeedForMotor1(){
  return instruction.substring(0,instruction.indexOf('|')).toInt();
}

int Decoder::GetSpeedForMotor2(){
  return instruction.substring(instruction.indexOf('|')+1).toInt();
}
