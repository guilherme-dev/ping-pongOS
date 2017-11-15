// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Teste do operador task_join

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ppos.h"

#define WORKLOAD 20000

task_t Pang, Peng, Ping, Pong, Pung ;

// simula um processamento pesado
int hardwork (int n)
{
   int i, j, soma ;

   soma = 0 ;
   for (i=0; i<n; i++)
      for (j=0; j<n; j++)
         soma += j ;
   return (soma) ; 
}

// corpo das threads
void Body (void * arg)
{
   int i, max ;

   max = task_id() * 2 ;

   printf ("%s: inicio\n", (char *) arg) ;
   for (i=0; i<max; i++)
   {
      printf ("%s: %d\n", (char *) arg, i) ;
      hardwork (WORKLOAD) ;
   }
   printf ("%s: fim\n", (char *) arg) ;
   task_exit (0) ;
}

int main (int argc, char *argv[])
{
   int i ;

   ppos_init () ;

   printf ("main: inicio\n");

   task_create (&Pang, Body, "    Pang") ;
   sleep(1);
   task_create (&Peng, Body, "        Peng") ;
   sleep(1);
   task_create (&Ping, Body, "            Ping") ;
   sleep(1);
   task_create (&Pong, Body, "                Pong") ;
   sleep(1);
   task_create (&Pung, Body, "                    Pung") ;

   for (i=0; i<2; i++)
   {
      printf ("main: %d\n", i) ;
      hardwork (WORKLOAD) ;
   }

   printf ("main: esperando Pang...\n") ;
   task_join (&Pang) ;

   printf ("main: Pang acabou, esperando Peng...\n") ;
   task_join (&Peng) ;

   printf ("main: Peng acabou, esperando Ping...\n") ;
   task_join (&Ping) ;

   printf ("main: Ping acabou, esperando Pong...\n") ;
   task_join (&Pong) ;

   printf ("main: Pong acabou, esperando Pung...\n") ;
   task_join (&Pung) ;

   printf ("main: Pung acabou\n") ;
   printf ("main: fim\n");

   task_exit (0) ;

   exit (0) ;
}
