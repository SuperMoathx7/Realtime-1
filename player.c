#include "header.h"

void player_main(int team_id, int player_id) {
    PlayerInfo *player = (team_id == 1) ? &team1[player_id] : &team2[player_id];
    
    // Initialize player state
    srand(time(NULL) ^ (getpid() << 16));
    int energy = rand() % (config.energy_max - config.energy_min) + config.energy_min;
    int decrease_rate = rand() % (config.decrease_max - config.decrease_min) + config.decrease_min;
    
    // For unnamed pipes:
    close(player->pipe_fd[PIPE_READ]); // Close read end for player
    
    // For named pipes:
    // int fd = open(player->fifo_name, O_WRONLY);
    
    while (1) {
        sleep(1);
        energy -= decrease_rate;
        
        // For unnamed pipes:
        write(player->pipe_fd[PIPE_WRITE], &energy, sizeof(energy));
        
        // For named pipes:
        // write(fd, &energy, sizeof(energy));
    }
}