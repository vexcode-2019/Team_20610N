#include "main.h"
// #include "fstream"

ADIEncoder leftEncoder('C', 'D');
ADIEncoder rightEncoder('A', 'B');
ADIGyro Gyro('E',1);

// std::shared_ptr<ChassisController> chassis;

int newHeading = 0;
int JoisticHeading = 0;
int chassisHeading;
bool chassisBrake = false;

Motor leftMotor(7, false, AbstractMotor::gearset::red, AbstractMotor::encoderUnits::degrees);
Motor rightMotor(5, true, AbstractMotor::gearset::red, AbstractMotor::encoderUnits::degrees);
Motor middleMotor(6, false, AbstractMotor::gearset::red, AbstractMotor::encoderUnits::degrees);
std::shared_ptr<ChassisController> chassis = ChassisControllerBuilder()
  .withMotors(leftMotor, rightMotor, middleMotor)
  .withSensors(leftEncoder, rightEncoder)
  .withDimensions({{4_in, 10_in}, imev5RedTPR})
  .withMaxVelocity(100)
  .withLogger(
      std::make_shared<Logger>(
          TimeUtilFactory::createDefault().getTimer(), // It needs a Timer
          "/usd/Chassis_Diagnostics", // Output to the SDCard
          Logger::LogLevel::info // Most verbose log level
      )
  )
  .build();

// Thread that controls the chassis 
void ChassisOpcontrol(void* param) {

  // chassis declaration
    
  // // Write GyroHeading and ChassisVelocity to a file
  // FILE *fp = fopen("/usd/Chassis_Diagnostics", "a");
  // std::string diagnosticsStr = "Testing\n";
  // fprintf(fp, "%s", diagnosticsStr.c_str()); 
  // fclose(fp);
    
  // assigning the chassis to a H-drive model
  std::shared_ptr<okapi::HDriveModel> driveTrain = std::dynamic_pointer_cast<HDriveModel>(chassis->getModel());
  // driveTrain->setBrakeMode(AbstractMotor::brakeMode::hold);  
  
  int leftMotorControl = 0;
  int rightMotorControl = 0;
  int middleMotorControl = 0;
  double forward, straff, turning;
  int temp;
  float theta;
  bool FieldCenteric = true;
  
  ControllerButton fieldCentericToggle(ControllerDigital::A);
  ControllerButton gyroReset(ControllerDigital::X);

  while (isAuton == false) { // isAuton == false
    chassisHeading = Gyro.get() / 10;
    newHeading = chassisHeading - JoisticHeading;

    //Updates display values.
		updateLineVariable(1, chassisHeading);
		updateLineVariable(2, JoisticHeading); //middleEncoder.controllerGet()
		updateLineVariable(3, leftEncoder.controllerGet());
		updateLineVariable(4, rightEncoder.controllerGet());
    
    // Graphs the drive motor temps
    lv_chart_set_next(chart, GreenLine, leftMotor.getTemperature());
    lv_chart_set_next(chart, LimeLine, rightMotor.getTemperature());
    lv_chart_set_next(chart, SilverLine, middleMotor.getTemperature());
    
    forward = master.getAnalog(ControllerAnalog::leftY)*100;
    if (abs(forward) < 0.3) {
      forward = 0;
    }
    straff = master.getAnalog(ControllerAnalog::leftX)*100;
    if (abs(straff) < 0.3) {
      straff = 0;
    }
    turning = master.getAnalog(ControllerAnalog::rightX)*100;
    if (abs(turning) < 0.3) {
      turning = 0;
    }

    theta = (-chassisHeading * 3.1415926535) / 180;
    temp  = forward * cos(theta) - straff * sin(theta);
    straff = forward * sin(theta) + straff * cos(theta);
    forward = temp;
    
    if (fieldCentericToggle.changedToPressed()) {
      if (FieldCenteric == true) {
        FieldCenteric = false;
      }
      else {
        FieldCenteric = true;
      }
    }
    
    if (gyroReset.changedToPressed()) {
      Gyro.reset();
    }
    
    if (FieldCenteric == true) {
      driveTrain->hArcade((straff / 100),
                          (forward / 100),
                          (turning / 100), 0);
    }
    else {
      driveTrain->hArcade(master.getAnalog(ControllerAnalog::leftX), 
                          master.getAnalog(ControllerAnalog::leftY), 
                          master.getAnalog(ControllerAnalog::rightX), 0.3);
    }
    //   chassisBrake = false;
    
    // if (chassisBrake == false) {
    //   leftMotor.moveRelative(0, 200);
    //   rightMotor.moveRelative(0, 200);
    //   middleMotor.moveRelative(0, 200);
    //   chassisBrake = true;
    // }
    pros::delay(20);
  }
}