#include "header.h"

void handle_signal(int sig) {
    static bool pulling = false;
    
    if (sig == SIGUSR1) {
        // Alignment logic
        pulling = false;
    } else if (sig == SIGUSR2) {
        // Start pulling
        pulling = true;
    }
}

void player_main(int team_id, int player_id) {
    // Initialize player state
    srand(time(NULL) ^ (getpid() << 16));
    int energy = rand() % (config.energy_max - config.energy_min) + config.energy_min;
    int decrease_rate = rand() % (config.decrease_max - config.decrease_min) + config.decrease_min;
    
    signal(SIGUSR1, handle_signal);
    signal(SIGUSR2, handle_signal);

    // Get appropriate pipes
    int *pipe_out = team_id == 1 ? pipes_team1[player_id] : pipes_team2[player_id];
    
    while (1) {
        sleep(1);
        energy -= decrease_rate;
        write(pipe_out[PIPE_WRITE], &energy, sizeof(energy));
    }
}