/* Autor: Douglas Liberalesso
   Compilar utilizando "gcc main.c -pthread pilha.h -o prod_cons" */

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include "pilha.h"

/* Quantos ciclos o produtor vai fazer */
#define LIMITE 512

/* Estrutura a ser compartilha entre processos
   Tem 2 semaforos, 1 sinalizador, e outra estrutura que é uma Pilha (ver Pilha.h)
   A pilha tem 256 posicoes */
typedef struct {
  sem_t p_sem;
  sem_t c_sem;
  int termina;
  Pilha pilha;
} Shared;

/* Funcao para gerar numero randomico */
int randomNumber(int min, int max) {
  int result = 0, low = 0, high = 0;
  if (min < max) {
    low = min;
    high = max + 1;
  } else {
    low = max + 1;
    high = min;
  }

  result = (rand() % (high - low)) + low;
  return result;
}

void produtor(Shared *shm) {
  int i, j, n;

  /* Total de unidades produzidas */
  int produzidos = 0;

  for(i = 1; i <= LIMITE; i++) {
    /* Aguarda ate semaforo ficar verde */
    sem_wait(&shm->p_sem);
    
    /* produz entre 0 e 9 vezes por ciclo */
    n = randomNumber(0, 9);
    printf("[PID: %d] Produzir: %d vezes\n", getpid(), n);

    /* Produz N produtos por ciclo */
    for(j = 0; j < n; j++) {
      if(!pilhaCheia(&shm->pilha)) {
        printf("[PID: %d] Produzindo: %d - Lote: %d\n", getpid(), ++produzidos, i);
        empilha(&shm->pilha, produzidos);
      } else {
        printf("[PID: %d] Pilha encheu! O lote %d teve %d unidades\n", getpid(), i, j);
        break;
      }
    }

    /* Sinaliza que nao vai mais produzir caso tenha executado todos os ciclos */
    if(i == LIMITE) {
      shm->termina = 1;
      printf("[PID: %d] Não vai mais produzir\n", getpid());
    }

    /* Libera consumidores */
    sem_post(&shm->c_sem);
  }
}

void consumidor(Shared *shm) {
  int j, n;
  while(1) {
    /* Se o produtor nao vai mais produzir, o proprio consumidor tem que liberar o seu semaforo */
    if(shm->termina) {
      sem_post(&shm->c_sem);
      /* Se a pilha ficar vazia e o produtor nao vai mais produzir, terminou a tarefa */
      if(pilhaVazia(&shm->pilha)) break;
    }

    /* Aguarda ate semaforo ficar verde */
    sem_wait(&shm->c_sem);
    
    /* Cada consumidor vai consumir entre 0 e 9 vezes por ciclo */
    n = randomNumber(0, 9);
    printf("[PID: %d] Consumir: %d vezes\n", getpid(), n);

    /* Consome N vezes por ciclo */
    for(j = 0; j < n; j++) {
      if(!pilhaVazia(&shm->pilha)) {
        printf("[PID: %d] Consumindo: %d\n", getpid(), topoPilha(&shm->pilha));
        desempilha(&shm->pilha);
      } else {
        printf("[PID: %d] Pilha esvaziou! Lote teve %d unidades\n", getpid(), j);
        break;
      }
    }

    /* Libera produtor */
    sem_post(&shm->p_sem);
  }
}

int main(void) {
  int processos = 2, id = 0, i = 0;

  /* Cria area compartilhada na memoria e inicializa semaforos */
  Shared *shm = mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  sem_init(&shm->p_sem, 1, 1);
  sem_init(&shm->c_sem, 1, 0);
  shm->termina = 0;

  /* O buffer compartilhado é uma pilha de 256 posições, ver pilha.h criada com o Alessandro */
  initPilha(&shm->pilha);

  /* Seed do gerador de numeros randomicos */
  srand(time(NULL));

  /* Cria filhos consumidores */
  for(i = 0; i < processos; i++) {
    id = fork();
    if(id < 0) {
      perror("Erro!\n");
      exit(1);
    } else if(id == 0) {
      consumidor(shm);
      exit(0);
    }
  }

  /* Processo pai é o produtor */
  produtor(shm);

  /* Aguarda que os filhos terminem a execução */
  for(i = 0; i < processos; ++i) {
    wait(NULL);
  }

  printf("FIM\n");
  return 1;
}
