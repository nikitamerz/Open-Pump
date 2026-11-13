
//MQTT CALLBACK
//===============================================================
void callback(String topic, byte* message, unsigned int length) {
  //(byte*) message to (String)message_temp
  String message_temp;
  for (int i = 0; i < length; i++) {
    message_temp += (char)message[i];
  }
  response(topic, message_temp);
}
//===============================================================

//PUMP
//====================================================
void pump_resp(String slot, bool calib, float val ) {
  if (calib) {
    //Calibration mode
    if (val > 0) {
      PumpCalibCalc(slot, val);
    }
    else {
      out = "Pump " + slot +  " Activated";
      Serial.println(out);
      client.publish(TopicDEBUG, out.c_str());
      PumpCalibTurn(slot);
    }
  }
  else { //CMD mode
    out = "Pump " + slot + " CMD: " + val;
    Serial.println(out);
    client.publish(TopicDEBUG, out.c_str());
    PumpInject(slot, val);
  }
}
//====================================================

//SYR
//=======================================================
void syr_resp(String slot, bool calib , float val = 0) {
  if (calib) { //Calibration mode
    out = "Syringe " + slot +  " calibration";
    Serial.println(out);
    client.publish(TopicDEBUG, out.c_str());
    SyrHome(slot);
  }
  else { //CMD mode
    out = "Syringe " + slot + " CMD: " + val;
    Serial.println(out);
    client.publish(TopicDEBUG, out.c_str());
    SyrInject(slot, val);
    //(atof(index2.c_str()));
  }
}
//=======================================================

//Response to the command
//============================================================================================================
void response(String topic, String message_temp) {
  if (topic == TopicCMD) {
    String slot = message_temp.substring(0, 1);   //Index0 = X, Y or Z
    String calib = message_temp.substring(1, 2); //Index1 = C/H or (float)CMD
    String command = message_temp.substring(1); //Index from 1 = volume
    String calval = message_temp.substring(2); //Index from 2 = calibration volume
    String debugmqtt; //Debug response

    //Slot X selected
    //--------------------------------------------------------------------------------------------------------
    if (slot == "X") {

      //Tool = Syringe
      //..................................................................................
      if (ModulesInfos["X"] == "S") {
        (calib == "C") ?  syr_resp(slot, true) : syr_resp(slot, false, command.toFloat());
      }
      //..................................................................................

      //Tool = pump
      //.......................................................................................................
      else if (ModulesInfos["X"] == "P") {
        (calib == "C") ? pump_resp(slot, true, calval.toFloat()) : pump_resp(slot, false, command.toFloat());
      }
      //.......................................................................................................

      //Tool not attached
      //...................................................
      else {
        Serial.println("No module attached");
        client.publish(TopicDEBUG, "No module attached");
        RedLED(0);
        return;
      }
      //...................................................
    }
    //--------------------------------------------------------------------------------------------------------

    //Slot Y selected
    //--------------------------------------------------------------------------------------------------------
    if (slot == "Y") {

      //Tool = Syringe
      //..................................................................................
      if (ModulesInfos["Y"] == "S") {
        (calib == "C") ?  syr_resp(slot, true) : syr_resp(slot, false, command.toFloat());
      }
      //..................................................................................

      //Tool = pump
      //.......................................................................................................
      else if (ModulesInfos["Y"] == "P") {
        (calib == "C") ? pump_resp(slot, true, calval.toFloat()) : pump_resp(slot, false, command.toFloat());
      }
      //.......................................................................................................

      //Tool not attached
      //...................................................
      else {
        Serial.println("No module attached");
        client.publish(TopicDEBUG, "No module attached");
        RedLED(0);
        return;
      }
      //...................................................
    }
    //--------------------------------------------------------------------------------------------------------

    //Slot Z selected
    //--------------------------------------------------------------------------------------------------------
    if (slot == "Z") {

      //Tool = Syringe
      //..................................................................................
      if (ModulesInfos["Z"] == "S") {
        (calib == "C") ?  syr_resp(slot, true) : syr_resp(slot, false, command.toFloat());
      }
      //..................................................................................

      //Tool = pump
      //.......................................................................................................
      else if (ModulesInfos["Z"] == "P") {
        (calib == "C") ? pump_resp(slot, true, calval.toFloat()) : pump_resp(slot, false, command.toFloat());
      }
      //.......................................................................................................

      //Tool not attached
      //...................................................
      else {
        Serial.println("No module attached");
        client.publish(TopicDEBUG, "No module attached");
        RedLED(0);
        return;
      }
      //...................................................
    }
    //--------------------------------------------------------------------------------------------------------
  }
  //============================================================================================================

  //Get the current config
  //-----------------------------
  if (topic == TopicCONFIG) {
    ConfigModules(message_temp);
  }
  //-----------------------------
}
