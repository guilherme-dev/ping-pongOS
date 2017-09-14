CFLAGS = -Wall -g  # gerar "warnings" detalhados e infos de depuração

all: ppos_core.c pingpong-tasks1.c
	gcc -Wall -o teste -DDEBUG ppos_core.c pingpong-tasks1.c
tasks2: ppos_core.c pingpong-tasks2.c
	gcc -Wall -o teste -DDEBUG ppos_core.c pingpong-tasks2.c
tasks3: ppos_core.c pingpong-tasks3.c
	gcc -Wall -o teste -DDEBUG ppos_core.c pingpong-tasks3.c

# remove arquivos temporarios
clean:
	-rm *.o