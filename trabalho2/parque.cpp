
/*
   Suponha um parque com um carro de montanha-russa e N visitantes. Os
   visitantes repetidamente esperam para andar no carro, que pode conter no
   máximo C passageiros, onde C < N. Porém, o carro só pode iniciar uma volta
   quando estiver cheio. Depois de terminar uma volta, cada passageiro sai do
   carro e volta para a fila do brinquedo.

   Implemente um monitor para essei problema e também a thread carro e
   passageiros. A solução deve cumprir os seguintes requisitos obrigatórios:

   O carro sempre inicia uma volta com exatamente C passageiros;
   Nenhum passageiro pode pular fora do carro enquanto o carro está dando uma
   volta;
   Nenhum passageiro saltará para dentro do carro enquanto o carro está
   dando uma volta;
   Nenhum passageiro vai permanecer no carro após o término de uma volta (tentar
   dar duas voltas pagando apenas 1 bilhete);
   Os passageiros da volta n só poderão começar a entrar no carro depois que
   todos os passageiros da volta n-1 já saíram do carro.
*/

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <vector>
using namespace std;

const int C = 5;  // capacidade do carro
const int V = 10; // número de visitantes
const int N = 3;  // número de voltas

class MonitorCarro {
private:
  int passageiros = 0;
  mutex m;
  bool running = false;
  condition_variable na_fila;
  condition_variable no_carro;
  condition_variable enxendo_carro;
  condition_variable esvaziando_carro;

public:
  MonitorCarro() {}

  ~MonitorCarro() {}

  void entra_carro(int id) {
    unique_lock<mutex> lock(m);

    na_fila.wait(lock, [this] { return !running; });
    printf("    Visitante %d entrou no carro\n", id);
    passageiros++;
    if (passageiros == C) {
      enxendo_carro.notify_one();
    }
  }

  void sai_carro(int id) {
    unique_lock<mutex> lock(m);

    no_carro.wait(lock, [this] { return running; });
    passageiros--;
    printf("    Visitante %d saiu do carro\n", id);
    if (passageiros == 0) {
      esvaziando_carro.notify_one();
    }
  }

  void espera_encher(int v) {
    unique_lock<mutex> lock(m);
    printf("Carro esperando encher");
    enxendo_carro.wait(lock, [this] { return passageiros == C; });
    printf("Carro encheu - Iniciando volta %d", v);
    running = true;
  }

  void espera_esvaziar() {
    unique_lock<mutex> lock(m);
    printf("Carro esperando esvaziar");
    esvaziando_carro.wait(lock, [this] { return passageiros == 0; });
    printf("Carro esvaziou - Finalizando volta");
    running = false;
    na_fila.notify_all();
  }
};

MonitorCarro montanharussa;
pthread_barrier_t barrier;

void carro() {
  for (int i = 0; i < N; ++i) {
    montanharussa.espera_encher(i);
    /* dá volta */
    montanharussa.espera_esvaziar();
  }
}

void passageiro(int id) {
  for (int i = 0; i < N; i++) {
    montanharussa.entra_carro(id);
    montanharussa.sai_carro(id);
    pthread_barrier_wait(&barrier); // Todos os visitantes devem ter dado uma
                                    // volta antes de começar a próxima
  }
}

int main() {
  pthread_barrier_init(&barrier, NULL, V);
  vector<thread> threads;
  threads.push_back(thread(carro));

  for (int i = 0; i < V; i++) {
    threads.push_back(thread(passageiro, i));
  }

  for (auto &t : threads) {
    t.join();
  }
}
