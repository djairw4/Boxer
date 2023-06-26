#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TM1637Display.h>     //
#include <DFRobot_LIS2DW12.h>
#define CLK D3                       // Define the connections pins:
#define DIO D4

//When using I2C communication, use the following program to construct an object by DFRobot_LIS2DW12_I2C
/*!
 * @brief Constructor 
 * @param pWire I2c controller
 * @param addr  I2C address(0x18/0x19)
 */
DFRobot_LIS2DW12_I2C acce(&Wire,0x18);
TM1637Display display = TM1637Display(CLK, DIO);
int max_acc=0, start_count=0, stop_count=0;
bool start1_stop0=false;

void setup(void){
  display.clear();
  display.setBrightness(7);    
  Serial.begin(9600);
  while(!acce.begin()){
     Serial.println("Communication failed, check the connection and I2C address setting when using I2C communication.");
     delay(1000);
  }
  Serial.print("chip id : ");
  Serial.println(acce.getID(),HEX);
  //Chip soft reset
  acce.softReset();
  //Set whether to collect data continuously
  acce.continRefresh(true);
  
  
  /**！
    Set the sensor data collection rate:
               eRate_0hz           /<Measurement off>/
               eRate_1hz6          /<1.6hz, use only under low-power mode>/
               eRate_12hz5         /<12.5hz>/
               eRate_25hz          
               eRate_50hz          
               eRate_100hz         
               eRate_200hz         
               eRate_400hz       /<Use only under High-Performance mode>/
               eRate_800hz       /<Use only under High-Performance mode>/
               eRate_1k6hz       /<Use only under High-Performance mode>/
               eSetSwTrig        /<The software triggers a single measurement>/
  */
  acce.setDataRate(DFRobot_LIS2DW12::eRate_200hz);
  
  /**！
    Set the sensor measurement range:
                   e2_g   /<±2g>/
                   e4_g   /<±4g>/
                   e8_g   /<±8g>/
                   e16_g  /< ±16g>/
  */
  acce.setRange(DFRobot_LIS2DW12::e16_g);
  
  
  /**！
    Filter settings:
           eLPF (Low pass filter)
           eHPF (High pass filter)
  */
  acce.setFilterPath(DFRobot_LIS2DW12::eLPF);
  
  /**！
    Set bandwidth：
        eRateDiv_2  /<Rate/2 (up to Rate = 800 Hz, 400 Hz when Rate = 1600 Hz)>/
        eRateDiv_4  /<Rate/4 (High Power/Low power)>*
        eRateDiv_10 /<Rate/10 (HP/LP)>/
        eRateDiv_20 /< Rate/20 (HP/LP)>/
  */
  acce.setFilterBandwidth(DFRobot_LIS2DW12::eRateDiv_4);
  
  /**！
   Set power mode:
       eHighPerformance_14bit         /<High-Performance Mode,14-bit resolution>/
       eContLowPwr4_14bit             /<Continuous measurement,Low-Power Mode 4(14-bit resolution)>/
       eContLowPwr3_14bit             /<Continuous measurement,Low-Power Mode 3(14-bit resolution)>/
       eContLowPwr2_14bit             /<Continuous measurement,Low-Power Mode 2(14-bit resolution)/
       eContLowPwr1_12bit             /<Continuous measurement,Low-Power Mode 1(12-bit resolution)>/
       eSingleLowPwr4_14bit           /<Single data conversion on demand mode,Low-Power Mode 4(14-bit resolution)>/
       eSingleLowPwr3_14bit           /<Single data conversion on demand mode,Low-Power Mode 3(14-bit resolution)>/
       eSingleLowPwr2_14bit           /<Single data conversion on demand mode,Low-Power Mode 2(14-bit resolution)>/
       eSingleLowPwr1_12bit           /<Single data conversion on demand mode,Low-Power Mode 1(12-bit resolution)>/
       eHighPerformanceLowNoise_14bit /<High-Performance Mode,Low-noise enabled,14-bit resolution>/
       eContLowPwrLowNoise4_14bit     /<Continuous measurement,Low-Power Mode 4(14-bit resolution,Low-noise enabled)>/
       eContLowPwrLowNoise3_14bit     /<Continuous measurement,Low-Power Mode 3(14-bit resolution,Low-noise enabled)>/
       eContLowPwrLowNoise2_14bit     /<Continuous measurement,Low-Power Mode 2(14-bit resolution,Low-noise enabled)>/
       eContLowPwrLowNoise1_12bit     /<Continuous measurement,Low-Power Mode 1(12-bit resolution,Low-noise enabled)>/
       eSingleLowPwrLowNoise4_14bit   /<Single data conversion on demand mode,Low-Power Mode 4(14-bit resolution),Low-noise enabled>/
       eSingleLowPwrLowNoise3_14bit   /<Single data conversion on demand mode,Low-Power Mode 3(14-bit resolution),Low-noise enabled>/
       eSingleLowPwrLowNoise2_14bit   /<Single data conversion on demand mode,Low-Power Mode 2(14-bit resolution),Low-noise enabled>/
       eSingleLowPwrLowNoise1_12bit   /<Single data conversion on demand mode,Low-Power Mode 1(12-bit resolution),Low-noise enabled>/
  */
  acce.setPowerMode(DFRobot_LIS2DW12::eContLowPwrLowNoise2_14bit);
  Serial.print("Acceleration:\n");
  delay(100);
}

void loop(void){
    int x,y,z;
    //Request a measurement under single data conversion on demand mode
    //acce.demandData();
    //The mearsurement range is ±2g,±4g,±8g or ±16g, set by the setRange() function.
    //Serial.print("x: ");
    //Read the acceleration in the x direction
    x=abs(acce.readAccX()/10);
    y=abs(acce.readAccY()/10);
    z=abs(acce.readAccZ()/10);
    if(x>max_acc){max_acc = x;}
    if(y>max_acc){max_acc = y;}
    if(z>max_acc) {max_acc = z;}
    if(x<120 && x>80 && z<50){start_count++;}
    else{start_count=0;}
    if(z<120 && z>80 && x<50){stop_count++;}
    else{stop_count=0;}
    // if(start_count==0 && start1_stop0==true){
    //   if(x>max_acc){max_acc = x;}
    //   if(y>max_acc){max_acc = y;}
    //   if(z>max_acc) {max_acc = z;}
    // }
    if(start_count>=15000){start_count = 105;}
    if(stop_count>=15000){stop_count = 105;}
    if(start_count>=100 && start1_stop0==false){
      max_acc=0;
      start1_stop0 = true;
      display.showNumberDec(0, false, 4, 0);      
      //display.clear();
      //Serial.print("start");
      // Serial.print(x);
      // Serial.print("\ty: ");
      // Serial.print(y);
      // Serial.print("\tz: ");
      // Serial.println(z);
    }
    if(stop_count>=100 && start1_stop0==true){
      start1_stop0 = false;
      uint8_t step = max_acc/20;
      for(int i = 0;i<max_acc;i+=step){
        display.showNumberDec(i, false, 4, 0);
      }
      display.showNumberDec(max_acc, false, 4, 0);
      Serial.print("max:");
      // Serial.println(max_acc);
      
    }
    Serial.print(x);
    Serial.print("\ty: ");
    // Read the acceleration in the y direction
    Serial.print(y);
    Serial.print("\tz: ");
    // Read the acceleration in the z direction
    Serial.print(z);
    Serial.print("\tmax:");
    Serial.println(max_acc);
    //delay(300);
}