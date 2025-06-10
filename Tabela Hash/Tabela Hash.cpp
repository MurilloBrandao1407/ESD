
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;
using namespace std::chrono;

#define TAMANHO_TABELA 1031

struct Medicao {
    string data;
    float insolacao, precipitacao, temp_max, temp_min, umidade, vento;
};

struct No {
    Medicao medicao;
    No* prox;
};

No* tabela[TAMANHO_TABELA] = {nullptr};
string data_minima = "9999-12-31";
string data_maxima = "0000-01-01";

int calcularHash(string data) {
    int soma = 0;
    for (char c : data) soma += c;
    return soma % TAMANHO_TABELA;
}

void inserir(Medicao m) {
    int indice = calcularHash(m.data);
    No* novo = new No{m, tabela[indice]};
    tabela[indice] = novo;
    if (m.data < data_minima) data_minima = m.data;
    if (m.data > data_maxima) data_maxima = m.data;
}

bool remover(string data) {
    int indice = calcularHash(data);
    No* atual = tabela[indice];
    No* anterior = nullptr;
    while (atual != nullptr) {
        if (atual->medicao.data == data) {
            if (anterior == nullptr) tabela[indice] = atual->prox;
            else anterior->prox = atual->prox;
            delete atual;
            return true;
        }
        anterior = atual;
        atual = atual->prox;
    }
    return false;
}

void buscar(string data) {
    int indice = calcularHash(data);
    No* atual = tabela[indice];
    while (atual != nullptr) {
        if (atual->medicao.data == data) {
            cout << "\nDados da data " << data << ":\n";
            cout << "Insolacao (h): " << atual->medicao.insolacao << endl;
            cout << "Precipitacao (mm): " << atual->medicao.precipitacao << endl;
            cout << "Temperatura Max (C): " << atual->medicao.temp_max << endl;
            cout << "Temperatura Min (C): " << atual->medicao.temp_min << endl;
            cout << "Umidade Relativa (%): " << atual->medicao.umidade << endl;
            cout << "Velocidade do Vento (m/s): " << atual->medicao.vento << endl;
            return;
        }
        atual = atual->prox;
    }
    cout << "Data nao encontrada.\n";
}

void lerCSV(string nomeArquivo) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo.\n";
        return;
    }
    string linha;
    getline(arquivo, linha);
    while (getline(arquivo, linha)) {
        stringstream ss(linha);
        string item;
        Medicao m;
        getline(ss, m.data, ',');
        getline(ss, item, ','); m.insolacao = atof(item.c_str());
        getline(ss, item, ','); m.precipitacao = atof(item.c_str());
        getline(ss, item, ','); m.temp_max = atof(item.c_str());
        getline(ss, item, ','); m.temp_min = atof(item.c_str());
        getline(ss, item, ','); m.umidade = atof(item.c_str());
        getline(ss, item);      m.vento = atof(item.c_str());
        inserir(m);
    }
    arquivo.close();
}

void salvarCSV(string nomeArquivo = "dados_atualizados.csv") {
    ofstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cout << "Erro ao salvar o arquivo.\n";
        return;
    }
    arquivo << "Data,INSOLACAO,PRECIPITACAO,TEMP_MAX,TEMP_MIN,UMIDADE,VENTO\n";
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        No* atual = tabela[i];
        while (atual != nullptr) {
            Medicao m = atual->medicao;
            arquivo << m.data << "," << m.insolacao << "," << m.precipitacao << ","
                    << m.temp_max << "," << m.temp_min << "," << m.umidade << "," << m.vento << "\n";
            atual = atual->prox;
        }
    }
    arquivo.close();
    cout << "Arquivo salvo com sucesso.\n";
}

