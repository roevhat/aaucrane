#ifndef _CONTROLLER_H_
#define _CONTROLLER_H

#define X_ERR_BAND 0.007
#define Y_ERR_BAND 0.005
#define ANGLE_ERR_BAND 0.03
#define SETTLE_HITS 250

/* RTAI tasks */
extern RT_TASK *rt_x_axis_controller;
extern RT_TASK *rt_y_axis_controller;

/* Logger task - enable and disable flag */
extern RT_TASK *rt_logger;
extern static SEM logger_sem;
extern static int enable_logger;
extern static int new_log;

void* task_x_axis_controller(void *);
void* task_y_axis_controller(void *);
void* task_logger(void *);

int init_logger();

int enable_logger();
int disable_logger();

#endif
