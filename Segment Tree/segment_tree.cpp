
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>

using namespace std;

#define MAX_REGISTROS 15000

struct Medicao {
    string data;
    float insolacao;
    float precipitacao;
    float temp_max;
    float temp_min;
    float umidade;
    float vento;
};

struct SegmentTree {
    vector<float> tree;
    int n;

    SegmentTree(const vector<float>& data) {
        n = data.size();
        tree.resize(2 * n);
        for (int i = 0; i < n; i++) tree[n + i] = data[i];
        for (int i = n - 1; i > 0; i--) tree[i] = tree[i << 1] + tree[i << 1 | 1];
    }

    float query(int l, int r) {
        float sum = 0.0;
        l += n; r += n;
        while (l <= r) {
            if (l % 2 == 1) sum += tree[l++];
            if (r % 2 == 0) sum += tree[r--];
            l >>= 1; r >>= 1;
        }
        return sum;
    }
};

struct MaxSegmentTree {
    vector<float> tree;
    int n;

    MaxSegmentTree(const vector<float>& data) {
        n = data.size();
        tree.resize(2 * n);
        for (int i = 0; i < n; i++) tree[n + i] = data[i];
        for (int i = n - 1; i > 0; i--) tree[i] = max(tree[i << 1], tree[i << 1 | 1]);
    }

    float query(int l, int r) {
        float result = -1e9;
        l += n; r += n;
        while (l <= r) {
            if (l % 2 == 1) result = max(result, tree[l++]);
            if (r % 2 == 0) result = max(result, tree[r--]);
            l >>= 1; r >>= 1;
        }
        return result;
    }
};

vector<Medicao> dados;
map<string, int> indice_data;
SegmentTree *st_temp_max = nullptr;
MaxSegmentTree *st_max_temp = nullptr;

void carregar_dados_csv(const string& nome_arquivo) {
    ifstream file(nome_arquivo);
    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo " << nome_arquivo << endl;
        exit(1);
    }

    string linha;
    getline(file, linha);
    int count = 0;
    while (getline(file, linha) && count < MAX_REGISTROS) {
        stringstream ss(linha);
        string campo;
        Medicao m;
        getline(ss, m.data, ',');
        getline(ss, campo, ','); m.insolacao = stof(campo);
        getline(ss, campo, ','); m.precipitacao = stof(campo);
        getline(ss, campo, ','); m.temp_max = stof(campo);
        getline(ss, campo, ','); m.temp_min = stof(campo);
        getline(ss, campo, ','); m.umidade = stof(campo);
        getline(ss, campo, ','); m.vento = stof(campo);

        indice_data[m.data] = dados.size();
        dados.push_back(m);
        count++;
    }

    vector<float> temp_max;
    for (auto& d : dados) {
        temp_max.push_back(d.temp_max);
    }

    st_temp_max = new SegmentTree(temp_max);
    st_max_temp = new MaxSegmentTree(temp_max);
}

void consultar_temp_max(string data_ini, string data_fim, const string& tipo) {
    int l = indice_data[data_ini];
    int r = indice_data[data_fim];
    if (l > r) swap(l, r);

    clock_t inicio = clock();

    if (tipo == "media" || tipo == "soma") {
        float soma = st_temp_max->query(l, r);
        if (tipo == "soma") {
            cout << "Soma da temperatura maxima: " << soma << endl;
        } else {
            cout << "Media da temperatura maxima: " << (soma / (r - l + 1)) << endl;
        }
    } else if (tipo == "maximo") {
        float maximo = st_max_temp->query(l, r);
        cout << "Temperatura maxima absoluta no intervalo: " << maximo << "C" << endl;
    } else {
        cout << "Tipo de consulta invalido.\n";
    }

    clock_t fim = clock();
    double duracao = double(fim - inicio) / CLOCKS_PER_SEC;
    cout << "Tempo da consulta: " << duracao << "segundos\n";
}

int main() {
    cout << "=== CONSULTA DE TEMPERATURA MAXIMA COM SEGMENT TREE ===\n";
    cout << "Consulta por intervalo de datas. Operacoes: soma, media, maximo.\n";
    cout << "Intervalo de datas: 1995 a 2025. Formato: AAAA-MM-DD.\n";

    string nome_arquivo = "d1dos_tratado.csv";
    clock_t inicio = clock();
    carregar_dados_csv(nome_arquivo);
    clock_t fim = clock();
    cout << "Tempo para carregar CSV e construir arvores: " 
         << double(fim - inicio) / CLOCKS_PER_SEC << "segundos\n";

    while (true) {
        string d1, d2, tipo;
        cout << "\nDigite data inicial (AAAA-MM-DD) ou 'sair': ";
        cin >> d1;
        if (d1 == "sair") break;
        cout << "Digite data final (AAAA-MM-DD): ";
        cin >> d2;
        cout << "Tipo de consulta (soma, media, maximo): ";
        cin >> tipo;

        if (indice_data.count(d1) && indice_data.count(d2)) {
            consultar_temp_max(d1, d2, tipo);
        } else {
            cout << "Datas nao encontradas.\n";
        }
    }

    return 0;
}
