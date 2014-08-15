/*************************
 *  #Include declarations
 *************************
 */
    #include "mbed.h"
    #include "main.h"
    #include "sstream"
    #include "SC16IS750.h"
    #include <string> 
 
 /*************************
 *  # Defines
 *************************
 */
 #define IP_STRING  "192.168.1.11";
 #define IP_PORT    "6001";
 #define openStr    "open";
  
/**************************
 *  Pins used: mbed lpc1768
 **************************
 */
    CAN can1(p9, p10);      // CAN1 uses pins 9 and 10 (tx, rx)
    SPI spi(p11, p12, p13); //MOSI, MISO, SCK
    Serial pc(USBTX,USBRX);
   
/**************************
 *  Object declaration
 **************************
 */
    
    SC16IS750_SPI serial_spi(&spi, p8);
    CANMessage can_MsgRx;
  
/**************************
 *  Local variables
 **************************
 */
  
    char        buffer[50];
    
    uint16_t    buffer_pos = 0;
    uint16_t    _CANIDArrayLength = 0;
    uint16_t    _CANIDAccepted [64];
  
    bool _isSPIRunning      = false;
    bool _isCANRunning      = false;
    bool _enableCANForUse   = true;
    bool _enableSPIForUse   = true;
    bool _inConfigMode      = false;
    bool _receivingMessage  = false; //We are in the middle of receiving a message;
    bool _readingComplete   = false; //We were reading a message, and now it's complete.
    
    string IP = "192.168.1.11";
    string port = "6001";
    string open = "open";
 
    void initCAN()
    {
        can1.frequency(500000);
        _isCANRunning = true;
        pc.printf("\r\nCAN Iniitalised"); 
    }
 
    void initSPI()
    {
        if(serial_spi.connected())
        {
            serial_spi.writeString("\r");
            serial_spi.writeString("$$$");
            wait(0.2); 
            serial_spi.writeString("\r\n");
            wait(0.2);
            std::string str = (open + " " + IP + " " + port + "\r\n");
            char * writable = new char[str.size() + 1];
            std::copy(str.begin(), str.end(), writable);
            writable[str.size()] = '\0'; // don't forget the terminating 0
            serial_spi.writeString(writable);
            wait(0.1);
            serial_spi.writeString("exit\r\n");
            delete[] writable;
            pc.printf("\r\nSPI Iniitalised");
            _isSPIRunning = true; 
        }
    }
    
    void initDefaultAcceptedID()
    {
        addNewID(0x01);
        addNewID(0x0B);
        addNewID(0x0C);
        pc.printf("\r\nDefault IDs added"); 
    }
    
    void addNewID(uint16_t canID)
    {
        _CANIDAccepted[_CANIDArrayLength] = canID;
        _CANIDArrayLength++;
    }
    
    bool removeID(uint16_t canID)
    {
        bool idRemoved = false;
        for(int i = 0; i <= _CANIDArrayLength; i++)
        {
            if(_CANIDAccepted[i] == canID)
            {
                _CANIDAccepted[i] = 0x00;
                idRemoved = true;
            }
        }
        
        return idRemoved;
    }
    
    bool isCANIDRecognised(uint16_t id)
    {
        bool recognised = false;
        for(int i = 0; i <= _CANIDArrayLength; i++)
        {
            if(_CANIDAccepted[i] == id)
            {
                recognised = true;
                break;
            }
        }
        return recognised;
    }
    
    std::string intToString(int i)
    {
        std::stringstream ss;
        std::string s;
        ss << std::hex << i;
        s = (i == 0) ? "00" : ss.str();

        return s;
    }
    
    uint16_t canIDStringToInt(string str)
    {
        uint16_t id;
        istringstream buffer(str);
        buffer >> id;
        return id;
    }
    
    void handleCommand(string cmdID, string cmd)
    {
        if(cmdID == "FQY")
        {
            
        }
        else if(cmdID == "AID")
        {
            uint16_t id = canIDStringToInt(cmd);
            addNewID(id);
        }
        else if(cmdID == "RID")
        {
            uint16_t id = canIDStringToInt(cmd);
            removeID(id);
        }
    }
    
    void handleCANMessage(CANMessage can_msg)
    {
        
            if(isCANIDRecognised(can_MsgRx.id))
            {
                std::string _id = intToString(can_MsgRx.id);
                std::string _DB0 = intToString(can_MsgRx.data[0]);
                std::string _DB1 = intToString(can_MsgRx.data[1]);
                std::string _DB2 = intToString(can_MsgRx.data[2]);
                std::string _DB3 = intToString(can_MsgRx.data[3]);
                std::string _DB4 = intToString(can_MsgRx.data[4]);
                std::string _DB5 = intToString(can_MsgRx.data[5]);
                std::string _DB6 = intToString(can_MsgRx.data[6]);
                std::string _DB7 = intToString(can_MsgRx.data[7]);
                
                string str = "[" + _id + " " + _DB0 + " " + _DB1 + " " + _DB2  + " " + _DB3 + " " + _DB4 + " " + _DB5 + " " + _DB6 + " " + _DB7 + "]\r";
                std::string id = str;
                char * writable = new char[str.size() + 1];
                std::copy(str.begin(), str.end(), writable);
                writable[str.size()] = '\0'; // don't forget the terminating 0
                pc.printf(writable);
                serial_spi.writeString(writable);
                delete[] writable;
            }
            else{
                //This is a message that has a messageID  we are not interested in.
            }
    }
    
    
    void mainReading()
    {
        while(1)
        {
            if((can1.read(can_MsgRx)) && (_inConfigMode == false))
            {
                handleCANMessage(can_MsgRx); //Sending message to Android 
            }
            
            while(serial_spi.readable()) //While we have something to read from SPI (from the Android)
            {
                char c = (char)serial_spi.getc(); //read Byte from WiFiModule 
                pc.printf("%c",c);
                if((c == '[') || (c =='{')) //'[' is the START of a Message '[' = CAN Message, '{' Configuration Message
                {
                    pc.printf("got start");
                    _receivingMessage = true;
                }
                 
                else if((c == ']') || (c == '}'))//']' is the END of a Message '[' = CAN Message, '{' Configuration Message
                {
                    pc.printf("got end");
                    buffer[buffer_pos] = c;
                    buffer_pos++;
                    //handleInput();
                    _receivingMessage = false;
                    _readingComplete = true;
                }
                
                if(_receivingMessage == true) // in the middle of receiving a message
                {
                    buffer[buffer_pos] = c;
                    buffer_pos++;
                }
                
                if(_readingComplete == true) //Received a message with complete SOF and EOF.
                {
                    if((buffer[0] == '[' ) && (buffer[31] == ']') && (buffer_pos == 32))
                    {
                        buffer[0] = '\0';
                        buffer_pos = 0;
                        //handleInput();
                        //we have a CAN message, now handle it.
                    }
                    else if((buffer[0] == '{' ) && (buffer[buffer_pos-1] == '}'))
                    {
                        //_inConfigMode = true; 
                        //buffer[0] = '\0';
                        //buffer_pos = 0;
                        configMode();
                        
                    }
                    else{
                        //We SHOULDN'T enter here, if we do, clear the buffer, we got a dodgy message
                        pc.printf("We are in the unwanted else");
                        buffer[0] = '\0';
                        buffer_pos = 0;
                    }
                    
                    _readingComplete = false; // we are done now, finished our reading, reset buffer.
                    buffer[0] = '\0';
                    buffer_pos = 0;
                }
            }
        }
    }
        
    void configMode()
    {
        char command_type = buffer[1]; //{SI 123}
        pc.printf("inconfig\n");
        switch(command_type){
            case 'S':
            pc.printf("in switch\n");
                        if(buffer[2] == 'I'){
                            pc.printf("in Set ID Case");
                            std::string stringID = "";
                            stringID += buffer[3];
                            stringID += buffer[4];
                            stringID += buffer[5];
                            stringID += buffer[6];
                            pc.printf("This is what you're looking for: ");
                            char * writable = new char[stringID.size()+1];
                            std::copy(stringID.begin(), stringID.end(), writable);
                            writable[stringID.size()] = '\0';
                            pc.printf(writable);
                            pc.printf(" -------");
                            
                            uint16_t id = canIDStringToInt(stringID);
                            addNewID(id);
                            pc.printf("added");
                            _inConfigMode = false;
                            serial_spi.writeString("{ID Added}");
                            serial_spi.writeString(" ");
                            //serial_spi.flush();
                            //pc.printf(stringID);
                            //uint16_t = canIDStringToInt(//buffer[3] -> buffer[bufferpos-1]
                        }
                        break;
            
            case 'R':
                        if(buffer[2] == 'I'){
                            std::string stringID = "";
                            stringID += buffer[3];
                            stringID += buffer[4];
                            stringID += buffer[5];//(3,buffer[4]);
                            stringID += buffer[6];
                            pc.printf("This is what you're looking for: ");
                            char * writable = new char[stringID.size()+1];
                            std::copy(stringID.begin(), stringID.end(), writable);
                            writable[stringID.size()] = '\0';
                            pc.printf(writable);
                            pc.printf(" -------");
                            
                            uint16_t id = canIDStringToInt(stringID);
                            bool hasIDBeenRemoved = removeID(id);
                            if(hasIDBeenRemoved){
                                serial_spi.writeString("{ID Removed}");
                                serial_spi.writeString(" ");
                                //serial_spi.flush();
                            }
                            else{
                                
                                serial_spi.writeString("ID didn't exist/wasn't removed}");
                                serial_spi.writeString(" ");
                                //serial_spi.flush();
                                
                            }
                            pc.printf("removed");
                            _inConfigMode = false;
                            //Remove ID
                        }
                        break;
                        
            case 'E':
                        if(buffer[2] == 'C'){
                            if(_inConfigMode){
                                _inConfigMode = false;
                            }
                            else{
                                _inConfigMode = true;
                            }
                            pc.printf("in the method anyway");
                        }
                        break;
            default:
                        break;
        
        }
        
        
    }
    
 
    int main()
    {
        pc.printf("PiFi Init \r\n");
        initSPI();
        initCAN();
        initDefaultAcceptedID();
        if(_isSPIRunning && _isCANRunning)
        {
            mainReading();
        }
    }