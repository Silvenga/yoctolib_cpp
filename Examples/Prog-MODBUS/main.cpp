#include "yocto_api.h"
#include "yocto_relay.h"
#include <iostream>
#include <ctype.h>
#include <stdlib.h>

using namespace std;

#include <iostream>

#include "yocto_api.h"
#include "yocto_serialport.h"

using namespace std;

int main(int argc, const char * argv[])
{
    string errmsg;

    if (YAPI::RegisterHub("usb", errmsg) != YAPI::SUCCESS) {
        cerr << "RegisterHub error : " << errmsg<<endl;
        return 1;
    }

    YSerialPort *serialPort;
    if (argc > 1) {
        serialPort = YSerialPort::FindSerialPort(string(argv[1]));
    } else {
        serialPort = YSerialPort::FirstSerialPort();
        if (serialPort == NULL) {
            cerr <<"No module connected (check USB cable)"<<endl;
            return 1;
        }
    }
    
    int slave, reg, val;
    string cmd;
    do {
        cout << "Please enter the MODBUS slave address (1...255)" << endl;
        cout << "Slave: ";
        cin >> slave;
    } while(slave < 1 || slave > 255);
    do {
        cout << "Please select a Coil No (>=1), Input Bit No (>=10001+)," << endl;
        cout << "       Register No (>=30001) or Input Register No (>=40001)" << endl;
        cout << "No: " ;
        cin >> reg;
    } while(reg < 1 || reg >= 50000 || (reg % 10000) == 0);
    while(true) {
        if(reg >= 40001) {
            val = serialPort->modbusReadInputRegisters(slave, reg-40001, 1)[0];
        } else if(reg >= 30001) {
            val = serialPort->modbusReadRegisters(slave, reg-30001, 1)[0];
        } else if(reg >= 10001) {
            val = serialPort->modbusReadInputBits(slave, reg-10001, 1)[0];
        } else {
            val = serialPort->modbusReadBits(slave, reg - 1, 1)[0];
        }
        cout << "Current value: " << val << endl;
        cout << "Press R to read again, Q to quit";
        if((reg % 30000) < 10000) {
            cout << " or enter a new value";
        }
        cout << ": " << endl;
        cin >> cmd;
        if(cmd == "q" || cmd == "Q") break;
        if (cmd != "r" && cmd != "R" && (reg % 30000) < 10000) {
            val = atoi(cmd.c_str());
            if(reg >= 30001) {
                serialPort->modbusWriteRegister(slave, reg - 30001, val);
            } else {
                serialPort->modbusWriteBit(slave, reg - 1, val);
            }
        }
    }

    YAPI::FreeAPI();
    return 0;
}
