#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>


#define LINHA_MAX 256
#define MAX_RESULTADOS 20000

typedef struct {
    char data[20];
    float insolacao;
    float precipitacao;
    float temp_max;
    float temp_min;
    float umidade;
    float vento;
} Medicao;

typedef struct Nodo {
    Medicao medicao;
    struct Nodo* prox;
} Nodo;

char datas_encontradas[MAX_RESULTADOS][20];
int total_resultados = 0;
int total_amostras = 0;
int amostras_atuais = 0;

Nodo* inserir_ordenado(Nodo* lista, Medicao m) {
    Nodo* novo = (Nodo*)malloc(sizeof(Nodo));
    novo->medicao = m;
    novo->prox = NULL;

    if (!lista || strcmp(m.data, lista->medicao.data) < 0) {
        novo->prox = lista;
        return novo;
    }

    Nodo* atual = lista;
    while (atual->prox && strcmp(m.data, atual->prox->medicao.data) > 0)
        atual = atual->prox;

    novo->prox = atual->prox;
    atual->prox = novo;
    return lista;
}

Nodo* remover_por_data(Nodo* lista, const char* data) {
    Nodo *atual = lista, *anterior = NULL;
    while (atual) {
        if (strcmp(atual->medicao.data, data) == 0) {
            if (anterior)
                anterior->prox = atual->prox;
            else
                lista = atual->prox;
            free(atual);
            amostras_atuais--;
            return lista;
        }
        anterior = atual;
        atual = atual->prox;
    }
    return lista;
}

void imprimir_lista(Nodo* lista) {
    while (lista) {
        Medicao m = lista->medicao;
        printf("Data: %s | Insolacao: %.1f | Precipitacao: %.1f | Temp Max: %.1f | Temp Min: %.1f | Umidade: %.1f | Vento: %.1f\n",
               m.data, m.insolacao, m.precipitacao, m.temp_max, m.temp_min, m.umidade, m.vento);
        lista = lista->prox;
    }
}

void buscar_por_valor(Nodo* lista, int campo, void* valor, int* contador) {
    *contador = 0;
    while (lista && *contador < MAX_RESULTADOS) {
        Medicao m = lista->medicao;
        int match = 0;
        switch (campo) {
            case 1: match = strstr(m.data, (char*)valor) != NULL; break;
            case 2: match = m.insolacao == *(float*)valor; break;
            case 3: match = m.precipitacao == *(float*)valor; break;
            case 4: match = m.temp_max == *(float*)valor; break;
            case 5: match = m.temp_min == *(float*)valor; break;
            case 6: match = m.umidade == *(float*)valor; break;
            case 7: match = m.vento == *(float*)valor; break;
        }
        if (match) {
            strcpy(datas_encontradas[*contador], m.data);
            (*contador)++;
        }
        lista = lista->prox;
    }
}

void liberar_lista(Nodo* lista) {
    while (lista) {
        Nodo* temp = lista;
        lista = lista->prox;
        free(temp);
    }
}

Nodo* remover_resultados(Nodo* lista) {
    for (int i = 0; i < total_resultados; i++) {
        lista = remover_por_data(lista, datas_encontradas[i]);
    }
    total_resultados = 0;
    return lista;
}

