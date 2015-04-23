#include<stdio.h>
#include<libcrane.h>

#include "../include/filter.h"
const static double TS = 0.001;

double angle_integrate = 0, angle_prev = 0;


double angle_controller(double angle_pos){
  double angle_pos = 0, out, TS = .001, windup_val = 12.5, angle_windup_val = 1, k = 38, tp = .1, td = .5, ti = 4;

  out = angle_pos*(k*tp + k * ti * (angle_integrate) * TS + k) + k * td * (angle_prev-angle_pos);

  angle_integrate += angle_pos;

  if(angle_integrate < -angle_windup_val) angle_integrate = -angle_windup_val;
  else if(angle_integrate > angle_windup_val) angle_integrate = angle_windup_val;

  angle_prev = angle_pos;

  return out;
}

double position_controller_x(double error){
  double k_p = 30;
  return error * k_p;
}

double position_controller_y(double error){
  double k_p = 49.3;
  return error * k_p;
}

double velocity_controller_x(double error){
  double k_p = 50;
  return k_p * error;
}

int ramp_maker(double step, double *ramp_arr){
  double i,  speed = .0005, off_set = get_xpos(); //speed is in m/ms
  int j = 0;

  if(step>0){
    for(i = 0; i<step; i += speed){
      ramp_arr[j] = i + off_set;
      j++;
    }
  } else if(step < 0){
    for(i = 0; i>step; i -= speed){
      ramp_arr[j] = i + off_set;
      j++;
    }
  }

  return j;

}
