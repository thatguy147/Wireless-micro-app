#ifndef __MAIN_H_INCLUDED__
#define __MAIN_H_INCLUDED__

#include "mbed.h"
#include <string>

 /*
  *Function Prototypes
  *
  */
   void initCAN();
   void initSPI();
   void initDefaultAcceptedID();
   void addNewID(uint16_t canID);
   bool removeID(uint16_t canID);
   bool isCANIDRecognised(uint16_t id);
   std::string intToString(int i);
   uint16_t canIDStringToInt(string str);
   void handleCommand(string cmdID, string cmd);
   void mainReading();
   void configMode();
   void handleCANMessage(CANMessage can_msg); 
   
#endif 