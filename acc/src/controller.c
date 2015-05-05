#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include <pthread.h>
#include <fcntl.h>
#include <mqueue.h>
#include <rtai_lxrt.h>

#include <libcrane.h>
#include <acc.h>
#include <controller.h>
#include <compensator.h>

RT_TASK *rt_x_axis_controller;
RT_TASK *rt_y_axis_controller;
RT_TASK *rt_logger;

void *task_x_axis_controller(void * argc)
{
  int hit_count = 0;
  int received_new_ref = 0;

  double x_ref = 0;
  double out = 0;

  mqd_t input, output;
  char * input_buffer = (char *)malloc(BUFFER_SIZE);
  input = mq_open(Q_TO_X, O_RDONLY | O_NONBLOCK);
  output = mq_open(Q_FROM_X, O_WRONLY);

  /*Transform to RTAI task*/
  RTIME period = nano2count(SAMPLE_TIME_NS);
  if(!(rt_x_axis_controller = rt_task_init_schmod(nam2num("rt_x_axis_controller"), 1, 0, 0, SCHED_FIFO, 0))){
    printf("Could not start rt_x_axis_controller\n");
    exit(42);
  }
  rt_task_make_periodic(rt_x_axis_controller, rt_get_time() + period, period);
  rt_make_hard_real_time();
  printf("Started rt_x_axis_controller\n");

  printf("Waiting for ref..\n");
  while(mq_receive(input, input_buffer, BUFFER_SIZE, 0) < 0) {
    if (errno != EAGAIN){
      printf("[X]: error %d, %s\n", errno, strerror(errno));
    }
    rt_task_wait_period();
  }

  memcpy(&x_ref, input_buffer, sizeof(double));
  printf("[X]: New x_ref = %.3f\n", x_ref);
  received_new_ref = 1;
  init_ramp(x_ref);

  while (1) {
    /*If new reference in queue*/
    if (mq_receive(input, input_buffer, BUFFER_SIZE, 0) > 0) {
      memcpy(&x_ref, input_buffer, sizeof(double));
      x_ref = (double)((int)(x_ref * 100) / 100.00);
      printf("[X]: New x_ref = %.3f\n", x_ref);
      received_new_ref = 1;
      init_ramp(x_ref);
    }
    /*EAGAIN = No new data available*/
    else if (errno != EAGAIN){
      printf("[X]: error %d, %s\n", errno, strerror(errno));
    }

    out = pid_get_controller_output(x_ref);
    
    /*Settled?*/
    double err = libcrane_truncate(x_ref - get_xpos());
    if ( (fabs(err) <= X_ERR_BAND) /* && (get_motorx_velocity() == 0) */ && (fabs(get_angle()) < ANGLE_ERR_BAND) ) {
      /*Has this happened more than SETTLE_HITS times?*/
      if( received_new_ref && ((++hit_count) >= SETTLE_HITS) ) {
         /*Settled! Allow for new reference and reset hit counter!*/
        received_new_ref = 0;
        hit_count = 0;
        /*Send msg that we have settled!*/
        int msg = 1;
        printf("[X]: DONE @ %lf\n", get_xpos());
        if (mq_send(output, (char *)&msg, sizeof(int), 0) == -1)
          printf("%s\n", strerror(errno));
      }
    } else {
      hit_count = 0;
    }
    
    run_motorx(out);

    rt_task_wait_period();
  }
}

