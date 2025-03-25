#include "header.h"

Config config;
PlayerInfo team1[MAX_PLAYERS];
PlayerInfo team2[MAX_PLAYERS];
int score_team1 = 0;
int score_team2 = 0;
float rope_offset = 0.0f;
int current_round = 1;
int time_remaining = 0;
bool game_over = false;

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
    for(int i = 0; i < config.team_players; i++) {
        pipe(team1[i].pipe_fd);
        pipe(team2[i].pipe_fd);
    }
}

void sort_players(PlayerInfo *team) {
    for(int i = 0; i < config.team_players; i++) {
        for(int j = i+1; j < config.team_players; j++) {
            if(team[i].energy < team[j].energy) {
                PlayerInfo temp = team[i];
                team[i] = team[j];
                team[j] = temp;
            }
        }
    }
}

void reset_round() {
    for(int i = 0; i < config.team_players; i++) {
        kill(team1[i].pid, SIGUSR1);
        kill(team2[i].pid, SIGUSR1);
    }
    time_remaining = config.game_duration;
    current_round++;
    rope_offset = 0.0f;
}


void update_game_state() {
    int effort1 = 0, effort2 = 0;
    time_remaining--;
    
    for(int i = 0; i < config.team_players; i++) {
        read(team1[i].pipe_fd[PIPE_READ], &team1[i].energy, sizeof(int));
        read(team2[i].pipe_fd[PIPE_READ], &team2[i].energy, sizeof(int));
    }

    sort_players(team1);
    sort_players(team2);

    for(int i = 0; i < config.team_players; i++) {
        effort1 += team1[i].energy * (i+1);
        effort2 += team2[i].energy * (i+1);
    }

    rope_offset = (effort1 - effort2) / 1000.0f;
    if(rope_offset > 0.4f) rope_offset = 0.4f;
    if(rope_offset < -0.4f) rope_offset = -0.4f;

    if(time_remaining <= 0 || abs(effort1 - effort2) > config.threshold) {
        if(effort1 > effort2) score_team1++;
        else score_team2++;
        
        if(score_team1 >= config.score_limit || score_team2 >= config.score_limit) {
            game_over = true;
        }
        else {
            reset_round();
        }
    }
}

void spawn_players() {
    for(int i = 0; i < config.team_players; i++) {
        pid_t pid = fork();
        if(pid == 0) {
            close(team1[i].pipe_fd[PIPE_READ]);
            srand(time(NULL) ^ getpid());
            int energy = rand() % (config.energy_max - config.energy_min) + config.energy_min;
            int decrease = rand() % (config.decrease_max - config.decrease_min) + config.decrease_min;
            
            while(1) {
                usleep(100000);
                if(team1[i].recovery_time > 0) {
                    team1[i].recovery_time--;
                    if(team1[i].recovery_time == 0) {
                        energy = rand() % (config.energy_max - config.energy_min) + config.energy_min;
                    }
                    else {
                        energy = 0;
                    }
                }
                else if(rand() % 100 < 5) { // 5% chance to fall
                    team1[i].recovery_time = rand() % (config.recovery_max - config.recovery_min) + config.recovery_min;
                    energy = 0;
                }
                else {
                    energy -= decrease;
                    if(energy < 0) energy = 0;
                }
                write(team1[i].pipe_fd[PIPE_WRITE], &energy, sizeof(int));
            }
            exit(0);
        }
    }

    for(int i = 0; i < config.team_players; i++) {
        pid_t pid = fork();
        if(pid == 0) {
            close(team2[i].pipe_fd[PIPE_READ]);
            srand(time(NULL) ^ getpid());
            int energy = rand() % (config.energy_max - config.energy_min) + config.energy_min;
            int decrease = rand() % (config.decrease_max - config.decrease_min) + config.decrease_min;
            
            while(1) {
                usleep(100000);
                if(team2[i].recovery_time > 0) {
                    team2[i].recovery_time--;
                    if(team2[i].recovery_time == 0) {
                        energy = rand() % (config.energy_max - config.energy_min) + config.energy_min;
                    }
                    else {
                        energy = 0;
                    }
                }
                else if(rand() % 100 < 5) { // 5% chance to fall
                    team2[i].recovery_time = rand() % (config.recovery_max - config.recovery_min) + config.recovery_min;
                    energy = 0;
                }
                else {
                    energy -= decrease;
                    if(energy < 0) energy = 0;
                }
                write(team2[i].pipe_fd[PIPE_WRITE], &energy, sizeof(int));
            }
            exit(0);
        }
    }
}

int main(int argc, char **argv) {
    parse_config("config.txt");
    create_pipes();
    spawn_players();
    time_remaining = config.game_duration;
    init_graphics(argc, argv);
    return 0;
}