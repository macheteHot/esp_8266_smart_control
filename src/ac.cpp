#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ir_Airton.h>
#include <ir_Airwell.h>
#include <ir_Amcor.h>
#include <ir_Argo.h>
#include <ir_Bosch.h>
#include <ir_Carrier.h>
#include <ir_Coolix.h>
#include <ir_Corona.h>
#include <ir_Daikin.h>
#include <ir_Delonghi.h>
#include <ir_Ecoclim.h>
#include <ir_Electra.h>
#include <ir_Fujitsu.h>
#include <ir_Goodweather.h>
#include <ir_Gree.h>
#include <ir_Haier.h>
#include <ir_Hitachi.h>
#include <ir_Kelon.h>
#include <ir_Kelvinator.h>
#include <ir_LG.h>
#include <ir_Midea.h>
#include <ir_Mirage.h>
#include <ir_Mitsubishi.h>
#include <ir_Neoclima.h>
#include <ir_Panasonic.h>
#include <ir_Rhoss.h>
#include <ir_Samsung.h>
#include <ir_Sanyo.h>
#include <ir_Sharp.h>
#include <ir_Tcl.h>
#include <ir_Technibel.h>
#include <ir_Teco.h>
#include <ir_Toshiba.h>
#include <ir_Transcold.h>
#include <ir_Trotec.h>
#include <ir_Truma.h>
#include <ir_Vestel.h>
#include <ir_Voltas.h>
#include <ir_Whirlpool.h>
#include <ir_York.h>

extern PubSubClient client;
extern String availabilityTopic;
extern String macStr;
extern String configPageUrl;

