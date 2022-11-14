
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

#include <assert.h>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <vector>
using namespace std;

const int C = 20;   // capacidade do carro
const int V = 1000; // número de visitantes
const int N = 150;  // número de voltas

class MonitorCarro {
private:
  int passageiros = 0;
  mutex m;
  bool podeSair = false;
  bool podeEntrar = false;
  bool closed = false;
  condition_variable na_fila;
  condition_variable no_carro;
  condition_variable enchendo_carro;
  condition_variable esvaziando_carro;

public:
  MonitorCarro() {}

  ~MonitorCarro() {}

  bool entra_carro(int id) {
    unique_lock<mutex> lock(m);

    na_fila.wait(lock, [this] { return podeEntrar && passageiros < C; });
    if (closed) {
      printf("Visitante %d não entrou no carro porque o parque fechou\n", id);
      return false;
    }
    printf("    Visitante %d entrou no carro\n", id);
    passageiros++;
    if (passageiros == C) {
      enchendo_carro.notify_one();
    }

    return true;
  }

  void sai_carro(int id) {
    unique_lock<mutex> lock(m);
    no_carro.wait(lock, [this] { return podeSair; });
    passageiros--;
    printf("    Visitante %d saiu do carro\n", id);
    if (passageiros == 0) {
      esvaziando_carro.notify_one();
    }
  }

  void espera_encher(int v) {
    unique_lock<mutex> lock(m);

    printf("Carro esperando encher\n");
    podeEntrar = true;
    podeSair = false;
    na_fila.notify_all();
    enchendo_carro.wait(lock, [this] { return passageiros == C; });

    printf("Carro encheu - Iniciando volta %d\n", v);
  }

  void espera_esvaziar(bool close_park) {
    unique_lock<mutex> lock(m);
    printf("Carro esperando esvaziar\n");
    podeSair = true;
    podeEntrar = false;
    no_carro.notify_all();
    esvaziando_carro.wait(lock, [this] { return passageiros == 0; });
    printf("Carro esvaziou - Finalizando volta\n");
    if (close_park) {
      printf("**** Parque Fechado ****\n");
      closed = true;
      podeEntrar = true;
      na_fila.notify_all();
    }
  }
  bool is_closed() {
    unique_lock<mutex> lock(m);
    return closed;
  }
};

MonitorCarro montanharussa;
pthread_barrier_t barrier;

void carro() {
  for (int i = 1; i <= N; ++i) {
    montanharussa.espera_encher(i);
    /* dá volta */
    bool close = i == N;
    montanharussa.espera_esvaziar(close);
  }
}

void passageiro(int id) {
  while (true) {
    if (!montanharussa.entra_carro(id)) {
      printf("Visitante %d saiu do parque\n", id);
      break;
    }
    montanharussa.sai_carro(id);

    pthread_barrier_wait(&barrier); // Todos os visitantes devem ter dado uma
                                    // volta antes de começar a próxima
  }
}

int main() {
  // Usamos esse assert para garantir que todas os visitantes darão o memso
  // numero de voltas Para tanto, o numero de voltas deve ser multiplo do numero
  // de visitantes/ capacidade do carro
  assert(N % (V / C) == 0);
  pthread_barrier_init(&barrier, NULL, V);
  vector<thread> threads;
  printf("**** Parque Aberto ****\n");

  thread car(carro);

  for (int i = 0; i < V; i++) {
    threads.push_back(thread(passageiro, i));
  }

  car.join();
  pthread_barrier_destroy(&barrier);
  for (auto &t : threads) {
    t.join();
  }
}
