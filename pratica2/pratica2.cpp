/*******************************************************************************
 * Problema da festa com copo compartilhado utilizando monitor.
 * O código apresenta duas versões diferentes para o monitor. Para escolher a
 * versão a ser utilizada, comente o trecho indicado nas funções "serve" e
 * "bebe".
 */

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;

class MonitorFesta {
private:
  int bebida = 0; // inicialmente o copo está vazio
  mutex mux;
  condition_variable vazio;
  condition_variable cheio;
  int cont[3] = {0, 0, 0}; // usado para verificação do resultado
public:
  MonitorFesta() {}

  ~MonitorFesta() {}

  void serve(int b) {
    unique_lock<mutex> lck(mux);
    vazio.wait(lck, [this] { return bebida == 0; });
    bebida = b;
    // printf("servindo bebida %d\n",b);

    cheio.notify_all(); // versão 1
    // cheio.notify_one();         //versão 2
  }

  bool bebe(int id) {
    unique_lock<mutex> lck(mux);

    /* versão 1: Garçon precisa fazer notify-all */
    cheio.wait(lck, [this, id] { return bebida != 0 && bebida <= id; });

    /* versão 2: Se o convidado não quer beber, ele acorda um possível
    interessado e volta a dormir */
    // while(bebida==0 || bebida>id) {
    //     cheio.wait(lck);
    //     if (bebida>id)
    //         cheio.notify_one();
    // }

    if (bebida == -1)
      return false; // indica o fim da festa
    // printf("             %d bebe bebida %d\n",id,bebida);
    cont[id - 1]++; // doses bebidas por cada convidado
    bebida = 0;
    vazio.notify_one();
    return true;
  }

  void fim() {
    unique_lock<mutex> lck(mux);
    vazio.wait(lck, [this] { return bebida == 0; });
    bebida = -1;
    cheio.notify_all();
  }

  void resultados(int numDoses) {
    /* Como o monitor não deve dar preferência para nenhum dos convidados,
    o esperado é que cada convidado beba aproximadamente:
    - convidado 1: 1/3 * 1/3 =                   11% do total
    - convidado 2: 1/3 * 1/3 + 1/3 * 1/2 =       28% do total
    - convidado 3: 1/3 * 1/3 + 1/3 * 1/2 + 1/3 = 61% do total  */
    printf("%d doses servidas\n", numDoses);
    int soma = 0;
    for (int i = 0; i < 3; ++i) {
      printf("%d bebeu %d doses (%.1f%%)\n", i + 1, cont[i],
             (100.0 * cont[i] / numDoses));
      soma += cont[i];
    }
    printf("%.0f%% das doses foram consumidas\n", (100.0 * soma / numDoses));
  }
};

MonitorFesta bebida;

void garcon(int n) {
  for (int i = 0; i < n; ++i) {
    int b = rand() % 3 + 1; // busca uma bebida aleatória
    bebida.serve(b);        // serve bebida aleatória
  }
  bebida.fim(); // mandar convidados embora
}

void convidado(int id) {
  while (true) {
    if (!bebida.bebe(id)) // retorna false quando a festa acabar
      break;
  }
}

int main() {
  int numDoses = 1000000;
  vector<thread> threads;
  threads.push_back(thread(garcon, numDoses));
  for (int i = 1; i <= 3; ++i)
    threads.push_back(thread(convidado, i));

  for (thread &t : threads)
    t.join();

  bebida.resultados(numDoses);
}
