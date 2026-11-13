
//Start the pump calibration
//===============================================
void PumpCalibTurn(String motor) {
  OrangeLED(0); //Debug LED
  calstep = 0; //Count the number of steps utill the Endstop is pressed

  //..........................................
  if (motor == "X") {
    calstep = StepperX.calibrate(ZPLUS, true);
  }
  else if (motor == "Y") {
    calstep = StepperY.calibrate(ZPLUS, true);
    Serial.println(calstep);
  }
  else if (motor == "Z") {
    calstep = StepperZ.calibrate(ZPLUS, true);
  }
  //..........................................
  
  //DEBUG
  //--------------------------------------------
  BlueLED(2); //Debug LED
  Serial.println("Pump " + motor + " Stopped");
  //--------------------------------------------
}
//===============================================

//Convert calsteps to ml/turn using the mesaured calibration volume
//=================================================================================
void PumpCalibCalc(String motor, float calvol) {
  if (calstep != 0) {
    float calval = calvol / (float(calstep) / 200);

    //Save calval in the memory
    //------------------------------------
    if (motor == "X") {
      ModulesInfos["XC"] = float(calval);
      SaveConfigModule();
    }
    else if (motor == "Y") {
      ModulesInfos["YC"] = float(calval);
      SaveConfigModule();
    }
    else if (motor == "Z") {
      ModulesInfos["ZC"] = float(calval);
      SaveConfigModule();
    }
    //------------------------------------

    //DEBUG
    //----------------------------------------------------------------------------
    GreenLED(0);
    String output = "Calibration performed. Val = " + String(calval) + "ml/turn";
    Serial.println(output);
    client.publish(TopicDEBUG, output.c_str());
    calstep = 0;
    //----------------------------------------------------------------------------
  }

  //ERROR
  //--------------------------------------------------------------
  else {
    Serial.println("ERROR ! Incomplete calibration");
    client.publish(TopicDEBUG, "ERROR ! Incomplete calibration");
  }
  //--------------------------------------------------------------
}
//=================================================================================

//INJECTION
//=====================================================================
void PumpInject(String motor, float volume) {
  float calval = 0;

  //Slot X
  //-------------------------------------------------------------
  if (motor == "X") {
    calval = ModulesInfos["XC"];
    if (calval < 0) {
      Serial.println("ERROR ! Pump not calibrated");
      client.publish(TopicDEBUG, "ERROR ! Pump not calibrated");
      RedLED(0);
      return;
    }
    else {
      BlueLED(0);
      StepperX.enable();
      StepperX.move((volume / calval) * 200);
    }
  }
  //-------------------------------------------------------------

  //Slot Y
  //-------------------------------------------------------------
  else if (motor == "Y") {
    calval = ModulesInfos["YC"];
    if (calval < 0) {
      Serial.println("ERROR ! Pump not calibrated");
      client.publish(TopicDEBUG, "ERROR ! Pump not calibrated");
      RedLED(0);
      return;
    }
    else {
      BlueLED(0);
      StepperY.enable();
      StepperY.move((volume / calval) * 200);
    }
  }
  //-------------------------------------------------------------

  //Slot Z
  //-------------------------------------------------------------
  else if (motor == "Z") {
    calval = ModulesInfos["ZC"];
    if (calval < 0) {
      Serial.println("ERROR ! Pump not calibrated");
      client.publish(TopicDEBUG, "ERROR ! Pump not calibrated");
      RedLED(0);
      return;
    }
    else {
      BlueLED(0);
      StepperZ.enable();
      StepperZ.move((volume / calval) * 200);
    }
  }
  //-------------------------------------------------------------

  Serial.println("Done");
  client.publish(TopicDEBUG, "Done");
}
//=====================================================================