void sendLog(const String &msg);
int getCurrentMonth();
void splitCommaSeparated(const String &payload, std::vector<String> &array);
namespace {

constexpr int kIrLedPin = 4;
int acTemperature = 24;
String acPower = "on";
String acMode = "cool";
String acBrand = "";
// off vertical horizontal both
String acSwing = "off";
// auto low medium high
String acFan = "auto";

// 空调自动发现
String climateConfigTopic = "homeassistant/climate/" + macStr + "/config";
// 空调模式设置
String ModeCommandTopic = "ac_remote/climate/" + macStr + "/mode/set";
// 空调模式状态
String modeStateTopic = "ac_remote/climate/" + macStr + "/mode/state";
// 空调温度设置
String TempCommandTopic = "ac_remote/climate/" + macStr + "/temp/set";
// 设置温度上报
String tempStateTopic = "ac_remote/climate/" + macStr + "/temp/state";
// 当前温度上报
String currentTempTopic = "ac_remote/climate/" + macStr + "/temp/current";
// 风速设置
String fanModesCommandTopic = "ac_remote/climate/" + macStr + "/fan/set";
// 风速状态
String fanModesStateTopic = "ac_remote/climate/" + macStr + "/fan/state";
// 摆风模式设置
String swingModeCommandTopic = "ac_remote/climate/" + macStr + "/swing/set";
// 摆风模式状态
String swingModeStateTopic = "ac_remote/climate/" + macStr + "/swing/state";
// 空调型号自动发现
String acBrandConfigTopic = "homeassistant/select/ac_" + macStr + "/config";
// 空调型号选择
String acBrandCommandTopic = "config/ha/ac/brand/ac_" + macStr + "/set";
String acBrandStateTopic = "config/ha/ac/brand/ac_" + macStr + "/state";
// 空调遥控模式切换码
String climateModeToggleTextConfigTopic =
    "homeassistant/text/climate_mode_" + macStr + "/config";
String climateModeToggleCommandTopic =
    "config/ha/climate/mode_" + macStr + "/set";
String climateModeToggleStateTopic =
    "config/ha/climate/mode_" + macStr + "/state";

// 空调摆风码
String climateSwingToggletextConfigTopic =
    "homeassistant/text/climate_swing_" + macStr + "/config";
String climateSwingToggleCommandTopic =
    "config/ha/climate/swing_" + macStr + "/set";
String climateSwingToggleStateTopic =
    "config/ha/climate/swing_" + macStr + "/state";

// 空调增温码
String climateTempUpTextConfigTopic =
    "homeassistant/text/climate_temp_up_" + macStr + "/config";
String climateTempUpCommandTopic =
    "config/ha/climate/temp_up_" + macStr + "/set";
String climateTempUpStateTopic =
    "config/ha/climate/temp_up_" + macStr + "/state";

// 空调降温码
String climateTempDownTextConfigTopic =
    "homeassistant/text/climate_temp_down_" + macStr + "/config";
String climateTempDownCommandTopic =
    "config/ha/climate/temp_down_" + macStr + "/set";
String climateTempDownStateTopic =
    "config/ha/climate/temp_down_" + macStr + "/state";

std::vector<String> ToggleCodesArray;
std::vector<String> TempUpCodesArray;
std::vector<String> TempDownCodesArray;
std::vector<String> SwingCodesArray;

void AirtonAcSend() { // 艾尔 顿
  IRAirtonAc airtonAc(kIrLedPin);
  airtonAc.begin();
  airtonAc.setPower(acPower == "on");
  airtonAc.setMode(acMode == "cool" ? kAirtonCool : kAirtonHeat);
  uint8_t fanMode = kAirtonFanAuto;
  if (acFan == "auto") {
    fanMode = kAirtonFanAuto;
  } else if (acFan == "low") {
    fanMode = kAirtonFanMin;
  } else if (acFan == "medium") {
    fanMode = kAirtonFanMed;
  } else if (acFan == "high") {
    fanMode = kAirtonFanMax;
  }
  airtonAc.setFan(fanMode);
  airtonAc.setSwingV(false);
  airtonAc.setSleep(false);
  airtonAc.setEcono(false);
  airtonAc.setSwingV(acSwing == "on");
  airtonAc.setTemp(acTemperature);
  airtonAc.send();
}

void AirwellSend() { // 艾尔威
  IRAirwellAc airwellAc(kIrLedPin);
  airwellAc.begin();
  airwellAc.setPowerToggle(acPower == "on");
  airwellAc.setMode(acMode == "cool" ? kAirwellCool : kAirwellHeat);
  uint8_t fanMode = kAirwellFanAuto;
  if (acFan == "auto") {
    fanMode = kAirwellFanAuto;
  } else if (acFan == "low") {
    fanMode = kAirwellFanLow;
  } else if (acFan == "medium") {
    fanMode = kAirwellFanMedium;
  } else if (acFan == "high") {
    fanMode = kAirwellFanHigh;
  }
  airwellAc.setFan(fanMode);
  airwellAc.setTemp(acTemperature);
  airwellAc.send();
}

void AmcorSend() {
  IRAmcorAc amcorAc(kIrLedPin);
  amcorAc.begin();
  amcorAc.setPower(acPower == "on");
  amcorAc.setMode(acMode == "cool" ? kAmcorCool : kAmcorHeat);
  uint8_t fanMode = kAmcorFanAuto;
  if (acFan == "auto") {
    fanMode = kAmcorFanAuto;
  } else if (acFan == "low") {
    fanMode = kAmcorFanMin;
  } else if (acFan == "medium") {
    fanMode = kAmcorFanMed;
  } else if (acFan == "high") {
    fanMode = kAmcorFanMax;
  }
  amcorAc.setFan(fanMode);
  amcorAc.setTemp(acTemperature);
  amcorAc.send();
}

void ArgoSend() {
  IRArgoAC argoAc(kIrLedPin);
  argoAc.begin();
  argoAc.setPower(acPower == "on");
  argoAc.setMode(acMode == "cool" ? kArgoCool : kArgoHeat);
  uint8_t fanMode = kArgoFanAuto;
  if (acFan == "auto") {
    fanMode = kArgoFanAuto;
  } else if (acFan == "low") {
    fanMode = kArgoFan1;
  } else if (acFan == "medium") {
    fanMode = kArgoFan2;
  } else if (acFan == "high") {
    fanMode = kArgoFan3;
  }
  argoAc.setFan(fanMode);
  argoAc.setTemp(acTemperature);
  argoAc.setNight(false);
  argoAc.send();
}

void BoschSend() {
  IRBosch144AC boschAc(kIrLedPin);
  boschAc.begin();
  boschAc.setPower(acPower == "on");
  boschAc.setMode(acMode == "cool" ? kBosch144Cool : kBosch144Heat);
  uint16_t fanMode = kBosch144FanAuto;
  if (acFan == "auto") {
    fanMode = kBosch144FanAuto;
  } else if (acFan == "low") {
    fanMode = kBosch144Fan20;
  } else if (acFan == "medium") {
    fanMode = kBosch144Fan60;
  } else if (acFan == "high") {
    fanMode = kBosch144Fan100;
  }
  boschAc.setFan(fanMode);
  boschAc.setQuiet(false);
  boschAc.setTemp(acTemperature);
  boschAc.send();
}
void CarrierSend() {
  IRCarrierAc64 carrierAc(kIrLedPin);
  carrierAc.begin();
  carrierAc.setPower(acPower == "on");
  carrierAc.setMode(acMode == "cool" ? kCarrierAc64Cool : kCarrierAc64Heat);
  uint8_t fanMode = kCarrierAc64FanAuto;
  if (acFan == "auto") {
    fanMode = kCarrierAc64FanAuto;
  } else if (acFan == "low") {
    fanMode = kCarrierAc64FanLow;
  } else if (acFan == "medium") {
    fanMode = kCarrierAc64FanMedium;
  } else if (acFan == "high") {
    fanMode = kCarrierAc64FanHigh;
  }
  carrierAc.setFan(fanMode);
  carrierAc.setSwingV(acSwing == "on");
  carrierAc.setTemp(acTemperature);
  carrierAc.send();
}
void CoolixSend() {
  IRCoolixAC coolixAc(kIrLedPin);
  coolixAc.begin();
  coolixAc.setPower(acPower == "on");
  coolixAc.setMode(acMode == "cool" ? kCoolixCool : kCoolixHeat);
  coolixAc.setFan(kCoolixFanAuto);
  uint8_t fanMode = kCoolixFanAuto;
  if (acFan == "auto") {
    fanMode = kCoolixFanAuto;
  } else if (acFan == "low") {
    fanMode = kCoolixFanMin;
  } else if (acFan == "medium") {
    fanMode = kCoolixFanMed;
  } else if (acFan == "high") {
    fanMode = kCoolixFanMax;
  }
  coolixAc.setFan(fanMode);
  // coolixAc.setLed(Kcoolled); toggleLed
  bool acSwingState = coolixAc.getSwing();
  if (acSwingState != (acSwing == "on")) {
    coolixAc.setSwing(); // 切换状态
  }
  coolixAc.setTemp(acTemperature);
  coolixAc.send();
}
void CoronaSend() {
  IRCoronaAc coronaAc(kIrLedPin);
  coronaAc.begin();
  coronaAc.setPower(acPower == "on");
  coronaAc.setMode(acMode == "cool" ? kCoronaAcModeCool : kCoronaAcModeHeat);
  uint8_t fanMode = kCoronaAcFanAuto;
  if (acFan == "auto") {
    fanMode = kCoronaAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kCoronaAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kCoronaAcFanMedium;
  } else if (acFan == "high") {
    fanMode = kCoronaAcFanHigh;
  }
  coronaAc.setFan(fanMode);
  coronaAc.setEcono(false);
  coronaAc.setSwingVToggle(acSwing == "on");
  coronaAc.setTemp(acTemperature);
  coronaAc.send();
}
void Daikin128Send() {
  IRDaikin128 daikin128Ac(kIrLedPin);
  daikin128Ac.begin();
  daikin128Ac.setPowerToggle(acPower == "on");
  daikin128Ac.setMode(acMode == "cool" ? kDaikinCool : kDaikinHeat);
  uint8_t fanMode = kDaikin128FanAuto;
  if (acFan == "auto") {
    fanMode = kDaikin128FanAuto;
  } else if (acFan == "low") {
    fanMode = kDaikin128FanLow;
  } else if (acFan == "medium") {
    fanMode = kDaikin128FanMed;
  } else if (acFan == "high") {
    fanMode = kDaikin128FanHigh;
  }
  daikin128Ac.setFan(fanMode);
  daikin128Ac.setSwingVertical(acSwing == "on");
  daikin128Ac.setSleep(false);
  daikin128Ac.setEcono(false);
  daikin128Ac.setPowerful(false);
  daikin128Ac.setTemp(acTemperature);
  daikin128Ac.send();
}
void DelonghiSend() {
  IRDelonghiAc delonghiAc(kIrLedPin);
  delonghiAc.begin();
  delonghiAc.setPower(acPower == "on");
  delonghiAc.setMode(acMode == "cool" ? kDelonghiAcCool : kDelonghiAcDry);
  uint8_t fanMode = kDelonghiAcFanAuto;
  if (acFan == "auto") {
    fanMode = kDelonghiAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kDelonghiAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kDelonghiAcFanMedium;
  } else if (acFan == "high") {
    fanMode = kDelonghiAcFanHigh;
  }
  delonghiAc.setFan(fanMode);
  delonghiAc.setBoost(false);
  delonghiAc.setTemp(acTemperature);
  delonghiAc.send();
}
void EcoclimSend() {
  IREcoclimAc ecoclimAc(kIrLedPin);
  ecoclimAc.begin();
  ecoclimAc.setPower(acPower == "on");
  ecoclimAc.setMode(acMode == "cool" ? kEcoclimCool : kEcoclimHeat);
  uint8_t fanMode = kEcoclimFanAuto;
  if (acFan == "auto") {
    fanMode = kEcoclimFanAuto;
  } else if (acFan == "low") {
    fanMode = kEcoclimFanMin;
  } else if (acFan == "medium") {
    fanMode = kEcoclimFanMed;
  } else if (acFan == "high") {
    fanMode = kEcoclimFanMax;
  }
  ecoclimAc.setFan(fanMode);
  ecoclimAc.setTemp(acTemperature);
  ecoclimAc.send();
}
void ElectraSend() {
  IRElectraAc electraAc(kIrLedPin);
  electraAc.begin();
  electraAc.setPower(acPower == "on");
  electraAc.setMode(acMode == "cool" ? kElectraAcCool : kElectraAcHeat);
  uint8_t fanMode = kElectraAcFanAuto;
  if (acFan == "auto") {
    fanMode = kElectraAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kElectraAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kElectraAcFanMed;
  } else if (acFan == "high") {
    fanMode = kElectraAcFanHigh;
  }
  electraAc.setFan(fanMode);
  electraAc.setSwingH(false);
  electraAc.setSwingV(acSwing == "on");
  electraAc.setTemp(acTemperature);
  electraAc.send();
}
void FujitsuSend() {
  IRFujitsuAC fujitsuAc(kIrLedPin);
  fujitsuAc.begin();
  fujitsuAc.setPower(acPower == "on");
  fujitsuAc.setMode(acMode == "cool" ? kFujitsuAcModeCool : kFujitsuAcModeHeat);
  uint8_t fanMode = kFujitsuAcFanAuto;
  if (acFan == "auto") {
    fanMode = kFujitsuAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kFujitsuAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kFujitsuAcFanMed;
  } else if (acFan == "high") {
    fanMode = kFujitsuAcFanHigh;
  }
  fujitsuAc.setFanSpeed(fanMode);

  fujitsuAc.setSwing(acSwing == "on" ? kFujitsuAcSwingVert
                                     : kFujitsuAcSwingOff);
  fujitsuAc.setTemp(acTemperature);
  fujitsuAc.send();
}
void GoodweatherSend() {
  IRGoodweatherAc goodweatherAc(kIrLedPin);
  goodweatherAc.begin();
  goodweatherAc.setPower(acPower == "on");
  goodweatherAc.setMode(acMode == "cool" ? kGoodweatherCool : kGoodweatherHeat);
  uint8_t fanMode = kGoodweatherFanAuto;
  if (acFan == "auto") {
    fanMode = kGoodweatherFanAuto;
  } else if (acFan == "low") {
    fanMode = kGoodweatherFanLow;
  } else if (acFan == "medium") {
    fanMode = kGoodweatherFanMed;
  } else if (acFan == "high") {
    fanMode = kGoodweatherFanHigh;
  }
  goodweatherAc.setFan(fanMode);
  goodweatherAc.setSwing(acSwing == "on" ? kGoodweatherSwingFast
                                         : kGoodweatherSwingOff);
  goodweatherAc.setTemp(acTemperature);
  goodweatherAc.setLight(false); // mbo 空调是反过来的不知道为啥
  goodweatherAc.setSleep(false);
  goodweatherAc.setTurbo(false);
  goodweatherAc.send();
}
void GreeSend() {
  IRGreeAC greeAc(kIrLedPin);
  greeAc.begin();
  greeAc.setPower(acPower == "on");
  greeAc.setMode(acMode == "cool" ? kGreeCool : kGreeHeat);
  greeAc.setTurbo(false);
  greeAc.setSleep(false);
  greeAc.setEcono(false);
  uint8_t fanMode = kGreeFanAuto;
  if (acFan == "auto") {
    fanMode = kGreeFanAuto;
  } else if (acFan == "low") {
    fanMode = kGreeFanMin;
  } else if (acFan == "medium") {
    fanMode = kGreeFanMed;
  } else if (acFan == "high") {
    fanMode = kGreeFanMax;
  }
  greeAc.setFan(fanMode);
  greeAc.setSwingHorizontal(kGreeSwingHOff);
  greeAc.setSwingVertical(acSwing == "on", kGreeSwingAuto);
  greeAc.setTemp(acTemperature);
  greeAc.send();
}

void HaierSend() {
  IRHaierAC160 haierAc160(kIrLedPin);
  haierAc160.begin();
  haierAc160.setPower(acPower == "on");
  haierAc160.setMode(acMode == "cool" ? kHaierAcCool : kHaierAcHeat);
  uint8_t fanMode = kHaierAcFanAuto;
  if (acFan == "auto") {
    fanMode = kHaierAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kHaierAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kHaierAcFanMed;
  } else if (acFan == "high") {
    fanMode = kHaierAcFanHigh;
  }
  haierAc160.setFan(fanMode);
  haierAc160.setSwingV(acSwing == "on" ? kHaierAc160SwingVAuto
                                       : kHaierAc160SwingVOff);
  haierAc160.setQuiet(false);
  haierAc160.setTurbo(false);
  haierAc160.setTemp(acTemperature);
  haierAc160.send();
}

void Haier176Send() {
  IRHaierAC176 haierAc176(kIrLedPin);
  haierAc176.begin();
  haierAc176.setPower(acPower == "on");
  haierAc176.setMode(acMode == "cool" ? kHaierAcCool : kHaierAcHeat);
  uint8_t fanMode = kHaierAcFanAuto;
  if (acFan == "auto") {
    fanMode = kHaierAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kHaierAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kHaierAcFanMed;
  } else if (acFan == "high") {
    fanMode = kHaierAcFanHigh;
  }
  haierAc176.setFan(fanMode);
  haierAc176.setSwingV(acSwing == "on" ? kHaierAcYrw02SwingVAuto
                                       : kHaierAcYrw02SwingVOff);
  haierAc176.setQuiet(false);
  haierAc176.setTurbo(false);
  haierAc176.setTemp(acTemperature);
  haierAc176.send();
}

void HaierYRW02Send() {
  IRHaierACYRW02 haierAcYrw02(kIrLedPin);
  haierAcYrw02.begin();
  haierAcYrw02.setPower(acPower == "on");
  haierAcYrw02.setMode(acMode == "cool" ? kHaierAcYrw02Cool
                                        : kHaierAcYrw02Heat);
  uint8_t fanMode = kHaierAcYrw02FanAuto;
  if (acFan == "auto") {
    fanMode = kHaierAcYrw02FanAuto;
  } else if (acFan == "low") {
    fanMode = kHaierAcYrw02FanLow;
  } else if (acFan == "medium") {
    fanMode = kHaierAcYrw02FanMed;
  } else if (acFan == "high") {
    fanMode = kHaierAcYrw02FanHigh;
  }
  haierAcYrw02.setFan(fanMode);
  haierAcYrw02.setSwingV(acSwing == "on" ? kHaierAcYrw02SwingVAuto
                                         : kHaierAcYrw02SwingVOff);
  haierAcYrw02.setQuiet(false);
  haierAcYrw02.setTurbo(false);
  haierAcYrw02.setTemp(acTemperature);
  haierAcYrw02.send();
}

void HitachiSend() {
  IRHitachiAc hitachiAc(kIrLedPin);
  hitachiAc.begin();
  hitachiAc.setPower(acPower == "on");
  hitachiAc.setMode(acMode == "cool" ? kHitachiAcCool : kHitachiAcHeat);
  uint8_t fanMode = kHitachiAcFanAuto;
  if (acFan == "auto") {
    fanMode = kHitachiAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kHitachiAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kHitachiAcFanMed;
  } else if (acFan == "high") {
    fanMode = kHitachiAcFanHigh;
  }
  hitachiAc.setFan(fanMode);
  hitachiAc.setSwingVertical(acSwing == "on");
  hitachiAc.setSwingHorizontal(false);
  hitachiAc.setTemp(acTemperature);
  hitachiAc.send();
}
void KelonSend() {
  IRKelonAc kelonAc(kIrLedPin);
  kelonAc.begin();
  kelonAc.setTogglePower(acPower == "on");
  kelonAc.setMode(acMode == "cool" ? kKelonModeCool : kKelonModeHeat);
  uint8_t fanMode = kKelonFanAuto;
  if (acFan == "auto") {
    fanMode = kKelonFanAuto;
  } else if (acFan == "low") {
    fanMode = kKelonFanMin;
  } else if (acFan == "medium") {
    fanMode = kKelonFanMedium;
  } else if (acFan == "high") {
    fanMode = kKelonFanMax;
  }
  kelonAc.setFan(fanMode);
  kelonAc.setToggleSwingVertical(acSwing == "on");
  kelonAc.setTemp(acTemperature);
  kelonAc.send();
}
void KelvinatorSend() {
  IRKelvinatorAC kelvinatorAc(kIrLedPin);
  kelvinatorAc.begin();
  kelvinatorAc.setPower(acPower == "on");
  kelvinatorAc.setMode(acMode == "cool" ? kKelvinatorCool : kKelvinatorHeat);
  uint8_t fanMode = kKelvinatorFanAuto;
  if (acFan == "auto") {
    fanMode = kKelvinatorFanAuto;
  } else if (acFan == "low") {
    fanMode = kKelvinatorFanMin;
  } else if (acFan == "medium") {
    fanMode = kKelvinatorFanAuto; // 中速没有
  } else if (acFan == "high") {
    fanMode = kKelvinatorFanMax;
  }
  kelvinatorAc.setFan(fanMode);
  kelvinatorAc.setSwingVertical(acSwing == "on", kKelvinatorSwingVAuto);
  kelvinatorAc.setSwingHorizontal(false);
  kelvinatorAc.setTemp(acTemperature);
  kelvinatorAc.send();
}
void LGSend() { // LG
  IRLgAc lgAc(kIrLedPin);
  lgAc.begin();
  lgAc.setPower(acPower == "on");
  lgAc.setMode(acMode == "cool" ? kLgAcCool : kLgAcHeat);
  uint8_t fanMode = kLgAcFanAuto;
  if (acFan == "auto") {
    fanMode = kLgAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kLgAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kLgAcFanMedium;
  } else if (acFan == "high") {
    fanMode = kLgAcFanHigh;
  }
  lgAc.setFan(fanMode);
  lgAc.setPower(false);
  lgAc.setSwingH(false);
  lgAc.setSwingV(acSwing == "on");
  lgAc.setLight(true);
  lgAc.setTemp(acTemperature);
  lgAc.send();
}

void MideaSend() { // 美的
  IRMideaAC mideaAc(kIrLedPin);
  mideaAc.begin();
  mideaAc.setPower(acPower == "on");
  mideaAc.setMode(acMode == "cool" ? kMideaACCool : kMideaACHeat);
  uint8_t fanMode = kMideaACFanAuto;
  if (acFan == "auto") {
    fanMode = kMideaACFanAuto;
  } else if (acFan == "low") {
    fanMode = kMideaACFanLow;
  } else if (acFan == "medium") {
    fanMode = kMideaACFanMed;
  } else if (acFan == "high") {
    fanMode = kMideaACFanHigh;
  }
  mideaAc.setFan(fanMode);
  mideaAc.setEconoToggle(false);
  mideaAc.setTurboToggle(false);
  mideaAc.setSwingVToggle(acSwing == "on");
  mideaAc.setSleep(false);
  mideaAc.setTemp(acTemperature);
  mideaAc.send();
}
void MirageSend() {
  IRMirageAc mirageAc(kIrLedPin);
  mirageAc.begin();
  mirageAc.setPower(acPower == "on");
  mirageAc.setMode(acMode == "cool" ? kMirageAcCool : kMirageAcHeat);
  uint8_t fanMode = kMirageAcFanAuto;
  if (acFan == "auto") {
    fanMode = kMirageAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kMirageAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kMirageAcFanMed;
  } else if (acFan == "high") {
    fanMode = kMirageAcFanHigh;
  }
  mirageAc.setFan(fanMode);
  mirageAc.setSleep(false);
  mirageAc.setSwingH(false);
  mirageAc.setSwingV(acSwing == "on" ? kMirageAcSwingVAuto
                                     : kMirageAcSwingVOff);
  mirageAc.setTemp(acTemperature);
  mirageAc.send();
}
void MitsubishiSend() { // 三菱电机
  IRMitsubishiAC mitsubishiAc(kIrLedPin);
  mitsubishiAc.begin();
  mitsubishiAc.setPower(acPower == "on");
  mitsubishiAc.setMode(acMode == "cool" ? kMitsubishiAcCool
                                        : kMitsubishiAcHeat);
  uint8_t fanMode = kMitsubishiAcFanAuto;
  if (acFan == "auto") {
    fanMode = kMitsubishiAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kMitsubishiAcFanSilent;
  } else if (acFan == "medium") {
    fanMode = kMitsubishiAcFanRealMax;
  } else if (acFan == "high") {
    fanMode = kMitsubishiAcFanMax;
  }
  mitsubishiAc.setFan(fanMode);
  mitsubishiAc.setTemp(acTemperature);
  mitsubishiAc.send();
}

void NeoclimaSend() {
  IRNeoclimaAc neoclimaAc(kIrLedPin);
  neoclimaAc.begin();
  neoclimaAc.setPower(acPower == "on");
  neoclimaAc.setMode(acMode == "cool" ? kNeoclimaCool : kNeoclimaHeat);
  neoclimaAc.setFan(kNeoclimaFanAuto);
  neoclimaAc.setTemp(acTemperature);
  neoclimaAc.send();
}
void PanasonicSend() {
  IRPanasonicAc panasonicAc(kIrLedPin);
  panasonicAc.begin();
  panasonicAc.setPower(acPower == "on");
  panasonicAc.setMode(acMode == "cool" ? kPanasonicAcCool : kPanasonicAcHeat);
  uint8_t fanMode = kPanasonicAcFanAuto;
  if (acFan == "auto") {
    fanMode = kPanasonicAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kPanasonicAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kPanasonicAcFanMed;
  } else if (acFan == "high") {
    fanMode = kPanasonicAcFanHigh;
  }
  panasonicAc.setFan(fanMode);
  panasonicAc.setTemp(acTemperature);
  panasonicAc.setSwingHorizontal(kPanasonicAcSwingHMiddle);
  panasonicAc.setSwingVertical(acSwing == "on" ? kPanasonicAcSwingVAuto
                                               : kPanasonicAcSwingVMiddle);
  panasonicAc.setPowerful(false);
  panasonicAc.setQuiet(false);
  panasonicAc.send();
}

void RhossSend() {
  IRRhossAc rhossAc(kIrLedPin);
  rhossAc.begin();
  rhossAc.setPower(acPower == "on");
  rhossAc.setMode(acMode == "cool" ? kRhossModeCool : kRhossModeHeat);
  uint8_t fanMode = kRhossFanAuto;
  if (acFan == "auto") {
    fanMode = kRhossFanAuto;
  } else if (acFan == "low") {
    fanMode = kRhossFanMin;
  } else if (acFan == "medium") {
    fanMode = kRhossFanMed;
  } else if (acFan == "high") {
    fanMode = kRhossFanMax;
  }
  rhossAc.setFan(fanMode);
  rhossAc.setSwing(acSwing == "on");
  rhossAc.setTemp(acTemperature);
  rhossAc.send();
}
void SamsungSend() {
  IRSamsungAc samsungAc(kIrLedPin);
  samsungAc.begin();
  samsungAc.setPower(acPower == "on");
  samsungAc.setMode(acMode == "cool" ? kSamsungAcCool : kSamsungAcHeat);
  uint8_t fanMode = kSamsungAcFanAuto;
  if (acFan == "auto") {
    fanMode = kSamsungAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kSamsungAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kSamsungAcFanMed;
  } else if (acFan == "high") {
    fanMode = kSamsungAcFanHigh;
  }
  samsungAc.setFan(fanMode);
  samsungAc.setBeep(false);
  samsungAc.setQuiet(false);
  samsungAc.setPowerful(false);
  samsungAc.setSwing(acSwing == "on");
  samsungAc.setTemp(acTemperature);
  samsungAc.send();
}
void SanyoSend() {
  IRSanyoAc sanyoAc(kIrLedPin);
  sanyoAc.begin();
  sanyoAc.setPower(acPower == "on");
  sanyoAc.setMode(acMode == "cool" ? kSanyoAcCool : kSanyoAcHeat);
  uint8_t fanMode = kSanyoAcFanAuto;
  if (acFan == "auto") {
    fanMode = kSanyoAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kSanyoAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kSanyoAcFanMedium;
  } else if (acFan == "high") {
    fanMode = kSanyoAcFanHigh;
  }

  sanyoAc.setFan(fanMode);
  sanyoAc.setBeep(false);
  sanyoAc.setSwingV(acSwing == "on" ? kSanyoAcSwingVAuto
                                    : kSanyoAcSwingVLowerMiddle);
  sanyoAc.setTemp(acTemperature);
  sanyoAc.send();
}
void SharpSend() {
  IRSharpAc sharpAc(kIrLedPin);
  sharpAc.begin();
  sharpAc.setPower(acPower == "on");
  sharpAc.setMode(acMode == "cool" ? kSharpAcCool : kSharpAcHeat);
  uint8_t fanMode = kSharpAcFanAuto;
  if (acFan == "auto") {
    fanMode = kSharpAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kSharpAcFanMin;
  } else if (acFan == "medium") {
    fanMode = kSharpAcFanMed;
  } else if (acFan == "high") {
    fanMode = kSharpAcFanMax;
  }
  sharpAc.setFan(fanMode);
  sharpAc.setSwingV(kSharpAcSwingVToggle, acSwing == "on");
  sharpAc.setTemp(acTemperature);
  sharpAc.send();
}
void TclSend() {
  IRTcl112Ac tclAc(kIrLedPin);
  tclAc.begin();
  tclAc.setPower(acPower == "on");
  tclAc.setMode(acMode == "cool" ? kTcl112AcCool : kTcl112AcHeat);
  uint8_t fanMode = kTcl112AcFanAuto;
  if (acFan == "auto") {
    fanMode = kTcl112AcFanAuto;
  } else if (acFan == "low") {
    fanMode = kTcl112AcFanLow;
  } else if (acFan == "medium") {
    fanMode = kTcl112AcFanMed;
  } else if (acFan == "high") {
    fanMode = kTcl112AcFanAuto;
  }
  tclAc.setFan(fanMode);
  tclAc.setQuiet(false);
  tclAc.setSwingHorizontal(false);
  tclAc.setSwingVertical(acSwing == "on" ? kTcl112AcSwingVOn
                                         : kTcl112AcSwingVOff);
  tclAc.setTemp(acTemperature);
  tclAc.send();
}
void TechnibelSend() {
  IRTechnibelAc technibelAc(kIrLedPin);
  technibelAc.begin();
  technibelAc.setPower(acPower == "on");
  technibelAc.setMode(acMode == "cool" ? kTechnibelAcCool : kTechnibelAcHeat);
  uint8_t fanMode = kTechnibelAcFanMedium;
  if (acFan == "auto") {
    fanMode = kTechnibelAcFanMedium;
  } else if (acFan == "low") {
    fanMode = kTechnibelAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kTechnibelAcFanMedium;
  } else if (acFan == "high") {
    fanMode = kTechnibelAcFanHigh;
  }
  technibelAc.setFan(fanMode);
  technibelAc.setSwing(acSwing == "on");
  technibelAc.setSleep(false);
  technibelAc.setTemp(acTemperature);
  technibelAc.send();
}
void TecoSend() {
  IRTecoAc tecoAc(kIrLedPin);
  tecoAc.begin();
  tecoAc.setPower(acPower == "on");
  tecoAc.setMode(acMode == "cool" ? kTecoCool : kTecoHeat);
  uint8_t fanMode = kTecoFanAuto;
  if (acFan == "auto") {
    fanMode = kTecoFanAuto;
  } else if (acFan == "low") {
    fanMode = kTecoFanLow;
  } else if (acFan == "medium") {
    fanMode = kTecoFanMed;
  } else if (acFan == "high") {
    fanMode = kTecoFanHigh;
  }
  tecoAc.setFan(fanMode);
  tecoAc.setSleep(false);
  tecoAc.setLight(true);
  tecoAc.setSwing(acSwing == "on");
  tecoAc.setTemp(acTemperature);
  tecoAc.send();
}
void ToshibaSend() {
  IRToshibaAC toshibaAc(kIrLedPin);
  toshibaAc.begin();
  toshibaAc.setPower(acPower == "on");
  toshibaAc.setMode(acMode == "cool" ? kToshibaAcCool : kToshibaAcHeat);
  uint8_t fanMode = kToshibaAcFanAuto;
  if (acFan == "auto") {
    fanMode = kToshibaAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kToshibaAcFanMin;
  } else if (acFan == "medium") {
    fanMode = kToshibaAcFanMed;
  } else if (acFan == "high") {
    fanMode = kToshibaAcFanMax;
  }
  toshibaAc.setFan(fanMode);
  toshibaAc.setSwing(acSwing == "on" ? kToshibaAcSwingOn : kToshibaAcSwingOff);
  toshibaAc.setTemp(acTemperature);
  toshibaAc.send();
}
void TranscoldSend() {
  IRTranscoldAc transcoldAc(kIrLedPin);
  transcoldAc.begin();
  transcoldAc.setPower(acPower == "on");
  transcoldAc.setMode(acMode == "cool" ? kTranscoldCool : kTranscoldHeat);
  uint8_t fanMode = kTranscoldFanAuto;
  if (acFan == "auto") {
    fanMode = kTranscoldFanAuto;
  } else if (acFan == "low") {
    fanMode = kTranscoldFanMin;
  } else if (acFan == "medium") {
    fanMode = kTranscoldFanMed;
  } else if (acFan == "high") {
    fanMode = kTranscoldFanMax;
  }
  transcoldAc.setFan(fanMode);
  // transcoldAc.setSwing(acSwing == "on");
  transcoldAc.setTemp(acTemperature);
  transcoldAc.send();
}
void TrotecSend() {
  IRTrotecESP trotecAc(kIrLedPin);
  trotecAc.begin();
  trotecAc.setPower(acPower == "on");
  trotecAc.setMode(acMode == "cool" ? kTrotecCool : kTrotecAuto);
  uint8_t fanMode = kTrotecFanMed;
  if (acFan == "auto") {
    fanMode = kTrotecFanMed;
  } else if (acFan == "low") {
    fanMode = kTrotecFanLow;
  } else if (acFan == "medium") {
    fanMode = kTrotecFanMed;
  } else if (acFan == "high") {
    fanMode = kTrotecFanHigh;
  }
  trotecAc.setFan(fanMode);
  trotecAc.setTemp(acTemperature);
  trotecAc.send();
}
void TrumaSend() {
  IRTrumaAc trumaAc(kIrLedPin);
  trumaAc.begin();
  trumaAc.setPower(acPower == "on");
  trumaAc.setMode(acMode == "cool" ? kTrumaCool : kTrumaAuto);
  trumaAc.setFan(kTrumaFanMed);
  trumaAc.setTemp(acTemperature);
  trumaAc.send();
}
void VestelSend() {
  IRVestelAc vestelAc(kIrLedPin);
  vestelAc.begin();
  vestelAc.setPower(acPower == "on");
  vestelAc.setMode(acMode == "cool" ? kVestelAcCool : kVestelAcDry);
  uint8_t fanMode = kVestelAcFanAuto;
  if (acFan == "auto") {
    fanMode = kVestelAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kVestelAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kVestelAcFanMed;
  } else if (acFan == "high") {
    fanMode = kVestelAcFanHigh;
  }
  vestelAc.setFan(fanMode);
  vestelAc.setTemp(acTemperature);
  vestelAc.setSwing(acSwing == "on");
  vestelAc.send();
}
void VoltasSend() {
  IRVoltas voltasAc(kIrLedPin);
  voltasAc.begin();
  voltasAc.setPower(acPower == "on");
  voltasAc.setMode(acMode == "cool" ? kVoltasCool : kVoltasHeat);
  uint8_t fanMode = kVoltasFanAuto;
  if (acFan == "auto") {
    fanMode = kVoltasFanAuto;
  } else if (acFan == "low") {
    fanMode = kVoltasFanLow;
  } else if (acFan == "medium") {
    fanMode = kVoltasFanMed;
  } else if (acFan == "high") {
    fanMode = kVoltasFanHigh;
  }
  voltasAc.setFan(fanMode);
  voltasAc.setSwingV(acSwing == "on");
  voltasAc.setTemp(acTemperature);
  voltasAc.send();
}
void WhirlpoolSend() {
  IRWhirlpoolAc whirlpoolAc(kIrLedPin);
  whirlpoolAc.begin();
  whirlpoolAc.setPowerToggle(acPower == "on");
  whirlpoolAc.setMode(acMode == "cool" ? kWhirlpoolAcCool : kWhirlpoolAcHeat);
  uint8_t fanMode = kWhirlpoolAcFanAuto;
  if (acFan == "auto") {
    fanMode = kWhirlpoolAcFanAuto;
  } else if (acFan == "low") {
    fanMode = kWhirlpoolAcFanLow;
  } else if (acFan == "medium") {
    fanMode = kWhirlpoolAcFanMedium;
  } else if (acFan == "high") {
    fanMode = kWhirlpoolAcFanHigh;
  }
  whirlpoolAc.setFan(fanMode);
  whirlpoolAc.setSwing(acSwing == "on");
  whirlpoolAc.setTemp(acTemperature);
  whirlpoolAc.send();
}

void sendAcIrCommand() {
  sendLog("AC Brand: " + acBrand);
  sendLog("AC Power: " + acPower);
  sendLog("AC Mode: " + acMode);
  sendLog("AC Temperature: " + String(acTemperature));

  if (acBrand == "Airton")
    AirtonAcSend();
  else if (acBrand == "Airwell")
    AirwellSend();
  else if (acBrand == "Amcor")
    AmcorSend();
  else if (acBrand == "Argo")
    ArgoSend();
  else if (acBrand == "Bosch")
    BoschSend();
  else if (acBrand == "Carrier")
    CarrierSend();
  else if (acBrand == "Coolix")
    CoolixSend();
  else if (acBrand == "Corona")
    CoronaSend();
  else if (acBrand == "Daikin")
    Daikin128Send();
  else if (acBrand == "Delonghi")
    DelonghiSend();
  else if (acBrand == "Ecoclim")
    EcoclimSend();
  else if (acBrand == "Electra")
    ElectraSend();
  else if (acBrand == "Fujitsu")
    FujitsuSend();
  else if (acBrand == "Goodweather")
    GoodweatherSend();
  else if (acBrand == "Gree")
    GreeSend();
  else if (acBrand == "Haier")
    HaierSend(); // 默认160
  else if (acBrand == "Haier176")
    Haier176Send();
  else if (acBrand == "HaierYRW02")
    HaierYRW02Send();
  else if (acBrand == "Hitachi")
    HitachiSend();
  else if (acBrand == "Kelon")
    KelonSend();
  else if (acBrand == "Kelvinator")
    KelvinatorSend();
  else if (acBrand == "LG")
    LGSend();
  else if (acBrand == "Midea")
    MideaSend();
  else if (acBrand == "Mirage")
    MirageSend();
  else if (acBrand == "Mitsubishi")
    MitsubishiSend();
  else if (acBrand == "Neoclima")
    NeoclimaSend();
  else if (acBrand == "Panasonic")
    PanasonicSend();
  else if (acBrand == "Rhoss")
    RhossSend();
  else if (acBrand == "Samsung")
    SamsungSend();
  else if (acBrand == "Sanyo")
    SanyoSend();
  else if (acBrand == "Sharp")
    SharpSend();
  else if (acBrand == "Tcl")
    TclSend();
  else if (acBrand == "Technibel")
    TechnibelSend();
  else if (acBrand == "Teco")
    TecoSend();
  else if (acBrand == "Toshiba")
    ToshibaSend();
  else if (acBrand == "Transcold")
    TranscoldSend();
  else if (acBrand == "Trotec")
    TrotecSend();
  else if (acBrand == "Truma")
    TrumaSend();
  else if (acBrand == "Vestel")
    VestelSend();
  else if (acBrand == "Voltas")
    VoltasSend();
  else if (acBrand == "Whirlpool")
    WhirlpoolSend();
}

void sendIr() {
  sendAcIrCommand();
  client.publish(tempStateTopic.c_str(), String(acTemperature).c_str(), true);
  client.publish(currentTempTopic.c_str(), String(acTemperature).c_str(), true);
  client.publish(fanModesStateTopic.c_str(), acFan.c_str(), true);
  client.publish(modeStateTopic.c_str(), acPower == "off" ? "off" : "auto",
                 true);
  client.publish(swingModeStateTopic.c_str(), acSwing.c_str(), true);
}

} // namespace

