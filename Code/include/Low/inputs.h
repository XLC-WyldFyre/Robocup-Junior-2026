#pragma once
#include "world.h"

//namespace World { struct In; }   // iba dopredu


float input_wrap180(float a);  //normalize from -180 to 180
float wrap360(float a);
float deg2rad(float d);
float rad2deg(float r);


namespace Pheripherials {
  void inputs_init_();
  void inputs_update();
}