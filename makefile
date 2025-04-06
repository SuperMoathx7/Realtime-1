run:
	# pkill -f ./habedtime2
	gcc gui.c player.c referee.c  signalsHand.c global.c readFile.c  main.c -o main -lglut -lGL -lGLU -lm -Wall
	./main