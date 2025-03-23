#ifndef READINPUTS_H
#define READINPUTS_H

#include <stdio.h>

extern FILE *file;
extern int energy_range[2];
extern int decreasing_rate[2];
extern int mul_factor[4];
extern int return_fall_range[2];
extern int threshold_effort_win;
extern int time_to_finish;
extern int win_score;
extern int i;

void readfile();

#endif 
