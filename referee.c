#include "header.h"

Config config;
PlayerInfo *team1 = NULL;
PlayerInfo *team2 = NULL;
int **pipes_team1 = NULL;
int **pipes_team2 = NULL;

void parse_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    fscanf(file, "team_players=%d\n", &config.team_players);
    fscanf(file, "energy_min=%d\n", &config.energy_min);
    fscanf(file, "energy_max=%d\n", &config.energy_max);
    fscanf(file, "decrease_min=%d\n", &config.decrease_min);
    fscanf(file, "decrease_max=%d\n", &config.decrease_max);
    fscanf(file, "recovery_min=%d\n", &config.recovery_min);
    fscanf(file, "recovery_max=%d\n", &config.recovery_max);
    fscanf(file, "threshold=%d\n", &config.threshold);
    fscanf(file, "game_duration=%d\n", &config.game_duration);
    fscanf(file, "score_limit=%d\n", &config.score_limit);
    fclose(file);
}

void create_pipes() {
    pipes_team1 = malloc(config.team_players * sizeof(int *));
    pipes_team2 = malloc(config.team_players * sizeof(int *));
    
    for (int i = 0; i < config.team_players; i++) {
        pipes_team1[i] = malloc(2 * sizeof(int));
        pipes_team2[i] = malloc(2 * sizeof(int));
        pipe(pipes_team1[i]);
        pipe(pipes_team2[i]);
    }
}

void spawn_players() {
    team1 = malloc(config.team_players * sizeof(PlayerInfo));
    team2 = malloc(config.team_players * sizeof(PlayerInfo));
    
    for (int i = 0; i < config.team_players; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            player_main(1, i);
            exit(0);
        }
        team1[i].pid = pid;
        team1[i].active = true;
    }
    
    for (int i = 0; i < config.team_players; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            player_main(2, i);
            exit(0);
        }
        team2[i].pid = pid;
        team2[i].active = true;
    }
}

void send_signal_to_team(PlayerInfo *team, int sig) {
    for (int i = 0; i < config.team_players; i++) {
        if (team[i].active) {
            kill(team[i].pid, sig);
        }
    }
}

int main(int argc, char *argv[]) {
    parse_config("config.txt");
    create_pipes();
    spawn_players();
    
    // Game initialization
    init_graphics(&argc, argv);
    
    // Game loop
    while (1) {
        // Alignment phase
        send_signal_to_team(team1, SIGUSR1);
        send_signal_to_team(team2, SIGUSR1);
        sleep(1);
        
        // Pulling phase
        send_signal_to_team(team1, SIGUSR2);
        send_signal_to_team(team2, SIGUSR2);
        
        // Effort calculation
        int effort1 = 0, effort2 = 0;
        for (int i = 0; i < config.team_players; i++) {
            int e;
            read(pipes_team1[i][PIPE_READ], &e, sizeof(e));
            effort1 += e * (i + 1);
            read(pipes_team2[i][PIPE_READ], &e, sizeof(e));
            effort2 += e * (i + 1);
        }
        
        update_display(effort1, effort2);
        sleep(1);
    }
    
    cleanup_resources();
    return 0;
}