void publishClimateDiscoveryConfig() {
  // 先发布空消息以清除旧配置
  sendLog("清除旧的 Home Assistant 空调发现配置");
  client.publish(climateConfigTopic.c_str(), "", true);
  client.publish(acBrandConfigTopic.c_str(), "", true);
  client.publish(climateModeToggleTextConfigTopic.c_str(), "", true);
  client.publish(climateSwingToggletextConfigTopic.c_str(), "", true);
  client.publish(climateTempUpTextConfigTopic.c_str(), "", true);
  client.publish(climateTempDownTextConfigTopic.c_str(), "", true);
  // 通用的设备信息对象
  JsonDocument deviceInfo;
  JsonArray ids = deviceInfo["identifiers"].to<JsonArray>();
  ids.add("esp8266_" + macStr + "_climate");
  deviceInfo["name"] = "空调";
  deviceInfo["model"] = "mqtt_climate_remote";
  deviceInfo["serial_number"] = macStr;
  deviceInfo["manufacturer"] = "虎哥科技";
  deviceInfo["configuration_url"] = configPageUrl;

  sendLog("注册 Home Assistant 发现配置");
  JsonDocument climateDiscoveryDoc;
  climateDiscoveryDoc["name"] = "空调";
  climateDiscoveryDoc["unique_id"] = macStr + "_climate";
  climateDiscoveryDoc["mode_command_topic"] = ModeCommandTopic;
  climateDiscoveryDoc["mode_state_topic"] = modeStateTopic;
  climateDiscoveryDoc["temperature_command_topic"] = TempCommandTopic;
  climateDiscoveryDoc["current_temperature_topic"] = currentTempTopic;
  climateDiscoveryDoc["temperature_state_topic"] = tempStateTopic;
  climateDiscoveryDoc["swing_mode_command_topic"] = swingModeCommandTopic;
  climateDiscoveryDoc["swing_mode_state_topic"] = swingModeStateTopic;
  climateDiscoveryDoc["min_temp"] = 16;
  climateDiscoveryDoc["max_temp"] = 30;
  climateDiscoveryDoc["temp_step"] = 1;
  climateDiscoveryDoc["fan_mode_command_topic"] = fanModesCommandTopic;
  climateDiscoveryDoc["fan_mode_state_topic"] = fanModesStateTopic;
  climateDiscoveryDoc["fan_modes"] = JsonArray();
  climateDiscoveryDoc["fan_modes"].add("auto");
  climateDiscoveryDoc["fan_modes"].add("low");
  climateDiscoveryDoc["fan_modes"].add("medium");
  climateDiscoveryDoc["fan_modes"].add("high");
  climateDiscoveryDoc["modes"] = JsonArray();
  climateDiscoveryDoc["modes"].add("off");
  climateDiscoveryDoc["modes"].add("auto");
  climateDiscoveryDoc["swing_modes"] = JsonArray();
  climateDiscoveryDoc["swing_modes"].add("off");
  climateDiscoveryDoc["swing_modes"].add("on");
  climateDiscoveryDoc["availability_topic"] = availabilityTopic;
  climateDiscoveryDoc["payload_available"] = "online";
  climateDiscoveryDoc["payload_not_available"] = "offline";
  climateDiscoveryDoc["retain"] = true;
  climateDiscoveryDoc["device"] = deviceInfo;

  String jsonStr;
  serializeJson(climateDiscoveryDoc, jsonStr);
  // 发送当前温度以初始化
  client.publish(tempStateTopic.c_str(), String(acTemperature).c_str(), true);
  client.publish(currentTempTopic.c_str(), String(acTemperature).c_str(), true);
  size_t msgLen = jsonStr.length();

  // 使用 Home Assistant 的自动发现主题

  if (client.beginPublish(climateConfigTopic.c_str(), msgLen, true)) {
    client.print(jsonStr);
    if (client.endPublish()) {
      sendLog("发布空调成功");
    }
  }

  JsonDocument climateBrandSelectDoc;
  climateBrandSelectDoc["name"] = "① 型号";
  climateBrandSelectDoc["unique_id"] = macStr + "_Select";
  climateBrandSelectDoc["command_topic"] = acBrandCommandTopic;
  climateBrandSelectDoc["state_topic"] = acBrandStateTopic;
  climateBrandSelectDoc["options"] = JsonArray();
  deserializeJson(climateBrandSelectDoc["options"],
                  R"([
      "Airton","Airwell","Amcor","Argo","Bosch","Carrier","Coolix","Corona",
      "Daikin","Delonghi","Ecoclim","Electra","Fujitsu","Goodweather","Gree",
      "Haier","Hitachi","Kelon","Kelvinator","LG","Midea","Mirage","Mitsubishi",
      "Neoclima","Panasonic","Rhoss","Samsung","Sanyo","Sharp","Tcl","Technibel",
      "Teco","Toshiba","Transcold","Trotec","Truma","Vestel","Voltas","Whirlpool"
    ])");
  climateBrandSelectDoc["retain"] = true;
  climateBrandSelectDoc["device"] = deviceInfo;

  String jsonTextStr;
  serializeJson(climateBrandSelectDoc, jsonTextStr);
  size_t textLen = jsonTextStr.length();
  // 使用 Home Assistant 的自动发现主题
  if (client.beginPublish(acBrandConfigTopic.c_str(), textLen, true)) {
    client.print(jsonTextStr);
    if (client.endPublish()) {
      sendLog("发布空调型号选择成功");
    }
  }
  JsonDocument climateModeToggleTextDoc;
  climateModeToggleTextDoc["name"] = "② 模式切换（多个用,分割）";
  climateModeToggleTextDoc["unique_id"] = macStr + "_climate_mode_text";
  climateModeToggleTextDoc["command_topic"] = climateModeToggleCommandTopic;
  climateModeToggleTextDoc["state_topic"] = climateModeToggleStateTopic;
  climateModeToggleTextDoc["retain"] = true;
  climateModeToggleTextDoc["device"] = deviceInfo;

  String jsonModdleToggleStr;
  serializeJson(climateModeToggleTextDoc, jsonModdleToggleStr);
  if (client.beginPublish(climateModeToggleTextConfigTopic.c_str(),
                          jsonModdleToggleStr.length(), true)) {
    client.print(jsonModdleToggleStr);
    if (client.endPublish()) {
      sendLog("发布空调模式切换码配置成功");
    }
  }

  // 空调增温发布
  JsonDocument climateTempUpTextDoc;
  climateTempUpTextDoc["name"] = "③ 增温码（多个用,分割）";
  climateTempUpTextDoc["unique_id"] = macStr + "_climate_temp_up_text";
  climateTempUpTextDoc["command_topic"] = climateTempUpCommandTopic;
  climateTempUpTextDoc["state_topic"] = climateTempUpStateTopic;
  climateTempUpTextDoc["retain"] = true;
  climateTempUpTextDoc["device"] = deviceInfo;

  String jsonTempUpStr;
  serializeJson(climateTempUpTextDoc, jsonTempUpStr);
  if (client.beginPublish(climateTempUpTextConfigTopic.c_str(),
                          jsonTempUpStr.length(), true)) {
    client.print(jsonTempUpStr);
    if (client.endPublish()) {
      sendLog("发布空调增温码配置成功");
    }
  }
  // 空调降温发布
  JsonDocument climateTempDownTextDoc;
  climateTempDownTextDoc["name"] = "④ 降温码（多个用,分割）";
  climateTempDownTextDoc["unique_id"] = macStr + "_climate_temp_down_text";
  climateTempDownTextDoc["command_topic"] = climateTempDownCommandTopic;
  climateTempDownTextDoc["state_topic"] = climateTempDownStateTopic;
  climateTempDownTextDoc["retain"] = true;
  climateTempDownTextDoc["device"] = deviceInfo;

  String jsonTempDownStr;
  serializeJson(climateTempDownTextDoc, jsonTempDownStr);
  if (client.beginPublish(climateTempDownTextConfigTopic.c_str(),
                          jsonTempDownStr.length(), true)) {
    client.print(jsonTempDownStr);
    if (client.endPublish()) {
      sendLog("发布空调降温码配置成功");
    }
  }

  // 空调摆风发布
  JsonDocument climateSwingToggleTextDoc;
  climateSwingToggleTextDoc["name"] = "⑤ 摆风码（多个用,分割）";
  climateSwingToggleTextDoc["unique_id"] = macStr + "_climate_swing_text";
  climateSwingToggleTextDoc["command_topic"] = climateSwingToggleCommandTopic;
  climateSwingToggleTextDoc["state_topic"] = climateSwingToggleStateTopic;
  climateSwingToggleTextDoc["retain"] = true;
  climateSwingToggleTextDoc["device"] = deviceInfo;

  String jsonSwingDownStr;
  serializeJson(climateSwingToggleTextDoc, jsonSwingDownStr);
  if (client.beginPublish(climateSwingToggletextConfigTopic.c_str(),
                          jsonSwingDownStr.length(), true)) {
    client.print(jsonSwingDownStr);
    if (client.endPublish()) {
      sendLog("发布空调摆风码配置成功");
    }
  }
}

