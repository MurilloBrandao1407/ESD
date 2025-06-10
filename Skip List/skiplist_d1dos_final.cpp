#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>

using namespace std;

#define MAX_LEVEL 6
#define PROBABILIDADE 0.5
#define MAX_LINHA 512

struct Medicao {
    char data[20];
    float insolacao;
    float precipitacao;
    float temp_max;
    float temp_min;
    float umidade;
    float vento;
};

struct Node {
    Medicao medicao;
    Node** forward;
    int level;
};

class SkipList {
private:
    Node* header;
    int nivel_atual;

    int total;
    float soma_temp_max, soma_temp_min, soma_prec, soma_insol;
    float soma2_temp_max, soma2_temp_min, soma2_prec, soma2_insol;

    int randomLevel() {
        int lvl = 1;
        while (((float)rand() / RAND_MAX) < PROBABILIDADE && lvl < MAX_LEVEL)
            lvl++;
        return lvl;
    }

    Node* criarNode(int lvl, const Medicao& med) {
        Node* node = new Node;
        node->medicao = med;
        node->forward = new Node*[lvl + 1];
        memset(node->forward, 0, sizeof(Node*) * (lvl + 1));
        node->level = lvl;
        return node;
    }

public:
    SkipList() {
        Medicao vazio;
        strcpy(vazio.data, "");
        nivel_atual = 0;
        total = 0;
        soma_temp_max = soma_temp_min = soma_prec = soma_insol = 0;
        soma2_temp_max = soma2_temp_min = soma2_prec = soma2_insol = 0;
        header = criarNode(MAX_LEVEL, vazio);
        srand((unsigned)time(NULL));
    }

    void inserir(const Medicao& med) {
        Node* update[MAX_LEVEL + 1];
        Node* atual = header;

        for (int i = nivel_atual; i >= 0; i--) {
            while (atual->forward[i] && strcmp(atual->forward[i]->medicao.data, med.data) < 0)
                atual = atual->forward[i];
            update[i] = atual;
        }

        atual = atual->forward[0];

        if (!atual || strcmp(atual->medicao.data, med.data) != 0) {
            int novo_nivel = randomLevel();
            if (novo_nivel > nivel_atual) {
                for (int i = nivel_atual + 1; i <= novo_nivel; i++)
                    update[i] = header;
                nivel_atual = novo_nivel;
            }

            Node* novo = criarNode(novo_nivel, med);
            for (int i = 0; i <= novo_nivel; i++) {
                novo->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = novo;
            }

            total++;
            soma_temp_max += med.temp_max;
            soma_temp_min += med.temp_min;
            soma_prec += med.precipitacao;
            soma_insol += med.insolacao;

            soma2_temp_max += med.temp_max * med.temp_max;
            soma2_temp_min += med.temp_min * med.temp_min;
            soma2_prec += med.precipitacao * med.precipitacao;
            soma2_insol += med.insolacao * med.insolacao;
        }
    }

    void estatisticas() {
        if (total == 0) {
            cout << "Nenhum dado para calcular estatisticas.\n";
            return;
        }

        float media_max = soma_temp_max / total;
        float media_min = soma_temp_min / total;
        float media_prec = soma_prec / total;
        float media_insol = soma_insol / total;

        float dp_max = sqrt((soma2_temp_max / total) - (media_max * media_max));
        float dp_min = sqrt((soma2_temp_min / total) - (media_min * media_min));
        float dp_prec = sqrt((soma2_prec / total) - (media_prec * media_prec));
        float dp_insol = sqrt((soma2_insol / total) - (media_insol * media_insol));

        cout << "\n--- Estatisticas ---\n";
        cout << "Total de registros: " << total << endl;
        cout << "Media temperatura maxima: " << media_max << " | Desvio padrao: " << dp_max << endl;
        cout << "Media temperatura minima: " << media_min << " | Desvio padrao: " << dp_min << endl;
        cout << "Media precipitacao: " << media_prec << " | Desvio padrao: " << dp_prec << endl;
        cout << "Media insolacao: " << media_insol << " | Desvio padrao: " << dp_insol << endl;
    }
};

void lerCSV(const char* nome_arquivo, SkipList& lista) {
    FILE* arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        cout << "Erro ao abrir o arquivo.\n";
        return;
    }

    char linha[MAX_LINHA];
    fgets(linha, MAX_LINHA, arquivo); // pular cabecalho

    clock_t inicio = clock();

    while (fgets(linha, MAX_LINHA, arquivo)) {
        Medicao m;
        char* campo = strtok(linha, ",");

        if (campo != NULL) {
            strncpy(m.data, campo, 20);
            campo = strtok(NULL, ","); m.insolacao = campo ? atof(campo) : 0;
            campo = strtok(NULL, ","); m.precipitacao = campo ? atof(campo) : 0;
            campo = strtok(NULL, ","); m.temp_max = campo ? atof(campo) : 0;
            campo = strtok(NULL, ","); m.temp_min = campo ? atof(campo) : 0;
            campo = strtok(NULL, ","); m.umidade = campo ? atof(campo) : 0;
            campo = strtok(NULL, ","); m.vento = campo ? atof(campo) : 0;

            lista.inserir(m);
        }
    }

    clock_t fim = clock();
    double tempo_segundos = (double)(fim - inicio) / CLOCKS_PER_SEC;
    cout << "\nTempo de insercao: " << tempo_segundos << " segundos\n";

    fclose(arquivo);
}

int main() {
    SkipList lista;
    lerCSV("d1dos.csv", lista);
    lista.estatisticas();
    return 0;
}