void inserirManual() {
    Medicao m;
    cout << "\nDigite os dados da nova medicao:\n";
    string ano, mes, dia;
    cout << "Ano (" << data_minima.substr(0, 4) << " a " << data_maxima.substr(0, 4) << "): "; cin >> ano;
    cout << "Mes (" << data_minima.substr(5, 2) << " a " << data_maxima.substr(5, 2) << "): "; cin >> mes;
    cout << "Dia (" << data_minima.substr(8, 2) << " a " << data_maxima.substr(8, 2) << "): "; cin >> dia;
    m.data = ano + "-" + (mes.length()==1 ? "0"+mes : mes) + "-" + (dia.length()==1 ? "0"+dia : dia);
    cout << "Insolacao: "; cin >> m.insolacao;
    cout << "Precipitacao: "; cin >> m.precipitacao;
    cout << "Temperatura Max: "; cin >> m.temp_max;
    cout << "Temperatura Min: "; cin >> m.temp_min;
    cout << "Umidade: "; cin >> m.umidade;
    cout << "Vento: "; cin >> m.vento;
    inserir(m);
    cout << "Medicao inserida com sucesso.\n";
    salvarCSV();
}

void calcularMediaCampo() {
    string campo;
    cout << "\nCampo para calcular media: "; cin >> campo;
    double soma = 0;
    int cont = 0;
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        No* atual = tabela[i];
        while (atual) {
            if (campo == "insolacao") soma += atual->medicao.insolacao;
            else if (campo == "precipitacao") soma += atual->medicao.precipitacao;
            else if (campo == "temp_max") soma += atual->medicao.temp_max;
            else if (campo == "temp_min") soma += atual->medicao.temp_min;
            else if (campo == "umidade") soma += atual->medicao.umidade;
            else if (campo == "vento") soma += atual->medicao.vento;
            else { cout << "Campo invalido.\n"; return; }
            cont++; atual = atual->prox;
        }
    }
    if (cont == 0) cout << "Nenhum dado.\n";
    else cout << "Media: " << (soma / cont) << "\n";
}

void filtrarPorCondicao() {
    string campo; float limite;
    cout << "\nCampo para filtrar: "; cin >> campo;
    cout << "Valor minimo: "; cin >> limite;
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        No* atual = tabela[i];
        while (atual) {
            float valor = 0;
            if (campo == "insolacao") valor = atual->medicao.insolacao;
            else if (campo == "precipitacao") valor = atual->medicao.precipitacao;
            else if (campo == "temp_max") valor = atual->medicao.temp_max;
            else if (campo == "temp_min") valor = atual->medicao.temp_min;
            else if (campo == "umidade") valor = atual->medicao.umidade;
            else if (campo == "vento") valor = atual->medicao.vento;
            else { cout << "Campo invalido.\n"; return; }

            if (valor >= limite)
                cout << "Data: " << atual->medicao.data << " | " << campo << ": " << valor << "\n";

            atual = atual->prox;
        }
    }
}

void executarBenchmarkHash() {
    cout << "\n===== Benchmark: Tabela Hash =====\n";
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        while (tabela[i]) {
            No* temp = tabela[i];
            tabela[i] = tabela[i]->prox;
            delete temp;
        }
    }
    ifstream arquivo("d1dos_meteorologicos_tratado (2).csv");
    string linha; getline(arquivo, linha);
    int total = 0, colisoes = 0;
    auto inicio = high_resolution_clock::now();
    while (getline(arquivo, linha) && total < 1000) {
        stringstream ss(linha); string item; Medicao m;
        getline(ss, m.data, ',');
        getline(ss, item, ','); m.insolacao = atof(item.c_str());
        getline(ss, item, ','); m.precipitacao = atof(item.c_str());
        getline(ss, item, ','); m.temp_max = atof(item.c_str());
        getline(ss, item, ','); m.temp_min = atof(item.c_str());
        getline(ss, item, ','); m.umidade = atof(item.c_str());
        getline(ss, item);      m.vento = atof(item.c_str());
        if (tabela[calcularHash(m.data)] != nullptr) colisoes++;
        inserir(m); total++;
    }
    auto fim = high_resolution_clock::now();
    cout << "Insercoes: " << total << " | Colisoes: " << colisoes
         << " | Tempo: " << duration_cast<milliseconds>(fim - inicio).count() << " ms\n";
}

int limite_max_registros = 500;
int registros_atuais = 0;

bool inserirComRestricao(Medicao m) {
    if (registros_atuais >= limite_max_registros) {
        cout << "Limite de memoria atingido.\n"; return false;
    }
    inserir(m); registros_atuais++;
    return true;
}

void simularLatencia() {
    cout << "\nSimulando latencia...\n";
    for (int i = 0; i < 5; i++) {
        cout << "."; this_thread::sleep_for(milliseconds(300));
    }
    cout << "\nPronto.\n";
}

