#include "readInputs.h"
#include <sys/types.h>
#include <unistd.h>


struct player{
	int pid;
	int team;
	int number;
	int energy;
	int decreasingRate;
	int mulFactor;
	int timeToReturn;
};

int main(){
	readfile();
	srand(getpid());
	pid_t childIDS[2];
	int parentID = getpid();
	for(int i=0;i<2;i++){
		if(getpid()==parentID)
			childIDS[i] = fork();
		if(childIDS[i] ==0){//I'm a child
			struct player this;
			this.pid = getpid();
			this.energy=(int)(energy_range[0] + (rand()%(energy_range[1] - energy_range[0])));
			this.decreasingRate=(int)(decreasing_rate[0] + (rand()%(decreasing_rate[1] - decreasing_rate[0])));	

		}	
	}
	printf("hello from: %d\n",getpid());

return 0;
}
