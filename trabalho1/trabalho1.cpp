//! Definitions of necessary libraries
#include "my_sync.h"
#include <assert.h>
#include <iostream>
#include <map>
#include <pthread.h>
#include <queue>
#include <string>
//! Namespace used
using namespace std;

//! Definitions
#define BUFFER_SIZE 1
#define NUM_THREADS 2
#define PROD_SIZE 10

//! Global variables
pthread_mutex_t mutex;
queue<pthread_t> fila_prod;
queue<pthread_t> fila_cons;
int buffer[BUFFER_SIZE];
int buffer_size = 0;
int pos_write = 0;
int pos_read = 0;

int consumir(int id_thread) {
  bool bloquear = false;
  int item;

  do {
    bloquear = false;
    pthread_mutex_lock(&mutex);
    if (buffer_size == 0) // se o buffer estiver vazio, eu espero colocarem algo
    {
      bloquear = true;
      fila_cons.push(pthread_self()); // adiciono a fila de consumidores
      printf("Consumidor bloqueado, buffer vazio: %d\n", buffer_size);
    } else // Tenho itens para consumir
    {
      item = buffer[pos_read];
      buffer[pos_read] = 0; // limpo a posição
      printf("Thread %d consumiu %d\n", id_thread, item);
      pos_read = (pos_read + 1) % BUFFER_SIZE;
      buffer_size--;

      if (!fila_prod.empty()) // Se a fila de produtores não estiver vazia, eu
                              // libero um produtor
      {
        pthread_t prod = fila_prod.front();
        fila_prod.pop();
        wakeup(prod);
      }
    }
    pthread_mutex_unlock(&mutex);
    if (bloquear)
      block();

  } while (bloquear);

  return item;
}

void produzir(int dado, int id_thread) {
  bool bloquear = false;
  do {

    bloquear = false;
    pthread_mutex_lock(&mutex);

    if (buffer_size == BUFFER_SIZE) // Buffer está cheio
    {
      bloquear = true;
      fila_prod.push(pthread_self()); // Adiciona o produtor na fila de espera
      printf("Produtor bloqueado, buffer cheio: %d\n", buffer_size);
    } else {
      assert(buffer[pos_write] == 0); // Não pode ter nada na posição
      buffer[pos_write] = dado;
      printf("Thread %d produziu %d\n", id_thread, dado);
      pos_write = (pos_write + 1) % BUFFER_SIZE;
      buffer_size++;
      if (!fila_cons.empty()) // Se houver algum consumidor esperando, acorda-o
      {
        pthread_t cons = fila_cons.front();
        fila_cons.pop();
        wakeup(cons);
      }
    }
    pthread_mutex_unlock(&mutex);
    if (bloquear)
      block();

  } while (bloquear);
}

void *thread(void *id) {
  int id_thread = (long)id;
  if (id_thread % 2 == 0) {
    // Codigo executado por threads pares (produtores)
    for (int i = 0; i < PROD_SIZE; i++) {
      produzir(i, id_thread);
    }
  } else {
    // Codigo executado por threads impares (consumidores)
    for (int i = 0; i < PROD_SIZE; i++) {
      int item = consumir(id_thread);
    }
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int a = 0;

  pthread_mutex_init(&mutex, NULL);

  pthread_t threads[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_t t;
    pthread_create(&t, NULL, thread, (void *)i);
    threads[i] = t;
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  cout << "Fim do programa. Buffer size: " << buffer_size << endl;
  pthread_mutex_destroy(&mutex);
  return 0;
}
