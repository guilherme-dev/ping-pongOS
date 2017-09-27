CFLAGS = -Wall -g  # gerar "warnings" detalhados e infos de depuração
objs = ppos_core.o queue.o

# regra default
all: main

# regra de ligacao
main: $(objs)

# regras de compilacao
main.o:      main.c
ppos_core.o: ppos_core.c ppos.h ppos_data.h
queue.o:     queue.c queue.h

# remove arquivos temporarios
clean:
	-rm $(objs) *~

# remove tudo que nao for codigo fonte
purge: clean
	-rm main
