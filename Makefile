CFLAGS = -Wall -lm -lrt #  gerar "warnings" detalhados e infos de depuração
objs =  ppos_core.o ppos_ipc.o queue.o main.o

# regra default
all: main

main: ppos_core.c ppos_ipc.c queue.c main.c ppos_disk.c hard_disk.c
	gcc -o main main.c queue.c ppos_core.c ppos_disk.c hard_disk.c ppos_disk.c ppos_ipc.c ppos_data.h ppos.h -Wall -lm -lrt

# regra de ligacao
# main: $(objs)
#
# # regras de compilacao
# main.o	   : main.c ppos_core.c ppos_disk.c hard_disk.c ppos_ipc.c queue.c
# ppos_core.o: ppos_core.c ppos.h ppos_data.h
# ppos_disk.o: ppos_disk.c ppos.h ppos_data.h ppos_disk.h
# hard_disk.o: hard_disk.c hard_disk.h ppos_ipc.c queue.c
# ppos_ipc.o : ppos_ipc.c ppos.h ppos_data.h
# queue.o    : queue.c queue.h

# remove arquivos temporarios
clean:
	-rm $(objs) *~

# remove tudo que nao for codigo fonte
purge: clean
	-rm main