void carregar_csv(const char* caminho, Nodo** lista) {
    FILE* arquivo = fopen(caminho, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    char linha[LINHA_MAX];
    fgets(linha, LINHA_MAX, arquivo);
    
    clock_t inicio = clock(); // INÍCIO da medição

    while (fgets(linha, LINHA_MAX, arquivo)) {
        Medicao m;
        char* token = strtok(linha, ",");
        if (token) strncpy(m.data, token, 20);
        token = strtok(NULL, ","); m.insolacao = atof(token);
        token = strtok(NULL, ","); m.precipitacao = atof(token);
        token = strtok(NULL, ","); m.temp_max = atof(token);
        token = strtok(NULL, ","); m.temp_min = atof(token);
        token = strtok(NULL, ","); m.umidade = atof(token);
        token = strtok(NULL, ","); m.vento = atof(token);
        *lista = inserir_ordenado(*lista, m);
        total_amostras++;
        amostras_atuais++;
    }
    
	clock_t fim = clock(); // FIM da medição
	printf("Tempo de insercao em massa (CSV): %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
	printf("Memoria estimada apos insercao em massa: %.2f KB\n", amostras_atuais * sizeof(Nodo) / 1024.0);


	
    fclose(arquivo);
}

void calcular_estatisticas(Nodo* lista) {
    if (!lista) {
        printf("Dataset vazio. Nenhuma estatistica disponivel.\n");
        return;
    }

    float s1=0,s2=0,s3=0,s4=0,s5=0,s6=0;
    float s1q=0,s2q=0,s3q=0,s4q=0,s5q=0,s6q=0;
    float min1=9999,max1=-9999,min2=9999,max2=-9999,min3=9999,max3=-9999;
    float min4=9999,max4=-9999,min5=9999,max5=-9999,min6=9999,max6=-9999;
    int cont = 0;

    while (lista) {
        Medicao m = lista->medicao;
        s1 += m.insolacao; s1q += m.insolacao * m.insolacao;
        s2 += m.precipitacao; s2q += m.precipitacao * m.precipitacao;
        s3 += m.temp_max; s3q += m.temp_max * m.temp_max;
        s4 += m.temp_min; s4q += m.temp_min * m.temp_min;
        s5 += m.umidade; s5q += m.umidade * m.umidade;
        s6 += m.vento; s6q += m.vento * m.vento;
        if (m.insolacao < min1) min1 = m.insolacao;
        if (m.insolacao > max1) max1 = m.insolacao;
        if (m.precipitacao < min2) min2 = m.precipitacao;
        if (m.precipitacao > max2) max2 = m.precipitacao;
        if (m.temp_max < min3) min3 = m.temp_max;
        if (m.temp_max > max3) max3 = m.temp_max;
        if (m.temp_min < min4) min4 = m.temp_min;
        if (m.temp_min > max4) max4 = m.temp_min;
        if (m.umidade < min5) min5 = m.umidade;
        if (m.umidade > max5) max5 = m.umidade;
        if (m.vento < min6) min6 = m.vento;
        if (m.vento > max6) max6 = m.vento;
        cont++;
        lista = lista->prox;
    }

    printf("\nTotal de registros: %d\n\n", cont);
    printf("%-15s %-10s %-10s %-10s %-15s\n", "Variavel", "Minimo", "Media", "Maximo", "Desvio padrao");
printf("-------------------------------------------------------------\n");
printf("%-15s %-10.2f %-10.2f %-10.2f %-15.2f\n", "Insolacao", min1, s1/cont, max1, sqrt(s1q/cont - pow(s1/cont, 2)));
printf("%-15s %-10.2f %-10.2f %-10.2f %-15.2f\n", "Precipitacao", min2, s2/cont, max2, sqrt(s2q/cont - pow(s2/cont, 2)));
printf("%-15s %-10.2f %-10.2f %-10.2f %-15.2f\n", "Temp Max", min3, s3/cont, max3, sqrt(s3q/cont - pow(s3/cont, 2)));
printf("%-15s %-10.2f %-10.2f %-10.2f %-15.2f\n", "Temp Min", min4, s4/cont, max4, sqrt(s4q/cont - pow(s4/cont, 2)));
printf("%-15s %-10.2f %-10.2f %-10.2f %-15.2f\n", "Umidade", min5, s5/cont, max5, sqrt(s5q/cont - pow(s5/cont, 2)));
printf("%-15s %-10.2f %-10.2f %-10.2f %-15.2f\n", "Vento", min6, s6/cont, max6, sqrt(s6q/cont - pow(s6/cont, 2)));

}

void mostrar_menu() {
    double memoria_estimada = amostras_atuais * sizeof(Nodo) / 1024.0;

    printf("\n========== MENU ==========\n");
    printf("Total de amostras coletadas: %d\n", total_amostras);
    printf("Memoria estimada: %.2f KB\n", memoria_estimada);
    printf("Amostras atuais: %d\n", amostras_atuais);
    printf("==========================\n");
    printf("1 - Buscar por campo com valor exato\n");
    printf("2 - Inserir nova medicao manual\n");
    printf("3 - Remover medicao por data\n");
    printf("4 - Visualizar dataset\n");
    printf("5 - Sobre o dataset\n");
    printf("6 - Limpar dataset\n");
    printf("7 - Estatisticas do dataset\n");
    printf("0 - Sair\n");
    printf("==========================\n");
    printf("Opcao: ");
}

int main() {
    Nodo* lista = NULL;
    carregar_csv("d1dos_meteorologicos_tratado (1).csv", &lista);
    printf("Dados carregados com sucesso.\n");

    int opcao;
    do {
        mostrar_menu();
        scanf("%d", &opcao);
        getchar();

        if (opcao == 1) {
            int campo;
            total_resultados = 0;
            printf("\nBuscar por valor exato em:\n");
            printf("1. Data\n2. Insolacao\n3. Precipitacao\n4. Temp Max\n5. Temp Min\n6. Umidade\n7. Vento\nEscolha: ");
            scanf("%d", &campo);
            getchar();

            if (campo == 1) {
                char valor[20];
                printf("Digite a data (YYYY-MM-DD): ");
                scanf("%s", valor);
                buscar_por_valor(lista, campo, valor, &total_resultados);
            } 
			else {
    			float valor;
    			printf("Digite o valor exato a buscar: ");
    			scanf("%f", &valor);

    			int repeticoes = 1000;
    			clock_t inicio = clock();

    			for (int i = 0; i < repeticoes; i++) {
        		total_resultados = 0;
        		buscar_por_valor(lista, campo, &valor, &total_resultados);
    		}
			
    clock_t fim = clock();
    double tempo_medio = (double)(fim - inicio) / CLOCKS_PER_SEC / repeticoes;
    printf("Tempo medio por busca: %.6f segundos\n", tempo_medio);
}


            printf("\nTotal de registros encontrados: %d\n", total_resultados);
            if (total_resultados > 0) {
                printf("\nDeseja apagar todos os registros listados? (sim/nao): ");
                char resposta[10];
                scanf("%s", resposta);
               if (strcmp(resposta, "sim") == 0 || strcmp(resposta, "SIM") == 0) {
    int removidos = total_resultados; // salva antes de zerar
    clock_t inicio = clock();
    lista = remover_resultados(lista); // aqui total_resultados vira 0
    clock_t fim = clock();

    amostras_atuais -= removidos;

    printf("Tempo de remocao em massa: %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
    printf("Total de registros removidos: %d\n", removidos);
    printf("Memoria liberada nesta operacao: %.2f KB\n", removidos * sizeof(Nodo) / 1024.0);
    printf("Registros apagados com sucesso.\n");
}

            }

        } 
		else if (opcao == 2) {
            Medicao m;
            printf("Digite a data (YYYY-MM-DD): "); scanf("%s", m.data);
            printf("Insolacao (h): "); scanf("%f", &m.insolacao);
            printf("Precipitacao (mm): "); scanf("%f", &m.precipitacao);
            printf("Temp Max (C): "); scanf("%f", &m.temp_max);
            printf("Temp Min (C): "); scanf("%f", &m.temp_min);
            printf("Umidade (%%): "); scanf("%f", &m.umidade);
            printf("Vento (m/s): "); scanf("%f", &m.vento);
            lista = inserir_ordenado(lista, m);
            amostras_atuais++;
            printf("Medicao inserida com sucesso.\n");
            printf("Memoria estimada nesta insercao: %.2f KB\n", sizeof(Nodo) / 1024.0);

        } else if (opcao == 3) {
   			char data[20];
    		printf("Digite a data da medicao a remover: ");
    		scanf("%s", data);

    		clock_t inicio = clock();
    		lista = remover_por_data(lista, data);
    		clock_t fim = clock();

    		printf("Tempo para remover medicao: %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
    		printf("Medicao removida se existente.\n");
    		printf("Memoria liberada nesta operacao: %.2f KB\n", sizeof(Nodo) / 1024.0);
		}		
 else if (opcao == 4) {
        	
    			clock_t inicio = clock();

    			imprimir_lista(lista);

    			clock_t fim = clock();
    			printf("Tempo para visualizar o dataset: %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
		}

		
		else if (opcao == 5) {
            printf("\n============= SOBRE O DATASET =============\n");
            printf("Local de coleta: Manaus - AM (INMET)\n");
            printf("Periodo dos dados: 1995-01-01 a 2025-01-01\n");
            printf("Total de amostras coletadas: %d\n", total_amostras);
            printf("Variaveis: Data, Insolacao, Precipitacao, Temp Max/Min, Umidade, Vento\n");
            printf("===========================================\n\n");

        } else if (opcao == 6) {
            char confirmacao[10];
            printf("Tem certeza que deseja apagar tudo? (sim/nao): ");
            scanf("%s", confirmacao);
            if (strcmp(confirmacao, "sim") == 0 || strcmp(confirmacao, "SIM") == 0) {
    clock_t inicio = clock();
printf("Memoria liberada com a limpeza: %.2f KB\n", amostras_atuais * sizeof(Nodo) / 1024.0);
    liberar_lista(lista);
    lista = NULL;
    amostras_atuais = 0;
    total_resultados = 0;

    clock_t fim = clock();
    printf("Tempo para limpar o dataset: %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);

    printf("Todos os dados foram apagados.\n");
}
 
			
			else {
                printf("Operacao cancelada.\n");
            }

        } else if (opcao == 7) {
   			clock_t inicio = clock();

    		calcular_estatisticas(lista);

    		clock_t fim = clock();
    		printf("\nTempo para calcular estatisticas: %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
		}			
 else if (opcao == 0) {
            printf("Programa encerrado.\n");
        }
    } while (opcao != 0);

    liberar_lista(lista);
    return 0;
}