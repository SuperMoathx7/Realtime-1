#include "readInputs.h"
#include <sys/types.h>
#include <unistd.h>
#include "player.h"

int main()
{
	readfile();
	srand(getpid());
	pid_t childIDS[2];
	int parentID = getpid();
	player this;
	// Generating team1
	for (int i = 0; i < 1; i++)
	{
		if (getpid() == parentID)
			childIDS[i] = fork();
		if (childIDS[i] == 0)
		{ // I'm a child
			this.pid = getpid();
			this.energy = (int)(energy_range[0] + (rand() % (energy_range[1] - energy_range[0])));
			this.decreasingRate = (int)(decreasing_rate[0] + (rand() % (decreasing_rate[1] - decreasing_rate[0])));
			//printf("inside the loop: %d\n", getpid());
			this.team=1;
			this.number=i;
			//this.mulfactor --> will be changed based on its order in the game.
			this.timeToReturn=(int)(return_fall_range[0] + (rand() % (return_fall_range[1] - return_fall_range[0])));
			
		}
	}
	if (getpid() == parentID)
		// Generating team2
		for (int i = 0; i < 1; i++)
		{
			if (getpid() == parentID)
				childIDS[i] = fork();
			if (childIDS[i] == 0)
			{ // I'm a child
				this.pid = getpid();
				this.energy = (int)(energy_range[0] + (rand() % (energy_range[1] - energy_range[0])));
				this.decreasingRate = (int)(decreasing_rate[0] + (rand() % (decreasing_rate[1] - decreasing_rate[0])));
				//printf("inside the loop: %d\n", getpid());
				this.team=2;
				this.number=i;
				//this.mulfactor --> will be changed based on its order in the game.
				this.timeToReturn=(int)(return_fall_range[0] + (rand() % (return_fall_range[1] - return_fall_range[0])));
			
				
			}
		}
	printf("hello from: %did with this energy: %d\n", this.pid, this.energy);

	return 0;
}

