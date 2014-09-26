/*********************************************************************
 *
 * $Id: yocto_serialport.cpp 17777 2014-09-23 08:33:15Z seb $
 *
 * Implements yFindSerialPort(), the high-level API for SerialPort functions
 *
 * - - - - - - - - - License information: - - - - - - - - - 
 *
 *  Copyright (C) 2011 and beyond by Yoctopuce Sarl, Switzerland.
 *
 *  Yoctopuce Sarl (hereafter Licensor) grants to you a perpetual
 *  non-exclusive license to use, modify, copy and integrate this
 *  file into your software for the sole purpose of interfacing
 *  with Yoctopuce products.
 *
 *  You may reproduce and distribute copies of this file in
 *  source or object form, as long as the sole purpose of this
 *  code is to interface with Yoctopuce products. You must retain
 *  this notice in the distributed source file.
 *
 *  You should refer to Yoctopuce General Terms and Conditions
 *  for additional information regarding your rights and
 *  obligations.
 *
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED 'AS IS' WITHOUT
 *  WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING 
 *  WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO
 *  EVENT SHALL LICENSOR BE LIABLE FOR ANY INCIDENTAL, SPECIAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 *  COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR 
 *  SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT 
 *  LIMITED TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR
 *  CONTRIBUTION, OR OTHER SIMILAR COSTS, WHETHER ASSERTED ON THE
 *  BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE), BREACH OF
 *  WARRANTY, OR OTHERWISE.
 *
 *********************************************************************/


#define _CRT_SECURE_NO_DEPRECATE //do not use windows secure crt
#include "yocto_serialport.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

