CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lglut -lGLU -lGL

SRC = referee.c player.c gui.c
OBJ = $(SRC:.c=.o)
EXEC = rope_game

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(EXEC)


