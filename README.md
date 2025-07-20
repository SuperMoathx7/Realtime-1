# Real-Time Tug-of-War Simulation 🏆

A real-time multi-process tug-of-war game implemented in C using OpenGL for visualization and Unix IPC mechanisms for inter-process communication. This project demonstrates advanced concepts in operating systems, real-time systems, and graphics programming.


## 📋 Table of Contents

- [🚀 Features](#-features)
- [🎬 Sample Run Video](#-sample-run-video)
- [🏗️ Architecture Overview](#️-architecture-overview)
- [🔧 Implementation Details](#-implementation-details)
- [📡 Communication Techniques](#-communication-techniques)
- [💻 Installation & Setup](#-installation--setup)
- [🎮 Usage](#-usage)
- [⚙️ Configuration](#️-configuration)
- [🏆 Game Rules](#-game-rules)
- [📊 Technical Specifications](#-technical-specifications)
- [👥 Contributors](#-contributors)

## 🚀 Features

- **Real-Time Simulation**: Multi-process architecture with real-time player behavior
- **OpenGL Visualization**: Smooth graphics with player positioning and movement
- **Inter-Process Communication**: Advanced IPC using pipes and Unix signals
- **Configurable Parameters**: Customizable game settings through configuration file
- **Dynamic Player Behavior**: Energy-based movement with decay and recovery mechanics
- **Round-Based Gameplay**: Multiple rounds with win conditions and streak tracking
- **Falling Mechanics**: Probabilistic player falling during intense moments
- **Team Strategy**: Position-based multipliers favoring strategic player placement

## 🎬 Sample Run Video

https://github.com/user-attachments/assets/90d6f4f9-0265-4867-80f0-9affed165215

> 📽️ The video shows players aligning at the start of each round, followed by the intense pulling phase where energy levels determine team success.


## 🏗️ Architecture Overview

The project follows a **multi-process architecture** where:

- **Parent Process (Referee)**: Manages game state, visualization, and coordination
- **Child Processes (Players)**: Independent player entities with autonomous behavior
- **Real-Time Communication**: Continuous data exchange between processes
- **Event-Driven Design**: Signal-based phase transitions and game events

### Process Hierarchy
```
Main Process (Referee)
├── Player 1 (Team 1, Member 1)
├── Player 2 (Team 1, Member 2)
├── Player 3 (Team 1, Member 3)
├── Player 4 (Team 1, Member 4)
├── Player 5 (Team 2, Member 1)
├── Player 6 (Team 2, Member 2)
├── Player 7 (Team 2, Member 3)
└── Player 8 (Team 2, Member 4)
```

## 🔧 Implementation Details

### Core Components

#### 1. **Main Process (`main.c`)**
- Initializes the game environment
- Creates child processes for each player
- Sets up IPC mechanisms (pipes and signals)
- Launches OpenGL visualization

#### 2. **Player Process (`player.c`)**
- Autonomous player behavior simulation
- Energy management and decay calculations
- Signal handling for game phase transitions
- Probabilistic falling mechanics (5% chance per iteration)

#### 3. **Referee System (`referee.c`)**
- Game state management and phase coordination
- Player alignment and positioning algorithms
- Score calculation and win condition checking
- Real-time game updates and visualization triggers

#### 4. **Visualization Engine (`gui.c`)**
- OpenGL-based real-time rendering
- Player position interpolation for smooth movement
- Game state display (scores, timer, round information)
- Dynamic visual feedback for game events

#### 5. **Signal Management (`signalsHand.c`)**
- Custom signal handlers for game phases
- Inter-process synchronization
- Phase transition management

### Data Structures

#### Player Information Structure
```c
typedef struct {
    int team;                    // Team number (1 or 2)
    int member;                  // Member number within team
    int energy, decay, returnAfter; // Player attributes
    int effort;                  // Calculated effort value
    float x, y;                  // Current display position
    float targetX, targetY;      // Target positions for movement
    float radius;                // Visual representation radius
    pid_t pid;                   // Process identifier
} PlayerInfo;
```

#### Communication Message Structure
```c
typedef struct {
    pid_t pid;                   // Sender process ID
    int team;                    // Team identifier
    int member;                  // Member identifier
    int effort;                  // Calculated effort (energy/decay_rate)
    int energy, decay, returnAfter; // Current player state
    int type;                    // Message type (0=periodic, 1=alignment, 2=pulling)
} EffortMsg;
```

## 📡 Communication Techniques

### 1. **Unix Pipes**
- **Anonymous Pipes**: Primary communication channel between parent and child processes
- **Non-blocking I/O**: Prevents deadlocks and ensures real-time responsiveness
- **Message-based Protocol**: Structured data exchange using `EffortMsg` format

```c
// Pipe creation and configuration
if (pipe(pipefd) == -1) {
    perror("pipe creation failed");
    exit(EXIT_FAILURE);
}
pipe_read_fd = pipefd[0]; // Parent reads from this end

// Non-blocking mode setup
int flags = fcntl(pipe_read_fd, F_GETFL, 0);
fcntl(pipe_read_fd, F_SETFL, flags | O_NONBLOCK);
```

### 2. **Unix Signals**
- **SIGUSR1**: Triggers player alignment phase
- **SIGUSR2**: Initiates pulling phase
- **SIGPWR**: Signals new round beginning
- **Custom Signal Handlers**: Phase-specific behavior implementation

```c
// Signal handler registration
sigset(SIGUSR1, alignment_handler);     // Alignment phase
sigset(SIGUSR2, pulling_handler);       // Pulling phase
sigset(SIGPWR, newrnd);                // New round
```

### 3. **Shared State Management**
- **Process-local State**: Each player maintains independent state
- **Centralized Coordination**: Referee process coordinates global game state
- **Event-driven Updates**: State changes trigger appropriate responses

### 4. **Real-time Synchronization**
- **Timer-based Coordination**: Precise timing for game phases
- **Asynchronous Message Processing**: Non-blocking communication patterns
- **Phase Synchronization**: Ensures all players transition phases together

## 💻 Installation & Setup

### Prerequisites
- **GCC Compiler**: C compiler with C99 support
- **OpenGL Libraries**: GLUT, GL, and GLU
- **Unix/Linux Environment**: POSIX-compliant system
- **Make Utility**: For build automation

### Dependencies Installation

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev
```

#### CentOS/RHEL:
```bash
sudo yum groupinstall "Development Tools"
sudo yum install freeglut-devel mesa-libGL-devel mesa-libGLU-devel
```

### Building the Project
```bash
# Clone the repository
git clone https://github.com/SuperMoathx7/Real-Time-Tug-of-war-Simulator.git
cd Real-Time-Tug-of-war-Simulator

# Build and run
make run
```

## 🎮 Usage

### Running the Game
```bash
make run
```

### Game Controls
- **Automatic Gameplay**: The simulation runs autonomously
- **Window Close**: Terminates all processes and exits gracefully
- **Real-time Monitoring**: Observe player movements and energy levels

### Output Monitoring
The game provides real-time console output including:
- Player creation and initialization
- Phase transitions (alignment → pulling)
- Score updates and round progression
- Win conditions and final results

## ⚙️ Configuration

The game behavior is customizable through the `inputs.txt` configuration file:

```
energy_min=75          # Minimum initial player energy
energy_max=100         # Maximum initial player energy
decay_min=3            # Minimum energy decay rate
decay_max=5            # Maximum energy decay rate
recovery_min=1         # Minimum recovery time
recovery_max=3         # Maximum recovery time
threshold=900          # Effort threshold for calculations
game_duration=80       # Maximum game duration (seconds)
score_limit=2          # Consecutive wins needed for victory
total_rounds=5         # Total number of rounds
```

### Parameter Validation
- Energy values must be positive integers
- Minimum values must be less than maximum values
- All parameters undergo validation during startup

## 🏆 Game Rules

### Team Composition
- **2 Teams**: Each with 4 players
- **Strategic Positioning**: Players closer to rope have higher multipliers
- **Energy-based Performance**: Player effectiveness depends on current energy

### Winning Conditions
1. **Midpoint Rule**: If any player crosses the center line, their team loses the round
2. **Consecutive Wins**: First team to win 2 consecutive rounds wins the game
3. **Round Limit**: Game ends after specified number of rounds
4. **Time Limit**: Game ends when duration expires
5. **Exhaustion**: If all players reach zero energy, the round is declared a tie

### Effort Calculation
- **Individual Effort**: `energy / decay_rate`
- **Position Multipliers**: 1x, 2x, 3x, 4x (closest to farthest from rope)
- **Team Effort**: Sum of all multiplied individual efforts
- **Movement**: Based on effort differential between teams

### Player Mechanics
- **Energy Decay**: Gradual energy loss during pulling phase
- **Recovery**: Players regenerate energy after falling
- **Falling**: 5% probability per iteration during intense pulling
- **Positioning**: Strategic alignment based on current energy levels

## 📊 Technical Specifications

### Performance Characteristics
- **Process Count**: 9 processes (1 referee + 8 players)
- **Update Frequency**: 1Hz for player updates, 60Hz for visualization
- **Memory Usage**: Minimal shared state, process-local data
- **Communication Overhead**: Efficient message-based IPC

### System Requirements
- **CPU**: Multi-core processor recommended
- **Memory**: 64MB RAM minimum
- **Graphics**: OpenGL 2.1 compatible
- **OS**: Linux/Unix with POSIX support

### Scalability
- **Configurable Team Size**: Adjustable via `MEMBERS_PER_TEAM` macro
- **Dynamic Process Creation**: Automatic scaling based on team configuration
- **Modular Architecture**: Easy extension for additional features

## 👥 Contributors

<table align="center">
<tr>
<td align="center">
<a href="https://github.com/SuperMoathx7">
<img src="https://github.com/SuperMoathx7.png" width="130px;" alt="Contributor 1"/>
<br />
<b style="font-size: 16px;">SuperMoathx7</b>
</a>
<br />
<span title="Core Architecture">🏗️</span>
</td>
<td align="center">
<a href="https://github.com/osaidnur">
<img src="https://github.com/osaidnur.png" width="130px;" alt="Contributor 2"/>
<br />
<b style="font-size: 16px;">Osaid Nur</b>
</a>
<br />
<span title="Visualization">📊</span>
</td>
<td align="center">
<a href="https://github.com/3ahma">
<img src="https://github.com/3ahma.png" width="130px;" alt="Contributor 3"/>
<br />
<b style="font-size: 16px;">Ahmad Hussin</b>
</a>
<br />
<span title="Gang Implementation">👥</span>
</td>
<td align="center">
<a href="https://github.com/MoaidKarakra">
<img src="https://github.com/MoaidKarakra.png" width="130px;" alt="Contributor 4"/>
<br />
<b style="font-size: 16px;">Moaid Karakra</b>
</a>
<br />
<span title="Police & Intelligence">🚔</span>
</td>
</tr>
</table>

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

