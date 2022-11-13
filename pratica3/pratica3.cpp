/*******************************************************************************
 * Problema da vacinação em duas etapas utilizando semáforo.
 * Em um esquema de vacinação em duas doses, onde 3 vacinas diferentes estão
 * disponíveis, uma pessoa pode tomar qualquer uma das 3 vacinas na primeira
 * dose. Na segunda dose, a vacina deve ser igual à tomada na primeira dose.
 * Por simplicidade, um único produtor produz as 3 vacinas e adiciona ao
 * estoque.
 * Solução utilizando apenas 1 semáforo para a primeira dose.
 */

#include <semaphore.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <vector>
using namespace std;

const int POPULACAO = 10000; // tamanho população
const int N_VACINAS = 3;
sem_t mux;
sem_t vac[N_VACINAS + 1]; // vac[0]: dose 1 ; vac[1..N]: dose 2
int nvac[N_VACINAS] = {}; // vacinas usadas na primeira dose

void prod() {
  // produzir primeira dose
  for (int i = 0; i < POPULACAO; ++i) {
    sem_wait(&mux);
    sem_post(&vac[0]);
    nvac[i % 3]++;
    sem_post(&mux);
    // usleep(100);
  }
  // produzir segunda dose
  for (int i = 0; i < POPULACAO; ++i) {
    sem_post(&vac[i % 3 + 1]);
  }
}

void cons(int id) {
  int v;
  sem_wait(&vac[0]); // só libera se existir dose 1 disponível
  sem_wait(&mux);

  if (nvac[0] > 0)
    v = 0;
  else if (nvac[1] > 0)
    v = 1;
  else if (nvac[2] > 0)
    v = 2;
  else
    printf("erro: nenhuma vacina disponível\n");

  nvac[v]--;

  if (nvac[0] < 0 || nvac[1] < 0 || nvac[2] < 0) // verificação de erro
    printf("erro: %d %d %d\n", nvac[0], nvac[1], nvac[2]);

  // printf("%d : dose 1 vacina %d\n",id,v+1);
  sem_post(&mux);

  sem_wait(&vac[v + 1]); // espera pela segunda dose
  // printf("   %d : dose 2 vacina %d\n", id, v + 1);
}

int main() {
  sem_init(&vac[0], 0, 0);
  sem_init(&vac[1], 0, 0);
  sem_init(&vac[2], 0, 0);
  sem_init(&vac[3], 0, 0);
  sem_init(&mux, 0, 1);
  vector<thread> v;

  thread p(prod);
  for (int i = 0; i < POPULACAO; ++i) {
    thread c(cons, i);
    v.push_back(move(c));
  }

  p.join();
  for (thread &t : v)
    t.join();

  printf("estoque vacinas: %d %d %d\n", nvac[0], nvac[1], nvac[2]);

  sem_destroy(&vac[0]);
  sem_destroy(&vac[1]);
  sem_destroy(&vac[2]);
  sem_destroy(&vac[3]);
  sem_destroy(&mux);
  return 0;
}
