
//Start the syringe calibration
//======================================================
void SyrHome(String motor) {
  String debug = "Syringe " + motor + " calibrated";
  OrangeLED(0);

  //Homing until home button is pressed
  //---------------------------------
  if (motor == "X") {
    StepperX.calibrate(ZPLUS, false);
  }
  else if (motor == "Y") {
    StepperY.calibrate(ZPLUS, false);
  }
  else if (motor == "Z") {
    StepperZ.calibrate(ZPLUS, false);
  }
  BlueLED(0);
  //---------------------------------


  //Tensioning
  //-------------------------
  if (motor == "X") {
    StepperX.enable();
    StepperX.move(BackPush);
    ModulesInfos["XC"] = 0;
    SaveConfigModule();
  }
  else if (motor == "Y") {
    StepperY.enable();
    StepperY.move(BackPush);
    ModulesInfos["YC"] = 0;
    SaveConfigModule();
  }
  else if (motor == "Z") {
    StepperZ.enable();
    StepperZ.move(BackPush);
    ModulesInfos["ZC"] = 0;
    SaveConfigModule();
  }
  GreenLED(0);
  //-------------------------

  //DEBUG
  //-----------------------------------------
  Serial.println(debug);
  client.publish(TopicDEBUG, debug.c_str());
  //-----------------------------------------
}

//INJECTION
//===============================================================
void SyrInject(String motor, float dist) {
  float calval = ModulesInfos["SC"];
  float pistpos = -1;
  float pistmax = ModulesInfos["SM"];

  //Slot X
  //----------------------------------------------------------
  if (motor == "X") {
    pistpos = ModulesInfos["XC"];
    //Check if the sysringe has been homed
    if (pistpos < 0) {
      Serial.println("ERROR ! Syringe not homed");
      client.publish(TopicDEBUG, "ERROR ! Syringe not homed");
      RedLED(0);
      return;
    }
    else {
      //Check current and future piston position
      if (pistpos + ((dist / calval) * 200) <= pistmax) {
        BlueLED(0);
        StepperX.enable();
        StepperX.move((dist / calval) * 200);
        ModulesInfos["XC"] = pistpos + ((dist / calval) * 200);
        SaveConfigModule();
      }
      else {
        Serial.println("ERROR ! Maximum volume exceeded");
        client.publish(TopicDEBUG, "ERROR ! Maximum volume exceeded");
        RedLED(0);
        return;
      }
    }
  }
  //----------------------------------------------------------

  //Slot Y
  //----------------------------------------------------------
  else if (motor == "Y") {
    //Slot Y selected
    pistpos = ModulesInfos["YC"];
    //Check if the syringe has  been homed
    if (pistpos < 0) {
      Serial.println("ERROR ! Syringe not homed");
      client.publish(TopicDEBUG, "ERROR ! Syringe not homed");
      RedLED(0);
      return;
    }
    else {
      //Check the current and future piston position
      if (pistpos + ((dist / calval) * 200) <= pistmax) {
        BlueLED(0);
        StepperY.enable();
        StepperY.move((dist / calval) * 200);
        ModulesInfos["YC"] = pistpos + ((dist / calval) * 200);
        SaveConfigModule();
      }
      else {
        Serial.println("ERROR ! Maximum volume exceeded");
        client.publish(TopicDEBUG, "ERROR ! Maximum volume exceeded");
        RedLED(0);
        return;
      }
    }
  }
  //----------------------------------------------------------

  //Slot Z
  //----------------------------------------------------------
  else if (motor == "Z") {
    //Slot Z selected
    pistpos = ModulesInfos["ZC"];
    if (pistpos < 0) {
      //Check if the syringe has been homed
      Serial.println("ERROR ! Syringe not homed");
      client.publish(TopicDEBUG, "ERROR ! Syringe not homed");
      RedLED(0);
      return;
    }
    else {
      //Check the current and future piston position then proceed to injection
      if (pistpos + ((dist / calval) * 200) <= pistmax) {
        BlueLED(0);
        StepperZ.enable();
        StepperZ.move((dist / calval) * 200);
        ModulesInfos["ZC"] = pistpos + ((dist / calval) * 200);
        SaveConfigModule();
      }
      else {
        Serial.println("ERROR ! Maximum volume exceeded");
        client.publish(TopicDEBUG, "ERROR ! Maximum volume exceeded");
        RedLED(0);
        return;
      }
    }
  }
  //----------------------------------------------------------

  //DEBUG
  //----------------------------------
  Serial.println("Done");
  client.publish(TopicDEBUG, "Done");
  //----------------------------------
}
//===============================================================
