CFLAGS = -Wall -g  # gerar "warnings" detalhados e infos de depuração
objs = testafila.o queue.o

# regra default
all: testafila

# regra de ligacao
testafila: $(objs)

# regras de compilacao
testafila.o: testafila.c queue.h
queue.o:     queue.c queue.h

# meu teste, compilacao simples enquanto queue.c nao esta pronto
teste: main.c queue.c queue.h
	gcc main.c queue.c -o main -Wall

# remove arquivos temporarios
clean:
	-rm $(objs) *~

# remove tudo que nao for codigo fonte
purge: clean
	-rm hello