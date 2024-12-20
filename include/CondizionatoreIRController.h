#ifndef _CONDIZIONATOREIRCONTROLLER_H_
#define _CONDIZIONATOREIRCONTROLLER_H_

#include <SinricProDevice.h>
#include <Capabilities/PowerStateController.h>
#include <Capabilities/RangeController.h>
#include <Capabilities/ModeController.h>

class CondizionatoreIRController 
: public SinricProDevice
, public PowerStateController<CondizionatoreIRController>
, public RangeController<CondizionatoreIRController>
, public ModeController<CondizionatoreIRController> {
  friend class PowerStateController<CondizionatoreIRController>;
  friend class RangeController<CondizionatoreIRController>;
  friend class ModeController<CondizionatoreIRController>;
public:
  CondizionatoreIRController(const String &deviceId) : SinricProDevice(deviceId, "CondizionatoreIRController") {};
};

#endif
