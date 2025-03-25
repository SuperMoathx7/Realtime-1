#include "readInputs.h"

FILE *file;
int energy_range[2];
int decreasing_rate[2];
int mul_factor[4];
int return_fall_range[2];
int threshold_effort_win;
int time_to_finish;
int win_score;
int i;

void readfile() {
    file = fopen("inputs.txt", "r");

    fscanf(file, "%d %d", &energy_range[0], &energy_range[1]);
    fscanf(file, "%d %d", &decreasing_rate[0], &decreasing_rate[1]);

    for (i = 0; i < 4; i++) {
        fscanf(file, "%d", &mul_factor[i]);
    }

    fscanf(file, "%d %d", &return_fall_range[0], &return_fall_range[1]);
    fscanf(file, "%d", &threshold_effort_win);
    fscanf(file, "%d", &time_to_finish);
    fscanf(file, "%d", &win_score);

    fclose(file);

    printf("Win Score: %d\n", win_score);
}