void inserirDadosLimitadoCSV() {
    ifstream arquivo("d1dos_meteorologicos_tratado (2).csv");
    string linha; getline(arquivo, linha);
    while (getline(arquivo, linha) && registros_atuais < limite_max_registros) {
        stringstream ss(linha); string item; Medicao m;
        getline(ss, m.data, ',');
        getline(ss, item, ','); m.insolacao = atof(item.c_str());
        getline(ss, item, ','); m.precipitacao = atof(item.c_str());
        getline(ss, item, ','); m.temp_max = atof(item.c_str());
        getline(ss, item, ','); m.temp_min = atof(item.c_str());
        getline(ss, item, ','); m.umidade = atof(item.c_str());
        getline(ss, item);      m.vento = atof(item.c_str());
        inserirComRestricao(m);
    }
    cout << "Importacao concluida com restricao.\n";
}


void mostrarMinimoMaximoPorCampo() {
    string menorAno = data_minima.substr(0, 4);
    string maiorAno = data_maxima.substr(0, 4);

    string menorMes = data_minima.substr(5, 2);
    string maiorMes = data_maxima.substr(5, 2);

    string menorDia = data_minima.substr(8, 2);
    string maiorDia = data_maxima.substr(8, 2);

    cout << "\n==== MINIMOS E MAXIMOS DE DATAS ====\n";
    cout << "Ano minimo: " << menorAno << " | Ano maximo: " << maiorAno << "\n";
    cout << "Mes minimo: " << menorMes << " | Mes maximo: " << maiorMes << "\n";
    cout << "Dia minimo: " << menorDia << " | Dia maximo: " << maiorDia << "\n";
}


int main() {
    lerCSV("d1dos_meteorologicos_tratado (2).csv");
    int opcao;

    cout << "\nDatas disponiveis no sistema:\n";
    cout << "  Ano minimo: " << data_minima.substr(0, 4) << " | Ano maximo: " << data_maxima.substr(0, 4) << "\n";
    cout << "  Mes minimo: " << data_minima.substr(5, 2) << " | Mes maximo: " << data_maxima.substr(5, 2) << "\n";
    cout << "  Dia minimo: " << data_minima.substr(8, 2) << " | Dia maximo: " << data_maxima.substr(8, 2) << "\n";

    do {
        cout << "\n===== MENU =====\n";
        cout << "1 - Buscar por data\n";
        cout << "2 - Inserir nova medicao\n";
        cout << "3 - Remover por data\n";
        cout << "4 - Calcular media\n";
        cout << "5 - Filtrar por condicao\n";
        cout << "6 - Benchmark\n";
        cout << "7 - Inserir com limite\n";
        cout << "8 - Simular latencia\n";
        cout << "9 - Ver limites de ano, mes e dia\n";
cout << "0 - Sair\n";
        cout << "Opcao: "; cin >> opcao;

        string data;
        switch (opcao) {
            case 1: {
                string ano, mes, dia;
                cout << "Ano (" << data_minima.substr(0, 4) << " a " << data_maxima.substr(0, 4) << "): "; cin >> ano;
                cout << "Mes (" << data_minima.substr(5, 2) << " a " << data_maxima.substr(5, 2) << "): "; cin >> mes;
                cout << "Dia (" << data_minima.substr(8, 2) << " a " << data_maxima.substr(8, 2) << "): "; cin >> dia;
                data = ano + "-" + (mes.length()==1?"0"+mes:mes) + "-" + (dia.length()==1?"0"+dia:dia);
                buscar(data);
                break;
            }
            case 2: inserirManual(); break;
            case 3: {
                cout << "Data para remover: "; cin >> data;
                remover(data) ? cout << "Removido.\n" : cout << "Nao encontrado.\n";
                break;
            }
            case 4: calcularMediaCampo(); break;
            case 5: filtrarPorCondicao(); break;
            case 6: executarBenchmarkHash(); break;
            case 7: inserirDadosLimitadoCSV(); break;
            case 8: simularLatencia(); break;
            case 9: mostrarMinimoMaximoPorCampo(); break;
    case 0: cout << "Saindo...\n"; break;
            default: cout << "Opcao invalida.\n";
        }
    } while (opcao != 0);
    return 0;
}