void subscribeClimateTopics() {
  sendLog("订阅空调控制主题");
  client.subscribe(ModeCommandTopic.c_str());              // 订阅模式设置
  client.subscribe(TempCommandTopic.c_str());              // 订阅温度设置
  client.subscribe(fanModesCommandTopic.c_str());          // 订阅风速设置
  client.subscribe(swingModeCommandTopic.c_str());         // 订阅扫风模式设置
  client.subscribe(acBrandCommandTopic.c_str());           // 订阅型号选择
  client.subscribe(climateModeToggleCommandTopic.c_str()); // 订阅模式切换码变化
  client.subscribe(climateTempUpCommandTopic.c_str());     // 订阅增温码变化
  client.subscribe(climateTempDownCommandTopic.c_str());   // 订阅降温码变化
  client.subscribe(climateSwingToggleCommandTopic.c_str()); // 订阅扫风码变化
}

void handleClimateCommands(String topic, String payload) {
  if (topic == ModeCommandTopic) {
    acPower = (payload == "off") ? "off" : "on"; // 防止字符串错误
    int month = getCurrentMonth();
    acMode = (month >= 5 && month <= 10) ? "cool" : "heat";
    sendIr();
    return;
  }
  if (topic == TempCommandTopic) {
    acTemperature = payload.toInt();
    sendIr();
    return;
  }
  if (topic == fanModesCommandTopic) {
    sendLog("设置风速: " + payload);
    if (payload == "auto" || payload == "low" || payload == "medium" ||
        payload == "high") {
      acFan = payload;
    } else {
      acFan = "auto"; // 默认值
    }
    sendIr();
    return;
  }
  if (topic == swingModeCommandTopic) {
    sendLog("设置扫风模式: " + payload);
    if (payload == "off" || payload == "on") {
      acSwing = payload;
    } else {
      acSwing = "off"; // 默认值
    }
    sendIr();
    return;
  }
  if (topic == acBrandCommandTopic) {
    sendLog("选择空调型号: " + payload);
    acBrand = payload;
    client.publish(acBrandStateTopic.c_str(), payload.c_str(), true);
    return;
  }
  if (topic == climateModeToggleCommandTopic) {
    sendLog("同步空调模式切换码");
    splitCommaSeparated(payload, ToggleCodesArray);
    for (const auto &codeStr : ToggleCodesArray) {
      sendLog(">" + codeStr);
    }
    client.publish(climateModeToggleStateTopic.c_str(), payload.c_str(), true);
    return;
  }
  if (topic == climateTempUpCommandTopic) {
    sendLog("同步空调增温码");
    splitCommaSeparated(payload, TempUpCodesArray);
    sendLog("更新后的 TempUpCodesArray:");
    for (const auto &codeStr : TempUpCodesArray) {
      sendLog(">" + codeStr);
    }
    client.publish(climateTempUpStateTopic.c_str(), payload.c_str(), true);
    return;
  }
  if (topic == climateTempDownCommandTopic) {
    sendLog("同步空调降温码");
    splitCommaSeparated(payload, TempDownCodesArray);
    sendLog("更新后的 TempDownCodesArray:");
    for (const auto &codeStr : TempDownCodesArray) {
      sendLog(">" + codeStr);
    }
    client.publish(climateTempDownStateTopic.c_str(), payload.c_str(), true);
    return;
  }
  if (topic == climateSwingToggleCommandTopic) {
    sendLog("同步空调摆风码");
    splitCommaSeparated(payload, SwingCodesArray);
    sendLog("更新后的 SwingCodesArray:");
    for (const auto &codeStr : SwingCodesArray) {
      sendLog(">" + codeStr);
    }
    client.publish(climateSwingToggleStateTopic.c_str(), payload.c_str(), true);
    return;
  }
}