YSerialPort::YSerialPort(const string& func): YFunction(func)
//--- (SerialPort initialization)
    ,_serialMode(SERIALMODE_INVALID)
    ,_protocol(PROTOCOL_INVALID)
    ,_rxCount(RXCOUNT_INVALID)
    ,_txCount(TXCOUNT_INVALID)
    ,_errCount(ERRCOUNT_INVALID)
    ,_msgCount(MSGCOUNT_INVALID)
    ,_lastMsg(LASTMSG_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackSerialPort(NULL)
    ,_rxptr(0)
//--- (end of SerialPort initialization)
{
    _className="SerialPort";
}

YSerialPort::~YSerialPort() 
{
//--- (YSerialPort cleanup)
//--- (end of YSerialPort cleanup)
}
//--- (YSerialPort implementation)
// static attributes
const string YSerialPort::SERIALMODE_INVALID = YAPI_INVALID_STRING;
const string YSerialPort::PROTOCOL_INVALID = YAPI_INVALID_STRING;
const string YSerialPort::LASTMSG_INVALID = YAPI_INVALID_STRING;
const string YSerialPort::COMMAND_INVALID = YAPI_INVALID_STRING;

int YSerialPort::_parseAttr(yJsonStateMachine& j)
{
    if(!strcmp(j.token, "serialMode")) {
        if(yJsonParse(&j) != YJSON_PARSE_AVAIL) goto failed;
        _serialMode =  _parseString(j);
        return 1;
    }
    if(!strcmp(j.token, "protocol")) {
        if(yJsonParse(&j) != YJSON_PARSE_AVAIL) goto failed;
        _protocol =  _parseString(j);
        return 1;
    }
    if(!strcmp(j.token, "rxCount")) {
        if(yJsonParse(&j) != YJSON_PARSE_AVAIL) goto failed;
        _rxCount =  atoi(j.token);
        return 1;
    }
    if(!strcmp(j.token, "txCount")) {
        if(yJsonParse(&j) != YJSON_PARSE_AVAIL) goto failed;
        _txCount =  atoi(j.token);
        return 1;
    }
    if(!strcmp(j.token, "errCount")) {
        if(yJsonParse(&j) != YJSON_PARSE_AVAIL) goto failed;
        _errCount =  atoi(j.token);
        return 1;
    }
    if(!strcmp(j.token, "msgCount")) {
        if(yJsonParse(&j) != YJSON_PARSE_AVAIL) goto failed;
        _msgCount =  atoi(j.token);
        return 1;
    }
    if(!strcmp(j.token, "lastMsg")) {
        if(yJsonParse(&j) != YJSON_PARSE_AVAIL) goto failed;
        _lastMsg =  _parseString(j);
        return 1;
    }
    if(!strcmp(j.token, "command")) {
        if(yJsonParse(&j) != YJSON_PARSE_AVAIL) goto failed;
        _command =  _parseString(j);
        return 1;
    }
    failed:
    return YFunction::_parseAttr(j);
}


/**
 * Returns the serial port communication parameters, as a string such as
 * "9600,8N1". The string includes the baud rate, the number of data bits,
 * the parity, and the number of stop bits. An optional suffix is included
 * if flow control is active: "CtsRts" for hardware handshake, "XOnXOff"
 * for logical flow control and "Simplex" for acquiring a shared bus using
 * the RTS line (as used by some RS485 adapters for instance).
 * 
 * @return a string corresponding to the serial port communication parameters, as a string such as
 *         "9600,8N1"
 * 
 * On failure, throws an exception or returns Y_SERIALMODE_INVALID.
 */
string YSerialPort::get_serialMode(void)
{
    if (_cacheExpiration <= YAPI::GetTickCount()) {
        if (this->load(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
            return YSerialPort::SERIALMODE_INVALID;
        }
    }
    return _serialMode;
}

/**
 * Changes the serial port communication parameters, with a string such as
 * "9600,8N1". The string includes the baud rate, the number of data bits,
 * the parity, and the number of stop bits. An optional suffix can be added
 * to enable flow control: "CtsRts" for hardware handshake, "XOnXOff"
 * for logical flow control and "Simplex" for acquiring a shared bus using
 * the RTS line (as used by some RS485 adapters for instance).
 * 
 * @param newval : a string corresponding to the serial port communication parameters, with a string such as
 *         "9600,8N1"
 * 
 * @return YAPI_SUCCESS if the call succeeds.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::set_serialMode(const string& newval)
{
    string rest_val;
    rest_val = newval;
    return _setAttr("serialMode", rest_val);
}

/**
 * Returns the type of protocol used over the serial line, as a string.
 * Possible values are "Line" for ASCII messages separated by CR and/or LF,
 * "Frame:[timeout]ms" for binary messages separated by a delay time,
 * "Modbus-ASCII" for MODBUS messages in ASCII mode,
 * "Modbus-RTU" for MODBUS messages in RTU mode,
 * "Char" for a continuous ASCII stream or
 * "Byte" for a continuous binary stream.
 * 
 * @return a string corresponding to the type of protocol used over the serial line, as a string
 * 
 * On failure, throws an exception or returns Y_PROTOCOL_INVALID.
 */
string YSerialPort::get_protocol(void)
{
    if (_cacheExpiration <= YAPI::GetTickCount()) {
        if (this->load(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
            return YSerialPort::PROTOCOL_INVALID;
        }
    }
    return _protocol;
}

/**
 * Changes the type of protocol used over the serial line.
 * Possible values are "Line" for ASCII messages separated by CR and/or LF,
 * "Frame:[timeout]ms" for binary messages separated by a delay time,
 * "Modbus-ASCII" for MODBUS messages in ASCII mode,
 * "Modbus-RTU" for MODBUS messages in RTU mode,
 * "Char" for a continuous ASCII stream or
 * "Byte" for a continuous binary stream.
 * 
 * @param newval : a string corresponding to the type of protocol used over the serial line
 * 
 * @return YAPI_SUCCESS if the call succeeds.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::set_protocol(const string& newval)
{
    string rest_val;
    rest_val = newval;
    return _setAttr("protocol", rest_val);
}

/**
 * Returns the total number of bytes received since last reset.
 * 
 * @return an integer corresponding to the total number of bytes received since last reset
 * 
 * On failure, throws an exception or returns Y_RXCOUNT_INVALID.
 */
int YSerialPort::get_rxCount(void)
{
    if (_cacheExpiration <= YAPI::GetTickCount()) {
        if (this->load(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
            return YSerialPort::RXCOUNT_INVALID;
        }
    }
    return _rxCount;
}

/**
 * Returns the total number of bytes transmitted since last reset.
 * 
 * @return an integer corresponding to the total number of bytes transmitted since last reset
 * 
 * On failure, throws an exception or returns Y_TXCOUNT_INVALID.
 */
int YSerialPort::get_txCount(void)
{
    if (_cacheExpiration <= YAPI::GetTickCount()) {
        if (this->load(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
            return YSerialPort::TXCOUNT_INVALID;
        }
    }
    return _txCount;
}

/**
 * Returns the total number of communication errors detected since last reset.
 * 
 * @return an integer corresponding to the total number of communication errors detected since last reset
 * 
 * On failure, throws an exception or returns Y_ERRCOUNT_INVALID.
 */
int YSerialPort::get_errCount(void)
{
    if (_cacheExpiration <= YAPI::GetTickCount()) {
        if (this->load(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
            return YSerialPort::ERRCOUNT_INVALID;
        }
    }
    return _errCount;
}

/**
 * Returns the total number of messages received since last reset.
 * 
 * @return an integer corresponding to the total number of messages received since last reset
 * 
 * On failure, throws an exception or returns Y_MSGCOUNT_INVALID.
 */
int YSerialPort::get_msgCount(void)
{
    if (_cacheExpiration <= YAPI::GetTickCount()) {
        if (this->load(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
            return YSerialPort::MSGCOUNT_INVALID;
        }
    }
    return _msgCount;
}

/**
 * Returns the latest message fully received (for Line, Frame and Modbus protocols).
 * 
 * @return a string corresponding to the latest message fully received (for Line, Frame and Modbus protocols)
 * 
 * On failure, throws an exception or returns Y_LASTMSG_INVALID.
 */
string YSerialPort::get_lastMsg(void)
{
    if (_cacheExpiration <= YAPI::GetTickCount()) {
        if (this->load(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
            return YSerialPort::LASTMSG_INVALID;
        }
    }
    return _lastMsg;
}

string YSerialPort::get_command(void)
{
    if (_cacheExpiration <= YAPI::GetTickCount()) {
        if (this->load(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
            return YSerialPort::COMMAND_INVALID;
        }
    }
    return _command;
}

int YSerialPort::set_command(const string& newval)
{
    string rest_val;
    rest_val = newval;
    return _setAttr("command", rest_val);
}

/**
 * Retrieves $AFUNCTION$ for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 * 
 * This function does not require that $THEFUNCTION$ is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YSerialPort.isOnline() to test if $THEFUNCTION$ is
 * indeed online at a given time. In case of ambiguity when looking for
 * $AFUNCTION$ by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 * 
 * @param func : a string that uniquely characterizes $THEFUNCTION$
 * 
 * @return a YSerialPort object allowing you to drive $THEFUNCTION$.
 */
YSerialPort* YSerialPort::FindSerialPort(string func)
{
    YSerialPort* obj = NULL;
    obj = (YSerialPort*) YFunction::_FindFromCache("SerialPort", func);
    if (obj == NULL) {
        obj = new YSerialPort(func);
        YFunction::_AddToCache("SerialPort", func, obj);
    }
    return obj;
}

/**
 * Registers the callback function that is invoked on every change of advertised value.
 * The callback is invoked only during the execution of ySleep or yHandleEvents.
 * This provides control over the time when the callback is triggered. For good responsiveness, remember to call
 * one of these two functions periodically. To unregister a callback, pass a null pointer as argument.
 * 
 * @param callback : the callback function to call, or a null pointer. The callback function should take two
 *         arguments: the function object of which the value has changed, and the character string describing
 *         the new advertised value.
 * @noreturn
 */
int YSerialPort::registerValueCallback(YSerialPortValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackSerialPort = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YSerialPort::_invokeValueCallback(string value)
{
    if (_valueCallbackSerialPort != NULL) {
        _valueCallbackSerialPort(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

int YSerialPort::sendCommand(string text)
{
    return this->set_command(text);
}

/**
 * Clears the serial port buffer and resets counters to zero.
 * 
 * @return YAPI_SUCCESS if the call succeeds.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::reset(void)
{
    _rxptr = 0;
    // may throw an exception
    return this->sendCommand("Z");
}

/**
 * Manually sets the state of the RTS line. This function has no effect when
 * hardware handshake is enabled, as the RTS line is driven automatically.
 * 
 * @param val : 1 to turn RTS on, 0 to turn RTS off
 * 
 * @return YAPI_SUCCESS if the call succeeds.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::set_RTS(int val)
{
    return this->sendCommand(YapiWrapper::ysprintf("R%d",val));
}

/**
 * Read the level of the CTS line. The CTS line is usually driven by
 * the RTS signal of the connected serial device.
 * 
 * @return 1 if the CTS line is high, 0 if the CTS line is low.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::get_CTS(void)
{
    string buff;
    int res = 0;
    
    // may throw an exception
    buff = this->_download("cts.txt");
    if (!((int)(buff).size() == 1)) {
        _throw(YAPI_IO_ERROR,"invalid CTS reply");
        return YAPI_IO_ERROR;
    }
    res = buff[0] - 48;
    return res;
}

/**
 * Sends an ASCII string to the serial port, as is.
 * 
 * @param text : the text string to send
 * 
 * @return YAPI_SUCCESS if the call succeeds.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::writeStr(string text)
{
    string buff;
    int bufflen = 0;
    int idx = 0;
    int ch = 0;
    
    buff = text;
    bufflen = (int)(buff).size();
    if (bufflen < 100) {
        ch = 0x20;
        idx = 0;
        while ((idx < bufflen) && (ch != 0)) {
            ch = buff[idx];
            if ((ch >= 0x20) && (ch < 0x7f)) {
                idx = idx + 1;
            } else {
                ch = 0;
            }
        }
        if (idx >= bufflen) {
            return this->sendCommand(YapiWrapper::ysprintf("+%s",text.c_str()));
        }
    }
    // send string using file upload
    return this->_upload("txdata", buff);
}

/**
 * Sends a binary buffer to the serial port, as is.
 * 
 * @param buff : the binary buffer to send
 * 
 * @return YAPI_SUCCESS if the call succeeds.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::writeBin(string buff)
{
    return this->_upload("txdata", buff);
}

/**
 * Sends a byte sequence (provided as a list of bytes) to the serial port.
 * 
 * @param byteList : a list of byte codes
 * 
 * @return YAPI_SUCCESS if the call succeeds.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::writeArray(vector<int> byteList)
{
    string buff;
    int bufflen = 0;
    int idx = 0;
    int hexb = 0;
    int res = 0;
    bufflen = (int)byteList.size();
    buff = string(bufflen, (char)0);
    idx = 0;
    while (idx < bufflen) {
        hexb = byteList[idx];
        buff[idx] = hexb;
        idx = idx + 1;
    }
    // may throw an exception
    res = this->_upload("txdata", buff);
    return res;
}

/**
 * Sends a byte sequence (provided as a hexadecimal string) to the serial port.
 * 
 * @param hexString : a string of hexadecimal byte codes
 * 
 * @return YAPI_SUCCESS if the call succeeds.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::writeHex(string hexString)
{
    string buff;
    int bufflen = 0;
    int idx = 0;
    int hexb = 0;
    int res = 0;
    bufflen = (int)(hexString).length();
    if (bufflen < 100) {
        return this->sendCommand(YapiWrapper::ysprintf("$%s",hexString.c_str()));
    }
    bufflen = ((bufflen) >> (1));
    buff = string(bufflen, (char)0);
    idx = 0;
    while (idx < bufflen) {
        hexb = (int)strtoul((hexString).substr( 2 * idx, 2).c_str(), NULL, 16);
        buff[idx] = hexb;
        idx = idx + 1;
    }
    // may throw an exception
    res = this->_upload("txdata", buff);
    return res;
}

/**
 * Sends an ASCII string to the serial port, followed by a line break (CR LF).
 * 
 * @param text : the text string to send
 * 
 * @return YAPI_SUCCESS if the call succeeds.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::writeLine(string text)
{
    string buff;
    int bufflen = 0;
    int idx = 0;
    int ch = 0;
    
    buff = YapiWrapper::ysprintf("%s\r\n",text.c_str());
    bufflen = (int)(buff).size()-2;
    if (bufflen < 100) {
        ch = 0x20;
        idx = 0;
        while ((idx < bufflen) && (ch != 0)) {
            ch = buff[idx];
            if ((ch >= 0x20) && (ch < 0x7f)) {
                idx = idx + 1;
            } else {
                ch = 0;
            }
        }
        if (idx >= bufflen) {
            return this->sendCommand(YapiWrapper::ysprintf("!%s",text.c_str()));
        }
    }
    // send string using file upload
    return this->_upload("txdata", buff);
}

/**
 * Sends a MODBUS message (provided as a hexadecimal string) to the serial port.
 * The message must start with the slave address. The MODBUS CRC/LRC is
 * automatically added by the function. This function does not wait for a reply.
 * 
 * @param hexString : a hexadecimal message string, including device address but no CRC/LRC
 * 
 * @return YAPI_SUCCESS if the call succeeds.
 * 
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::writeMODBUS(string hexString)
{
    return this->sendCommand(YapiWrapper::ysprintf(":%s",hexString.c_str()));
}

/**
 * Reads data from the receive buffer as a string, starting at current stream position.
 * If data at current stream position is not available anymore in the receive buffer, the
 * function performs a short read.
 * 
 * @param nChars : the maximum number of characters to read
 * 
 * @return a string with receive buffer contents
 * 
 * On failure, throws an exception or returns a negative error code.
 */
string YSerialPort::readStr(int nChars)
{
    string buff;
    int bufflen = 0;
    int mult = 0;
    int endpos = 0;
    int startpos = 0;
    int missing = 0;
    string res;
    if (nChars > 65535) {
        nChars = 65535;
    }
    // may throw an exception
    buff = this->_download(YapiWrapper::ysprintf("rxdata.bin?pos=%d&len=%d", _rxptr,nChars));
    bufflen = (int)(buff).size() - 1;
    endpos = 0;
    mult = 1;
    while ((bufflen > 0) && (buff[bufflen] != 64)) {
        endpos = endpos + mult * (buff[bufflen] - 48);
        mult = mult * 10;
        bufflen = bufflen - 1;
    }
    startpos = ((endpos - bufflen) & (0x7fffffff));
    if (startpos != _rxptr) {
        missing = ((startpos - _rxptr) & (0x7fffffff));
        if (missing > nChars) {
            nChars = 0;
            _rxptr = startpos;
        } else {
            nChars = nChars - missing;
        }
    }
    if (nChars > bufflen) {
        nChars = bufflen;
    }
    _rxptr = _rxptr + nChars;
    res = (buff).substr( 0, nChars);
    return res;
}

/**
 * Reads data from the receive buffer as a hexadecimal string, starting at current stream position.
 * If data at current stream position is not available anymore in the receive buffer, the
 * function performs a short read.
 * 
 * @param nBytes : the maximum number of bytes to read
 * 
 * @return a string with receive buffer contents, encoded in hexadecimal
 * 
 * On failure, throws an exception or returns a negative error code.
 */
string YSerialPort::readHex(int nBytes)
{
    string buff;
    int bufflen = 0;
    int mult = 0;
    int endpos = 0;
    int startpos = 0;
    int missing = 0;
    int ofs = 0;
    string res;
    if (nBytes > 65535) {
        nBytes = 65535;
    }
    // may throw an exception
    buff = this->_download(YapiWrapper::ysprintf("rxdata.bin?pos=%d&len=%d", _rxptr,nBytes));
    bufflen = (int)(buff).size()-1;
    endpos = 0;
    mult = 1;
    while ((bufflen > 0) && (buff[bufflen] != 64)) {
        endpos = endpos + mult * (buff[bufflen] - 48);
        mult = mult * 10;
        bufflen = bufflen - 1;
    }
    startpos = ((endpos - bufflen) & (0x7fffffff));
    if (startpos != _rxptr) {
        missing = ((startpos - _rxptr) & (0x7fffffff));
        if (missing > nBytes) {
            nBytes = 0;
            _rxptr = startpos;
        } else {
            nBytes = nBytes - missing;
        }
    }
    if (nBytes > bufflen) {
        nBytes = bufflen;
    }
    _rxptr = _rxptr + nBytes;
    res = "";
    ofs = 0;
    while (ofs+3 < nBytes) {
        res = YapiWrapper::ysprintf("%s%02x%02x%02x%02x", res.c_str(), buff[ofs], buff[ofs+1], buff[ofs+2],buff[ofs+3]);
        ofs = ofs + 4;
    }
    while (ofs < nBytes) {
        res = YapiWrapper::ysprintf("%s%02x", res.c_str(),buff[ofs]);
        ofs = ofs + 1;
    }
    return res;
}

/**
 * Reads a single line (or message) from the receive buffer, starting at current stream position.
 * This function can only be used when the serial port is configured for a message protocol,
 * such as 'Line' mode or MODBUS protocols. It does not  work in plain stream modes, eg. 'Char' or 'Byte').
 * 
 * If data at current stream position is not available anymore in the receive buffer,
 * the function returns the oldest available line and moves the stream position just after.
 * If no new full line is received, the function returns an empty line.
 * 
 * @return a string with a single line of text
 * 
 * On failure, throws an exception or returns a negative error code.
 */
string YSerialPort::readLine(void)
{
    string url;
    string msgbin;
    vector<string> msgarr;
    int msglen = 0;
    string res;
    // may throw an exception
    url = YapiWrapper::ysprintf("rxmsg.json?pos=%d&len=1&maxw=1",_rxptr);
    msgbin = this->_download(url);
    msgarr = this->_json_get_array(msgbin);
    msglen = (int)msgarr.size();
    if (msglen == 0) {
        return "";
    }
    // last element of array is the new position
    msglen = msglen - 1;
    _rxptr = atoi((msgarr[msglen]).c_str());
    if (msglen == 0) {
        return "";
    }
    res = this->_json_get_string(msgarr[0]);
    return res;
}

/**
 * Searches for incoming messages in the serial port receive buffer matching a given pattern,
 * starting at current position. This function can only be used when the serial port is
 * configured for a message protocol, such as 'Line' mode or MODBUS protocols.
 * It does not work in plain stream modes, eg. 'Char' or 'Byte', for which there is no "start" of message.
 * 
 * The search returns all messages matching the expression provided as argument in the buffer.
 * If no matching message is found, the search waits for one up to the specified maximum timeout
 * (in milliseconds).
 * 
 * @param pattern : a limited regular expression describing the expected message format,
 *         or an empty string if all messages should be returned (no filtering).
 *         When using binary protocols, the format applies to the hexadecimal
 *         representation of the message.
 * @param maxWait : the maximum number of milliseconds to wait for a message if none is found
 *         in the receive buffer.
 * 
 * @return an array of strings containing the messages found, if any.
 *         Binary messages are converted to hexadecimal representation.
 * 
 * On failure, throws an exception or returns an empty array.
 */
vector<string> YSerialPort::readMessages(string pattern,int maxWait)
{
    string url;
    string msgbin;
    vector<string> msgarr;
    int msglen = 0;
    vector<string> res;
    int idx = 0;
    // may throw an exception
    url = YapiWrapper::ysprintf("rxmsg.json?pos=%d&maxw=%d&pat=%s", _rxptr, maxWait,pattern.c_str());
    msgbin = this->_download(url);
    msgarr = this->_json_get_array(msgbin);
    msglen = (int)msgarr.size();
    if (msglen == 0) {
        return res;
    }
    // last element of array is the new position
    msglen = msglen - 1;
    _rxptr = atoi((msgarr[msglen]).c_str());
    idx = 0;
    while (idx < msglen) {
        res.push_back(this->_json_get_string(msgarr[idx]));
        idx = idx + 1;
    }
    return res;
}

/**
 * Changes the current internal stream position to the specified value. This function
 * does not affect the device, it only changes the value stored in the YSerialPort object
 * for the next read operations.
 * 
 * @param rxCountVal : the absolute position index (value of rxCount) for next read operations.
 * 
 * @return nothing.
 */
int YSerialPort::read_seek(int rxCountVal)
{
    _rxptr = rxCountVal;
    return YAPI_SUCCESS;
}

/**
 * Sends a text line query to the serial port, and reads the reply, if any.
 * This function can only be used when the serial port is configured for 'Line' protocol.
 * 
 * @param query : the line query to send (without CR/LF)
 * @param maxWait : the maximum number of milliseconds to wait for a reply.
 * 
 * @return the next text line received after sending the text query, as a string.
 *         Additional lines can be obtained by calling readLine or readMessages.
 * 
 * On failure, throws an exception or returns an empty array.
 */
string YSerialPort::queryLine(string query,int maxWait)
{
    string url;
    string msgbin;
    vector<string> msgarr;
    int msglen = 0;
    string res;
    // may throw an exception
    url = YapiWrapper::ysprintf("rxmsg.json?len=1&maxw=%d&cmd=!%s", maxWait,query.c_str());
    msgbin = this->_download(url);
    msgarr = this->_json_get_array(msgbin);
    msglen = (int)msgarr.size();
    if (msglen == 0) {
        return "";
    }
    // last element of array is the new position
    msglen = msglen - 1;
    _rxptr = atoi((msgarr[msglen]).c_str());
    if (msglen == 0) {
        return "";
    }
    res = this->_json_get_string(msgarr[0]);
    return res;
}

/**
 * Sends a message to a specified MODBUS slave connected to the serial port, and reads the
 * reply, if any. The message is the PDU, provided as a vector of bytes.
 * 
 * @param slaveNo : the address of the slave MODBUS device to query
 * @param pduBytes : the message to send (PDU), as a vector of bytes. The first byte of the
 *         PDU is the MODBUS function code.
 * 
 * @return the received reply, as a vector of bytes.
 * 
 * On failure, throws an exception or returns an empty array (or a MODBUS error reply).
 */
vector<int> YSerialPort::queryMODBUS(int slaveNo,vector<int> pduBytes)
{
    int funCode = 0;
    int nib = 0;
    int i = 0;
    string cmd;
    string url;
    string pat;
    string msgs;
    vector<string> reps;
    string rep;
    vector<int> res;
    int replen = 0;
    int hexb = 0;
    funCode = pduBytes[0];
    nib = ((funCode) >> (4));
    pat = YapiWrapper::ysprintf("%02x[%x%x]%x.*", slaveNo, nib, (nib+8),((funCode) & (15)));
    cmd = YapiWrapper::ysprintf("%02x%02x", slaveNo,funCode);
    i = 1;
    while (i < (int)pduBytes.size()) {
        cmd = YapiWrapper::ysprintf("%s%02x", cmd.c_str(),((pduBytes[i]) & (0xff)));
        i = i + 1;
    }
    // may throw an exception
    url = YapiWrapper::ysprintf("rxmsg.json?cmd=:%s&pat=:%s", cmd.c_str(),pat.c_str());
    msgs = this->_download(url);
    reps = this->_json_get_array(msgs);
    if (!((int)reps.size() > 1)) {
        _throw(YAPI_IO_ERROR,"no reply from slave");
        return res;
    }
    if ((int)reps.size() > 1) {
        rep = this->_json_get_string(reps[0]);
        replen = (((int)(rep).length() - 3) >> (1));
        i = 0;
        while (i < replen) {
            hexb = (int)strtoul((rep).substr(2 * i + 3, 2).c_str(), NULL, 16);
            res.push_back(hexb);
            i = i + 1;
        }
        if (res[0] != funCode) {
            i = res[1];
            if (!(i > 1)) {
                _throw(YAPI_NOT_SUPPORTED,"MODBUS error: unsupported function code");
                return res;
            }
            if (!(i > 2)) {
                _throw(YAPI_INVALID_ARGUMENT,"MODBUS error: illegal data address");
                return res;
            }
            if (!(i > 3)) {
                _throw(YAPI_INVALID_ARGUMENT,"MODBUS error: illegal data value");
                return res;
            }
            if (!(i > 4)) {
                _throw(YAPI_INVALID_ARGUMENT,"MODBUS error: failed to execute function");
                return res;
            }
        }
    }
    return res;
}

/**
 * Reads one or more contiguous internal bits (or coil status) from a MODBUS serial device.
 * This method uses the MODBUS function code 0x01 (Read Coils).
 * 
 * @param slaveNo : the address of the slave MODBUS device to query
 * @param pduAddr : the relative address of the first bit/coil to read (zero-based)
 * @param nBits : the number of bits/coils to read
 * 
 * @return a vector of integers, each corresponding to one bit.
 * 
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YSerialPort::modbusReadBits(int slaveNo,int pduAddr,int nBits)
{
    vector<int> pdu;
    vector<int> reply;
    vector<int> res;
    int bitpos = 0;
    int idx = 0;
    int val = 0;
    int mask = 0;
    pdu.push_back(0x01);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nBits) >> (8)));
    pdu.push_back(((nBits) & (0xff)));
    // may throw an exception
    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    bitpos = 0;
    idx = 2;
    val = reply[idx];
    mask = 1;
    while (bitpos < nBits) {
        if (((val) & (mask)) == 0) {
            res.push_back(0);
        } else {
            res.push_back(1);
        }
        bitpos = bitpos + 1;
        if (mask == 0x80) {
            idx = idx + 1;
            val = reply[idx];
            mask = 1;
        } else {
            mask = ((mask) << (1));
        }
    }
    return res;
}

/**
 * Reads one or more contiguous input bits (or discrete inputs) from a MODBUS serial device.
 * This method uses the MODBUS function code 0x02 (Read Discrete Inputs).
 * 
 * @param slaveNo : the address of the slave MODBUS device to query
 * @param pduAddr : the relative address of the first bit/input to read (zero-based)
 * @param nBits : the number of bits/inputs to read
 * 
 * @return a vector of integers, each corresponding to one bit.
 * 
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YSerialPort::modbusReadInputBits(int slaveNo,int pduAddr,int nBits)
{
    vector<int> pdu;
    vector<int> reply;
    vector<int> res;
    int bitpos = 0;
    int idx = 0;
    int val = 0;
    int mask = 0;
    pdu.push_back(0x02);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nBits) >> (8)));
    pdu.push_back(((nBits) & (0xff)));
    // may throw an exception
    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    bitpos = 0;
    idx = 2;
    val = reply[idx];
    mask = 1;
    while (bitpos < nBits) {
        if (((val) & (mask)) == 0) {
            res.push_back(0);
        } else {
            res.push_back(1);
        }
        bitpos = bitpos + 1;
        if (mask == 0x80) {
            idx = idx + 1;
            val = reply[idx];
            mask = 1;
        } else {
            mask = ((mask) << (1));
        }
    }
    return res;
}

/**
 * Reads one or more contiguous internal registers (holding registers) from a MODBUS serial device.
 * This method uses the MODBUS function code 0x03 (Read Holding Registers).
 * 
 * @param slaveNo : the address of the slave MODBUS device to query
 * @param pduAddr : the relative address of the first holding register to read (zero-based)
 * @param nWords : the number of holding registers to read
 * 
 * @return a vector of integers, each corresponding to one 16-bit register value.
 * 
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YSerialPort::modbusReadRegisters(int slaveNo,int pduAddr,int nWords)
{
    vector<int> pdu;
    vector<int> reply;
    vector<int> res;
    int regpos = 0;
    int idx = 0;
    int val = 0;
    pdu.push_back(0x03);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nWords) >> (8)));
    pdu.push_back(((nWords) & (0xff)));
    // may throw an exception
    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    regpos = 0;
    idx = 2;
    while (regpos < nWords) {
        val = ((reply[idx]) << (8));
        idx = idx + 1;
        val = val + reply[idx];
        idx = idx + 1;
        res.push_back(val);
        regpos = regpos + 1;
    }
    return res;
}

/**
 * Reads one or more contiguous input registers (read-only registers) from a MODBUS serial device.
 * This method uses the MODBUS function code 0x04 (Read Input Registers).
 * 
 * @param slaveNo : the address of the slave MODBUS device to query
 * @param pduAddr : the relative address of the first input register to read (zero-based)
 * @param nWords : the number of input registers to read
 * 
 * @return a vector of integers, each corresponding to one 16-bit input value.
 * 
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YSerialPort::modbusReadInputRegisters(int slaveNo,int pduAddr,int nWords)
{
    vector<int> pdu;
    vector<int> reply;
    vector<int> res;
    int regpos = 0;
    int idx = 0;
    int val = 0;
    pdu.push_back(0x04);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nWords) >> (8)));
    pdu.push_back(((nWords) & (0xff)));
    // may throw an exception
    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    regpos = 0;
    idx = 2;
    while (regpos < nWords) {
        val = ((reply[idx]) << (8));
        idx = idx + 1;
        val = val + reply[idx];
        idx = idx + 1;
        res.push_back(val);
        regpos = regpos + 1;
    }
    return res;
}

/**
 * Sets a single internal bit (or coil) on a MODBUS serial device.
 * This method uses the MODBUS function code 0x05 (Write Single Coil).
 * 
 * @param slaveNo : the address of the slave MODBUS device to drive
 * @param pduAddr : the relative address of the bit/coil to set (zero-based)
 * @param value : the value to set (0 for OFF state, non-zero for ON state)
 * 
 * @return the number of bits/coils affected on the device (1)
 * 
 * On failure, throws an exception or returns zero.
 */
int YSerialPort::modbusWriteBit(int slaveNo,int pduAddr,int value)
{
    vector<int> pdu;
    vector<int> reply;
    int res = 0;
    res = 0;
    if (value != 0) {
        value = 0xff;
    }
    pdu.push_back(0x05);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(value);
    pdu.push_back(0x00);
    // may throw an exception
    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    res = 1;
    return res;
}

/**
 * Sets several contiguous internal bits (or coils) on a MODBUS serial device.
 * This method uses the MODBUS function code 0x0f (Write Multiple Coils).
 * 
 * @param slaveNo : the address of the slave MODBUS device to drive
 * @param pduAddr : the relative address of the first bit/coil to set (zero-based)
 * @param bits : the vector of bits to be set (one integer per bit)
 * 
 * @return the number of bits/coils affected on the device
 * 
 * On failure, throws an exception or returns zero.
 */
int YSerialPort::modbusWriteBits(int slaveNo,int pduAddr,vector<int> bits)
{
    int nBits = 0;
    int nBytes = 0;
    int bitpos = 0;
    int val = 0;
    int mask = 0;
    vector<int> pdu;
    vector<int> reply;
    int res = 0;
    res = 0;
    nBits = (int)bits.size();
    nBytes = (((nBits + 7)) >> (3));
    pdu.push_back(0x0f);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nBits) >> (8)));
    pdu.push_back(((nBits) & (0xff)));
    pdu.push_back(nBytes);
    bitpos = 0;
    val = 0;
    mask = 1;
    while (bitpos < nBits) {
        if (bits[bitpos] != 0) {
            val = ((val) | (mask));
        }
        bitpos = bitpos + 1;
        if (mask == 0x80) {
            pdu.push_back(val);
            val = 0;
            mask = 1;
        } else {
            mask = ((mask) << (1));
        }
    }
    if (mask != 1) {
        pdu.push_back(val);
    }
    // may throw an exception
    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    res = ((reply[3]) << (8));
    res = res + reply[4];
    return res;
}

/**
 * Sets a single internal register (or holding register) on a MODBUS serial device.
 * This method uses the MODBUS function code 0x06 (Write Single Register).
 * 
 * @param slaveNo : the address of the slave MODBUS device to drive
 * @param pduAddr : the relative address of the register to set (zero-based)
 * @param value : the 16 bit value to set
 * 
 * @return the number of registers affected on the device (1)
 * 
 * On failure, throws an exception or returns zero.
 */
int YSerialPort::modbusWriteRegister(int slaveNo,int pduAddr,int value)
{
    vector<int> pdu;
    vector<int> reply;
    int res = 0;
    res = 0;
    if (value != 0) {
        value = 0xff;
    }
    pdu.push_back(0x06);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((value) >> (8)));
    pdu.push_back(((value) & (0xff)));
    // may throw an exception
    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    res = 1;
    return res;
}

/**
 * Sets several contiguous internal registers (or holding registers) on a MODBUS serial device.
 * This method uses the MODBUS function code 0x10 (Write Multiple Registers).
 * 
 * @param slaveNo : the address of the slave MODBUS device to drive
 * @param pduAddr : the relative address of the first internal register to set (zero-based)
 * @param values : the vector of 16 bit values to set
 * 
 * @return the number of registers affected on the device
 * 
 * On failure, throws an exception or returns zero.
 */
int YSerialPort::modbusWriteRegisters(int slaveNo,int pduAddr,vector<int> values)
{
    int nWords = 0;
    int nBytes = 0;
    int regpos = 0;
    int val = 0;
    vector<int> pdu;
    vector<int> reply;
    int res = 0;
    res = 0;
    nWords = (int)values.size();
    nBytes = 2 * nWords;
    pdu.push_back(0x10);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nWords) >> (8)));
    pdu.push_back(((nWords) & (0xff)));
    pdu.push_back(nBytes);
    regpos = 0;
    while (regpos < nWords) {
        val = values[regpos];
        pdu.push_back(((val) >> (8)));
        pdu.push_back(((val) & (0xff)));
        regpos = regpos + 1;
    }
    // may throw an exception
    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    res = ((reply[3]) << (8));
    res = res + reply[4];
    return res;
}

/**
 * Sets several contiguous internal registers (holding registers) on a MODBUS serial device,
 * then performs a contiguous read of a set of (possibly different) internal registers.
 * This method uses the MODBUS function code 0x17 (Read/Write Multiple Registers).
 * 
 * @param slaveNo : the address of the slave MODBUS device to drive
 * @param pduWriteAddr : the relative address of the first internal register to set (zero-based)
 * @param values : the vector of 16 bit values to set
 * @param pduReadAddr : the relative address of the first internal register to read (zero-based)
 * @param nReadWords : the number of 16 bit values to read
 * 
 * @return a vector of integers, each corresponding to one 16-bit register value read.
 * 
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YSerialPort::modbusWriteAndReadRegisters(int slaveNo,int pduWriteAddr,vector<int> values,int pduReadAddr,int nReadWords)
{
    int nWriteWords = 0;
    int nBytes = 0;
    int regpos = 0;
    int val = 0;
    int idx = 0;
    vector<int> pdu;
    vector<int> reply;
    vector<int> res;
    nWriteWords = (int)values.size();
    nBytes = 2 * nWriteWords;
    pdu.push_back(0x17);
    pdu.push_back(((pduReadAddr) >> (8)));
    pdu.push_back(((pduReadAddr) & (0xff)));
    pdu.push_back(((nReadWords) >> (8)));
    pdu.push_back(((nReadWords) & (0xff)));
    pdu.push_back(((pduWriteAddr) >> (8)));
    pdu.push_back(((pduWriteAddr) & (0xff)));
    pdu.push_back(((nWriteWords) >> (8)));
    pdu.push_back(((nWriteWords) & (0xff)));
    pdu.push_back(nBytes);
    regpos = 0;
    while (regpos < nWriteWords) {
        val = values[regpos];
        pdu.push_back(((val) >> (8)));
        pdu.push_back(((val) & (0xff)));
        regpos = regpos + 1;
    }
    // may throw an exception
    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    regpos = 0;
    idx = 2;
    while (regpos < nReadWords) {
        val = ((reply[idx]) << (8));
        idx = idx + 1;
        val = val + reply[idx];
        idx = idx + 1;
        res.push_back(val);
        regpos = regpos + 1;
    }
    return res;
}

YSerialPort *YSerialPort::nextSerialPort(void)
{
    string  hwid;
    
    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YSerialPort::FindSerialPort(hwid);
}

YSerialPort* YSerialPort::FirstSerialPort(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;
    
    if(YISERR(YapiWrapper::getFunctionsByClass("SerialPort", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YSerialPort::FindSerialPort(serial+"."+funcId);
}

//--- (end of YSerialPort implementation)

//--- (SerialPort functions)
//--- (end of SerialPort functions)
