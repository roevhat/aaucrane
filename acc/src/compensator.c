#include <stdio.h>
#include <libcrane.h>

#define NO_RAMP 1

/*RAMP STUFF*/
#define REF_ARR_SZ 8000
size_t nr_of_ref;
double ref_arr[REF_ARR_SZ];
int current_index = 0;

double angle_controller(double angle, int aw){
  double out, k, td, ti;

  static double angle_int  = 0;
  static double angle_pre  = 0;
  static double angle_pre2 = 0;
  static double out_pre    = 0;
  static double out_pre2   = 0;

  /* Diffrent parameters depending of container */
  if (get_ctrlpad_magnet_switch()) {
    k = 10;
    td = 0.02;
    /*Anti-Windup*/
    if (aw) { 
      ti = 0;
    } else if (!aw) {
      ti = 4.15;
    }
  } else {
    k = 10;
    td = 0.02;
    /*Anti-Windup*/
    if (aw) {
      ti = 0;
    } else if (!aw) {
      ti = 4.15;
    }
  }
  
  out = angle * k * ( 1 + (1/ti) * angle_int * X_SAMPLE_TIME_S + (angle_pre-angle)

  out = 1 + (1/ti) * angle_int * X_SAMPLE_TIME_S;
  out = out + ((angle_pre - angle)/X_SAMPLE_TIME_S) * Td;
  out = out * k * angle;

  /*out = 410*angle - 800*angle_pre + 390*angle_pre2 + out_pre2;*/
  
  /*Anti-Windup*/
  if (!aw) {
    angle_int += angle;
  }

  out_pre2 = out_pre;
  out_pre = out;

  angle_pre2 = angle_pre;
  angle_pre = angle;

  return out;
}

double position_controller_x(double error){
  double k_p = 15;
  return error * k_p;
}

double position_controller_y(double error){
  double k_p = 49.3;
  return error * k_p;
}

double pid_get_controller_output(double ref){
  double out;
  static int aw = 0;
  double angle = get_angle();

  out = angle_controller(angle, aw);

  /*STEP or RAMP?*/
  if (NO_RAMP) {
    out += position_controller_x(ref-get_xpos());
  } else {
    out += position_controller_x(ref_arr[current_index]);
  }

  if(current_index < (nr_of_ref - 1)) {
    current_index++;
  }

  if(out >= 12.5 && out <= -12.5) {
    aw = 1;
  } else {
    aw = 0;
  }

  return out;
}

void init_ramp(double x_ref){
  double step;
  x_goal = x_ref;
  step = x_ref-get_xpos();
  nr_of_ref = ramp_maker(step);
  current_index = 0;
}

int ramp_maker(double step){
  double i,  speed = .0005, off_set = get_xpos(); //speed is in m/ms
  int j = 0;

  if(step>0){
    for(i = 0; i<=step; i += speed){
      ref_arr[j] = i + off_set;
      j++;
    }
  } else if(step < 0){
    for(i = 0; i>=step; i -= speed){
      ref_arr[j] = i + off_set;
      j++;
    }
  }

  return j;
}