#ifndef Decoder_H
#define Decoder_H

#include <Arduino.h>

class Decoder
{       
  public:
    static Decoder* Instance();
    
    //Constants
    const char MODE_AUTO = 'a';
    const char MODE_MANUAL = 'm';
    const String HEADER_SPEED = "SPEED";
    const String HEADER_STATE = "STATE";
    const char SPLITTER_CHAR = ':';
    const char ENDING_CHAR = ';';
    const int NPOS = -1;

    //Functions
    void DecodeNewSerial(String serial);
    char GetInstruction();
    String GetState();
    int GetSpeedForMotor1();
    int GetSpeedForMotor2();

    //Variables
    bool isAutoMode = false;
    String player_name = "";
  private:
    Decoder(){};
    Decoder(Decoder const&){};
    Decoder& operator=(Decoder const&){};
    static Decoder* m_pInstance;
    
    //Variables
    String state;
    String instruction;
    String serial;

   
    
    //Functions
    bool ContainsSplitterChar();
    void DecodeInstructions();
    void ClearExistingInstructions();
    
};

#endif