void registerClimateCode(String code) {
  static unsigned long lastTime = 0;
  constexpr unsigned long CODE_INTERVAL_MS = 400; //  红外发射耗时

  unsigned long now = millis();

  for (const auto &codeStr : ToggleCodesArray) {
    if (codeStr == code) {
      if (now - lastTime > CODE_INTERVAL_MS) {
        sendLog(String(now) + " 收到匹配的模式切换码，切换空调状态");
        acPower = (acPower == "on") ? "off" : "on"; // 切换状态
        if (acPower == "on") {
          // 开机时根据月份设置模式
          int month = getCurrentMonth();
          acMode = (month >= 5 && month <= 10) ? "cool" : "heat";
        }
        sendIr();
      } else {
        sendLog(String(now) + " 连续操作的信号忽略: " + code);
      }
      lastTime = now;
      break;
    }
  }

  for (const auto &codeStr : TempUpCodesArray) {
    if (codeStr == code) {
      if (now - lastTime > CODE_INTERVAL_MS) {
        sendLog(String(now) + " 收到匹配的增温码，增温");
        if (acTemperature < 30) {
          acTemperature++;
          sendIr();
        } else {
          sendLog("温度已达上限30度，无法增温");
        }
      } else {
        sendLog(String(now) + " 连续操作的信号忽略: " + code);
      }
      lastTime = now;
      break;
    }
  }

  for (const auto &codeStr : TempDownCodesArray) {
    if (codeStr == code) {
      if (now - lastTime > CODE_INTERVAL_MS) {
        sendLog(String(now) + " 收到匹配的降温码，降温");
        if (acTemperature > 16) {
          acTemperature--;
          sendIr();
        } else {
          sendLog("温度已达下限16度，无法降温");
        }
      } else {
        sendLog(String(now) + " 连续操作的信号忽略: " + code);
      }
      lastTime = now;
      break;
    }
  }

  for (const auto &codeStr : SwingCodesArray) {
    if (codeStr == code) {
      if (now - lastTime > CODE_INTERVAL_MS) {
        sendLog(String(now) + " 收到匹配的摆风码，切换摆风状态");
        acSwing = (acSwing == "on") ? "off" : "on"; // 切换状态
        sendIr();
      } else {
        sendLog(String(now) + " 连续操作的信号忽略: " + code);
      }
      lastTime = now;
      break;
    }
  }
}