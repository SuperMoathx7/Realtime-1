#include "headers.h"

// Read the configuration file and set the parameters
void readFile(const char *filename) {
    FILE *file = fopen(filename, "r");
if (!file) {
    perror("Failed to open file");
    exit(EXIT_FAILURE);
}
//********************** */
    if(fscanf(file, "energy_min=%d\n", &INIT_ENERGY_MIN) !=1){
        printf("enegy_min is non-integer! Retry again.\n");
        exit(10);
    }
    if(fscanf(file, "energy_max=%d\n", &INIT_ENERGY_MAX) !=1){
        printf("enegy_max is non-integer! Retry again.\n");
        exit(10);
    }

    if(INIT_ENERGY_MIN > INIT_ENERGY_MAX){
        printf("Energy_min must be smaller than Energy_max! Retry again.\n");
        exit(5);
    }
    else if(INIT_ENERGY_MAX == INIT_ENERGY_MIN){
        printf("Energy_min must differ from Energy_max! Retry again.\n");
        exit(5);
    }
    else if(INIT_ENERGY_MIN < 0 || INIT_ENERGY_MAX < 0){
        printf("Energies must be positive! Retry again.\n");
        exit(5);
    }
//********************** */
    if(fscanf(file, "decay_min=%d\n", &DECAY_RATE_MIN) !=1){
        printf("decay_min is non-integer! Retry again.\n");
        exit(10);
    }
    if(fscanf(file, "decay_max=%d\n", &DECAY_RATE_MAX) !=1){
        printf("decay_max is non-integer! Retry again.\n");
        exit(10);
    }
    if(DECAY_RATE_MIN > DECAY_RATE_MAX){
        printf("Decay_min must be smaller than Decay_max! Retry again.\n");
        exit(6);
    }
    else if(DECAY_RATE_MAX == DECAY_RATE_MIN){
        printf("Decay_min must differ from Decay_max! Retry again.\n");
        exit(6);
    }
    else if(DECAY_RATE_MIN < 0 || DECAY_RATE_MAX < 0){
        printf("Decay time must be positive! Retry again.\n");
        exit(6);
    }
//********************** */
    if(fscanf(file, "recovery_min=%d\n", &RECOVERY_TIME_MIN) !=1){
        printf("recovery_min is non-integer! Retry again.\n");
        exit(10);
    }
    if(fscanf(file, "recovery_max=%d\n", &RECOVERY_TIME_MAX) !=1){
        printf("recovery_max is non-integer! Retry again.\n");
        exit(10);
    }
    if(RECOVERY_TIME_MIN > RECOVERY_TIME_MAX){
        printf("Recovery_min must be smaller than Recovery_max! Retry again.\n");
        exit(7);
    }
    else if(RECOVERY_TIME_MAX == RECOVERY_TIME_MIN){
        printf("Recovery_min must differ from Recovery_max! Retry again.\n");
        exit(7);
    }
    else if(RECOVERY_TIME_MIN < 0 || RECOVERY_TIME_MAX < 0){
        printf("Recovery time must be positive! Retry again.\n");
        exit(7);
    }
    //********************** */
    if(fscanf(file, "threshold=%d\n", &THRESHOLD_TO_WIN) !=1){
        printf("threshold is non-integer! Retry again.\n");
        exit(10);
    }
    if(THRESHOLD_TO_WIN < 0 ){
        printf("Threshold to win must be positive! Retry again.\n");
        exit(8);
    }
    //********************** */
    if(fscanf(file, "game_duration=%d\n", &COPY_GAME_DURATION) !=1){
        printf("game_duration is non-integer! Retry again.\n");
        exit(10);
    }
    if(COPY_GAME_DURATION < 0 ){
        printf("GAME_DURATION must be positive! Retry again.\n");
        exit(8);
    }
    //********************** */
    if(fscanf(file, "score_limit=%d\n", &STREAK_TO_WIN) !=1){
        printf("score_limit is non-integer! Retry again.\n");
        exit(10);
    }
    if(STREAK_TO_WIN < 0 ){
        printf("STREAK_TO_WIN must be positive! Retry again.\n");
        exit(8);
    }
    //********************** */
    if(fscanf(file, "total_rounds=%d\n", &total_rounds) !=1){
        printf("total_rounds is non-integer! Retry again.\n");
        exit(10);
    }
    if(total_rounds < 0 ){
        printf("total_rounds must be positive! Retry again.\n");
        exit(8);
    }
    //********************** */
    GAME_DURATION = COPY_GAME_DURATION;
    fclose(file);
}