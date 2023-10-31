//#include "Ab_LCD.h"
//
//void SerialLCD::page0() {
//  sendData(check4G, sizeof(check4G));
//  sendData(checkSensors, sizeof(checkSensors));
//  sendData(checkSever, sizeof(checkSever));
//  sendData(checkRFID, sizeof(checkRFID));
//  sendData(chekTemp, sizeof(chekTemp));
//  sendData(checkGPS, sizeof(checkGPS));
//
//}
//
//void SerialLCD::page1() {
//  const int delayTime = 300;
//
//  if (current.is4G) {
//    changeLastHexValue(check4G, sizeof(check4G), goodValue);
//    sendData(check4G, sizeof(check4G));
//    delay(delayTime);
//  }
//
//  if (current.isSensor) {
//    changeLastHexValue(checkSensors, sizeof(checkSensors), goodValue);
//    sendData(checkSensors, sizeof(checkSensors));
//    delay(delayTime);
//  }
//
//  if (current.isServer) {
//    changeLastHexValue(checkSever, sizeof(checkSever), goodValue);
//    sendData(checkSever, sizeof(checkSever));
//    delay(delayTime);
//  }
//
//  if (current.isRFID) {
//    changeLastHexValue(checkRFID, sizeof(checkRFID), goodValue);
//    sendData(checkRFID, sizeof(checkRFID));
//    delay(delayTime);
//  }
//
//  if (current.isTemp) {
//    changeLastHexValue(chekTemp, sizeof(chekTemp), goodValue);
//    sendData(chekTemp, sizeof(chekTemp));
//    delay(delayTime);
//  }
//
//  if (current.isGPS) {
//    changeLastHexValue(checkGPS, sizeof(checkGPS), goodValue);
//    sendData(checkGPS, sizeof(checkGPS));
//    delay(delayTime);
//  }
//
//  delay(2000);
//  selectPage(2);
//}
//
//void SerialLCD::page2() {
//  if (!readRFID) {
//    return;
//  }
//
//  long now = millis();
//
//  if (startTime == 0) {
//    startTime = now;
//  }
//
//  if (current.Username.isEmpty()) {
//    if (!popupSent) {
//      sendData(showPopUp, sizeof(showPopUp));
//      popupSent = true;
//    }
//
//    if (now - startTime > 3000) {
//      startTime = 0;
//      readRFID = false;
//
//      if (popupSent) {
//        sendData(HidePopUp, sizeof(HidePopUp));
//        popupSent = false;      }
//    }
//  } else {
//    startTime = 0;
//    readRFID = false;
//
//    if (popupSent) {
//      sendData(HidePopUp, sizeof(HidePopUp));
//      popupSent = false;
//    }
//
//    clearLCDObject(maxContentLength, h_setJobNumber, h_Length);
//    clearLCDObject(maxContentLength, h_setUser, h_Length);
//    selectPage(3);
//  }
//}
//
//void SerialLCD::page3() {
//  if (current.Username != previous.Username) {
//    clearLCDObject(maxContentLength, h_setUser, h_Length);
//    updateLCDObject(String(current.Username), h_setUser, h_Length);
//    previous.Username = current.Username;
//  }
//
//  if (current.maxJob > 0) {
//    prepareDataPage4();
//    selectPage(4);
//    return;  // Exit the function after selecting page 4
//  }
//
//  listentoLCD();
//
//  if (!isEmpty(receivedData, sizeof(receivedData))) {
//    if (memcmp(receivedData, b_logOut0, sizeof(b_logOut0)) == 0) {
//      selectPage(2);
//      clearLogOutData();
//      clearReceivedData();
//      return;  // Exit the function after returning to page 2
//    }
//  }
//}
//
//void SerialLCD::page4() {
//  if (current.maxJob > 0) {
//    prepareDataPage4();
//
//    long now = millis();
//    if (startTime == 0) {
//      startTime = now;
//    }
//
//    if (now - startTime > 3000 && startTime != 0) {
//      startTime = 0;
//      sendData(speakerBeep, sizeof(speakerBeep));
//    }
//  } else {
//    startTime = 0;
//    selectPage(3);
//    return;
//  }
//
//  listentoLCD();
//
//  if (!isEmpty(receivedData, sizeof(receivedData))) {
//    if (memcmp(receivedData, b_logOut1, sizeof(b_logOut1)) == 0) {
//      startTime = 0;
//      selectPage(2);
//      clearLogOutData();
//      clearReceivedData();
//      return;
//    }
//
//    if (memcmp(receivedData, b_job, sizeof(b_job)) == 0) {
//      startTime = 0;
//      prepareDataPage6();
//      selectPage(6);
//      clearReceivedData();
//      return;
//    }
//  }
//}
//
//void SerialLCD::page6() {
//  prepareDataPage6();
//
//  listentoLCD();
//
//  if (!isEmpty(receivedData, sizeof(receivedData))) {
//    if (memcmp(receivedData, b_logOut2, sizeof(b_logOut2)) == 0) {
//      selectPage(2);
//      clearLogOutData();
//      clearReceivedData();
//      return;
//    }
//
//    if (memcmp(receivedData, b_left, sizeof(b_left)) == 0) {
//      clearReceivedData();
//    }
//
//    if (memcmp(receivedData, b_right, sizeof(b_right)) == 0) {
//      clearReceivedData();
//    }
//
//    if (memcmp(receivedData, b_yes, sizeof(b_yes)) == 0) {
//      selectPage(8);
//      clearReceivedData();
//      return;
//    }
//
//    if (memcmp(receivedData, b_no, sizeof(b_no)) == 0) {
//      selectPage(4);
//      clearReceivedData();
//      return;
//    }
//  }
//}
//
//void SerialLCD::page8() {
//  listentoLCD();
//
//  if (!isEmpty(receivedData, sizeof(receivedData))) {
//    if (memcmp(receivedData, b_logOut3, sizeof(b_logOut3)) == 0) {
//      selectPage(2);
//      clearLogOutData();
//      clearReceivedData();
//      return;
//    }
//
//    if (memcmp(receivedData, b_finish1, sizeof(b_finish1)) == 0) {
//      selectPage(11);
//      clearReceivedData();
//      return;
//    }
//
//    if (memcmp(receivedData, b_cancel1, sizeof(b_cancel1)) == 0) {
//      selectPage(6);
//      clearReceivedData();
//      return;
//    }
//  }
//}
//
//void SerialLCD::page11() {
//  listentoLCD();
//
//  if (!isEmpty(receivedData, sizeof(receivedData))) {
//    if (memcmp(receivedData, b_confrim, sizeof(b_confrim)) == 0) {
//      selectPage(8);
//      clearReceivedData();
//      return;
//    }
//
//    if (memcmp(receivedData, b_cancel2, sizeof(b_cancel2)) == 0) {
//      selectPage(6);
//      clearReceivedData();
//      return;
//    }
//  }
//}
//
//void SerialLCD::sendData(byte data[], size_t dataSize) {
//  Serial2.write(data, dataSize);
//}
//
//void addDataToByteArray(std::vector<byte>& byteArray, const String& data) {
//  for (size_t i = 0; i < data.length(); i++) {
//    byte hexValue = static_cast<byte>(data.charAt(i));
//    byteArray.push_back(hexValue);
//  }
//}
//
//bool SerialLCD::checkReceivedData(byte receivedData[], size_t expectedDataSize) {
//  if (Serial2.available() >= expectedDataSize) {
//    byte header[2] = {0xA5, 0x5A};
//    bool headerFound = false;
//    size_t bytesRead = 0;
//
//    while (Serial2.available() && !headerFound) {
//      byte nextByte = Serial2.read();
//      if (nextByte == header[bytesRead]) {
//        bytesRead++;
//        if (bytesRead == 2) {
//          headerFound = true;
//        }
//      } else {
//        bytesRead = 0;
//      }
//    }
//
//    if (headerFound) {
//      size_t bytesToRead = expectedDataSize - 2;
//      if (Serial2.available() >= bytesToRead) {
//        Serial2.readBytes(&receivedData[2], bytesToRead);
//        receivedData[0] = 0xA5;
//        receivedData[1] = 0x5A;
//
//        //Serial.print("Received data: ");
//        for (size_t i = 0; i < expectedDataSize; i++) {
//          //Serial.print(receivedData[i], HEX);
//          //Serial.print(" ");
//        }
//        //Serial.println();
//
//        return true;
//      }
//    }
//  }
//  return false;
//}
//
//bool SerialLCD::checkPage(byte receivedData[]) {
//  byte expectedData[] = { 0xA5, 0x5A, 0x05, 0x81, 0x03, 0x02, 0x00 };
//  if (memcmp(receivedData, expectedData, sizeof(expectedData)) != 0) {
//    return false;
//  }
//
//  byte page = receivedData[sizeof(expectedData)];
//  switch (page) {
//    case 0:
//      return true;
//    case 1:
//      return true;
//    case 2:
//      return true;
//    default:
//      return false;
//  }
//}
//
//void SerialLCD::listentoLCD() {
//
//  if (checkReceivedData(receivedData, sizeof(receivedData))) {
//
//  }
//}
//void SerialLCD::selectPage(int page) {
//  pageSelector[6] = static_cast<byte>(page);
//  //Serial.println(pageSelector[6]);
//  sendData(pageSelector, sizeof(pageSelector));
//}
//void SerialLCD::changeLastHexValue(byte array[], size_t arraySize, byte lastValue) {
//  if (arraySize > 0) {
//    array[arraySize - 1] = lastValue;
//  }
//}
//bool SerialLCD::isEmpty(byte data[], size_t size) {
//  for (size_t i = 0; i < size; i++) {
//    if (data[i] != 0) {
//      return false;
//    }
//  }
//  return true;
//}
//void SerialLCD::clearReceivedData() {
//  memset(receivedData, 0, sizeof(receivedData));
//}
//
//byte* SerialLCD::stringToByteArray(const String& str, size_t arraySize) {
//
//  byte* byteArray = new byte[arraySize];
//
//  for (size_t i = 0; i < arraySize; i++) {
//    byteArray[i] = static_cast<byte>(str.charAt(i));
//  }
//
//  return byteArray;
//}
//
//byte* SerialLCD::combineArrays(byte* byteArray, size_t byteArraySize, byte staticArray[], size_t staticArraySize) {
//  size_t combinedSize = byteArraySize + staticArraySize;
//
//  byte* combinedArray = new byte[combinedSize];
//  for (size_t i = 0; i < staticArraySize; i++) {
//    combinedArray[i] = staticArray[i];
//    //Serial.print("code");
//    //Serial.println(combinedArray[i], HEX);
//
//  }
//  for (size_t i = 0; i < byteArraySize; i++) {
//    combinedArray[staticArraySize + i] = byteArray[i];
//    //Serial.print("value");
//    //Serial.println(combinedArray[staticArraySize + i], HEX);
//    //Serial.print("end");
//  }
//
//
//  return combinedArray;
//}
//
//void SerialLCD::updateLCDObject(String str, byte code[], size_t size) {
//  size_t arraySize =  str.length();
//  byte* byteArray = stringToByteArray(str, arraySize);
//  //Serial.print("size ");
//  //Serial.println(size);
//  byte* combinedArray = combineArrays(byteArray, arraySize, code, size);
//  combinedArray[2] = (byte)(size + arraySize - 3);
//  sendData(combinedArray, size + arraySize);
//  for (size_t i = 0; i <  size + arraySize; i++) {
//    //Serial.print(combinedArray[i], HEX);
//    //Serial.print(" ");
//  }
//  delete[] combinedArray;
//  delete[] byteArray;
//}
//void SerialLCD::clearLCDObject(int maxLength, byte code[], size_t size) {
//  size_t arraySize =  maxLength;
//  byte byteArray[arraySize];
//  memset(byteArray, 0, arraySize);
//  //Serial.print("maxLength ");
//  //Serial.println(arraySize);
//  byte* combinedArray = combineArrays(byteArray, arraySize, code, size);
//  combinedArray[2] = (byte)(size + arraySize - 3);
//  sendData(combinedArray, size + arraySize);
//  for (size_t i = 0; i <  size + arraySize; i++) {
//    //Serial.print(combinedArray[i], HEX);
//    //Serial.print(" ");
//  }
//  delete[] combinedArray;
//}
//void SerialLCD::clearLogOutData() {
//  current.Username = "";
//  current.Flight = "";
//  current.STD = "";
//  current.Bay = "";
//  current.ETD = "";
//  current.Gate = "";
//  current.maxJob = 0;
//  current.currentJob = 0;
//
//  previous.Username = "";
//  previous.Flight = "";
//  previous.STD = "";
//  previous.Bay = "";
//  previous.ETD = "";
//  previous.Gate = "";
//  previous.maxJob = 0;
//  previous.currentJob = 0;
//}
//void SerialLCD::prepareDataPage4() {
//  if (current.maxJob != previous.maxJob) {
//    clearLCDObject(maxContentLength, h_setJobNumber, h_Length);
//    updateLCDObject(String(current.maxJob), h_setJobNumber, h_Length);
//    previous.maxJob = current.maxJob;
//  }
//}
//void SerialLCD::prepareDataPage6() {
//  if (current.Username != previous.Username) {
//    //Serial.println(current.Username);
//    clearLCDObject(maxContentLength, h_setUser, h_Length);
//    updateLCDObject(String(current.Username), h_setUser, h_Length);
//    previous.Username = current.Username;
//  }
//  if (current.Flight != previous.Flight) {
//    //Serial.println(current.Flight);
//    clearLCDObject(maxContentLength, h_setFlight, h_Length);
//    updateLCDObject(String(current.Flight), h_setFlight, h_Length);
//    previous.Flight = current.Flight;
//  }
//  if (current.STD != previous.STD) {
//    //Serial.println(current.STD);
//    clearLCDObject(maxContentLength, h_setSTD, h_Length);
//    updateLCDObject(String(current.STD), h_setSTD, h_Length);
//    previous.STD = current.STD;
//  }
//  if (current.Bay != previous.Bay) {
//    //Serial.println(current.Bay);
//    clearLCDObject(maxContentLength, h_setBay, h_Length);
//    updateLCDObject(String(current.Bay), h_setBay, h_Length);
//    previous.Bay = current.Bay;
//  }
//  if (current.ETD != previous.ETD) {
//    //Serial.println(current.ETD);
//    clearLCDObject(maxContentLength, h_setETD, h_Length);
//    updateLCDObject(String(current.ETD), h_setETD, h_Length);
//    previous.ETD = current.ETD;
//  }
//  if (current.Gate != previous.Gate) {
//    //Serial.println(current.Gate);
//    clearLCDObject(maxContentLength, h_setGate, h_Length);
//    updateLCDObject(String(current.Gate), h_setGate, h_Length);
//    previous.Gate = current.Gate;
//  }
//  if (current.maxJob != previous.maxJob) {
//    current.Job = String(current.currentJob) + "/" + String(current.maxJob);
//    previous.maxJob = current.maxJob;
//  }
//  if (current.currentJob != previous.currentJob) {
//    current.Job = String(current.currentJob) + "/" + String(current.maxJob);
//    previous.currentJob = current.currentJob;
//  }
//  if (current.Job != previous.Job) {
//    //Serial.println(current.Job);
//    clearLCDObject(maxContentLength, h_setJob, h_Length);
//    updateLCDObject(String(current.Job), h_setJob, h_Length);
//    previous.Job = current.Job;
//  }
//}