void *task_y_axis_controller(void * argc)
{
  int hit_count = 0;
  int received_new_ref = 0;
  
  double y_ref = 0;
  double out = 0;

  mqd_t input, output;
  char* input_buffer = (char*) malloc(BUFFER_SIZE);
  input = mq_open(Q_TO_Y, O_RDONLY | O_NONBLOCK);
  output = mq_open(Q_FROM_Y, O_WRONLY);

  /*Transform to RTAI task*/
  RTIME period = nano2count(SAMPLE_TIME_NS);
  if(!(rt_y_axis_controller = rt_task_init_schmod(nam2num("rt_y_axis_controller"), 2, 0, 0, SCHED_FIFO, 0))) {
    printf("Could not start rt_y_axis_controller\n");
    exit(42);
  }
  rt_task_make_periodic(rt_y_axis_controller, rt_get_time() + period, period);
  rt_make_hard_real_time();
  printf("Started rt_y_axis_controller\n");

  while(mq_receive(input, input_buffer, BUFFER_SIZE, 0) < 0) {
    if (errno != EAGAIN){
      printf("[Y]: error %d, %s\n", errno, strerror(errno));
    }
    rt_task_wait_period();
  }

  memcpy(&y_ref, input_buffer, sizeof(double));
  printf("[Y]: New y_ref = %.3f\n", y_ref);
  received_new_ref = 1;
  init_ramp(y_ref);

  
  while (1) {
    /*If new reference in queue*/
    if (mq_receive(input, input_buffer, BUFFER_SIZE, 0) > 0) {
      memcpy(&y_ref, input_buffer, sizeof(double));
      printf("[Y]: New y_ref = %.3f\n", y_ref);
      received_new_ref = 1;
    }
    /*EAGAIN = No new data available*/
    else if (errno != EAGAIN) {
      printf("[Y]: error %d, %s\n", errno, strerror(errno));
    }

    /*Settled?*/
    double err = libcrane_truncate(y_ref - get_ypos());
    if ( (fabs(err) < Y_ERR_BAND) ) {
      if( received_new_ref && ((++hit_count) >= SETTLE_HITS) ) {
        /*Settled! Allow for new reference and reset hit counter!*/
        received_new_ref = 0;
        hit_count = 0;
        /*Send msg that we have settled!*/
        int msg = 1;
        printf("[Y]: DONE @ %lf\n", get_ypos());
        if (mq_send(output, (char *)&msg, sizeof(int), 0) == -1)
          printf("%s\n", strerror(errno));
      }
    } else {
      hit_count = 0;
    }

    out = position_controller_y(err);

    run_motory(out);
    rt_task_wait_period();
  }
}

void* task_logger(void* args){
  FILE* fp;
  unsigned long t_0, t_sample;
  uint32_t name_len = 256;
  char data_path = "/var/www/html/data/acc/steps/";
  char header = "TIME,ANGLE1,ANGLE2,XPOS,YPOS,XTACHO,YTACHO,XVOLT,YVOLT\n";

  RTIME period = nano2count(SAMPLE_TIME_NS); 
  if(!(rt_logger = rt_task_init_schmod(nam2num("logger"), 1, 0, 0, SCHED_FIFO, 0))){
    printf("Could not start logger task\n");
    exit(42);
  }
  rt_task_make_periodic(rt_logger, rt_get_time() + period, period);
  rt_make_hard_real_time();

  char tmp[name_len];
  sprintf(tmp, "%s/%d.csv", data_path, (int)time(NULL));
  fp = fopen(tmp, "w");
  fprintf(fp, header);

  t_0 = get_time_micros();
  while(anable_logger){
    /*GRAB TIMESTAMP*/
    t_sample = get_time_micros();
    fprintf(fp, "%ld,",  (t_sample - t_0));

    /*SAMPLE SENSORS*/
    fprintf(fp, "%f,", get_old_angle_raw());
    fprintf(fp, "%f,", get_angle());
    fprintf(fp, "%f,", get_xpos());
    fprintf(fp, "%f,", get_ypos());
    fprintf(fp, "%f,", get_motorx_velocity());
    fprintf(fp, "%f,", get_motory_velocity());
    fprintf(fp, "%f,", get_motorx_voltage());
    fprintf(fp, "%f",  get_motory_voltage());
    fprintf(fp, "\n");

    rt_task_wait_period();
  }
}

int init_logger(){
  static bool enable_logger;
  static bool new_log = true;
  rt_typed_sem_init(logger_sem, 1, BIN_SEM | FIFO_Q );
  
}

int disabel_logger(){
  int ret = 0;
  RTIME delay =  nano2count(1000);
  if (rt_sem_wait_timed(&loggger_sem, delay) == 0xFFFF){
    ret = -1;
  }
  else{
    enable_logger = false;
    if (rt_sem_signal(&logger_sem) == 0xFFFF){
      ret = -1;
    }
  }
  return ret;
}

int enable_logger(){
  int ret = 0;
  RTIME delay =  nano2count(1000);
  if (rt_sem_wait_timed(&loggger_sem, delay) == 0xFFFF){
    ret = -1;
  }
  else{
    enable_logger = true;
    new_log = true;
    if (rt_sem_signal(&logger_sem) == 0xFFFF){
      ret = -1;
    }
  }
  return ret;
}
