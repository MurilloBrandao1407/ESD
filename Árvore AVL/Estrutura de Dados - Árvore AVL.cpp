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

typedef struct AVLNode {
    Medicao medicao;
    struct AVLNode* esquerda;
    struct AVLNode* direita;
    int altura;
} AVLNode;

typedef int (*FiltroBuscaValor)(Medicao m, void* valor);

char datas_encontradas[MAX_RESULTADOS][20];
int total_resultados = 0;

int nodes_criados = 0;

int total_amostras = 0;         // total inicial carregado do CSV
int amostras_atuais = 0;        // amostras atuais na árvore AVL

int altura(AVLNode* no) {
    return no ? no->altura : 0;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int fator_balanceamento(AVLNode* no) {
    return no ? altura(no->esquerda) - altura(no->direita) : 0;
}

AVLNode* rotacao_direita(AVLNode* y) {
    AVLNode* x = y->esquerda;
    AVLNode* T2 = x->direita;

    x->direita = y;
    y->esquerda = T2;

    y->altura = max(altura(y->esquerda), altura(y->direita)) + 1;
    x->altura = max(altura(x->esquerda), altura(x->direita)) + 1;

    return x;
}

AVLNode* rotacao_esquerda(AVLNode* x) {
    AVLNode* y = x->direita;
    AVLNode* T2 = y->esquerda;

    y->esquerda = x;
    x->direita = T2;

    x->altura = max(altura(x->esquerda), altura(x->direita)) + 1;
    y->altura = max(altura(y->esquerda), altura(y->direita)) + 1;

    return y;
}

AVLNode* criar_no(Medicao medicao) {
    AVLNode* no = (AVLNode*)malloc(sizeof(AVLNode));
    nodes_criados++;  // <== conta cada nó criado
    no->medicao = medicao;
    no->esquerda = no->direita = NULL;
    no->altura = 1;
    return no;
}

AVLNode* inserir(AVLNode* raiz, Medicao medicao) {
    if (raiz == NULL) return criar_no(medicao);

    int cmp = strcmp(medicao.data, raiz->medicao.data);
    if (cmp < 0)
        raiz->esquerda = inserir(raiz->esquerda, medicao);
    else if (cmp > 0)
        raiz->direita = inserir(raiz->direita, medicao);
    else
        return raiz;

    raiz->altura = 1 + max(altura(raiz->esquerda), altura(raiz->direita));
    int balance = fator_balanceamento(raiz);

    if (balance > 1 && strcmp(medicao.data, raiz->esquerda->medicao.data) < 0)
        return rotacao_direita(raiz);
    if (balance < -1 && strcmp(medicao.data, raiz->direita->medicao.data) > 0)
        return rotacao_esquerda(raiz);
    if (balance > 1 && strcmp(medicao.data, raiz->esquerda->medicao.data) > 0) {
        raiz->esquerda = rotacao_esquerda(raiz->esquerda);
        return rotacao_direita(raiz);
    }
    if (balance < -1 && strcmp(medicao.data, raiz->direita->medicao.data) < 0) {
        raiz->direita = rotacao_direita(raiz->direita);
        return rotacao_esquerda(raiz);
    }

    return raiz;
}

AVLNode* remover(AVLNode* raiz, char* data) {
    if (!raiz) return raiz;

    int cmp = strcmp(data, raiz->medicao.data);
    if (cmp < 0)
        raiz->esquerda = remover(raiz->esquerda, data);
    else if (cmp > 0)
        raiz->direita = remover(raiz->direita, data);
    else {
        if (!raiz->esquerda || !raiz->direita) {
            AVLNode* temp = raiz->esquerda ? raiz->esquerda : raiz->direita;
            free(raiz);
            nodes_criados--;  // <== aqui atualiza corretamente
            return temp;
        }
        AVLNode* temp = raiz->direita;
        while (temp->esquerda) temp = temp->esquerda;
        raiz->medicao = temp->medicao;
        raiz->direita = remover(raiz->direita, temp->medicao.data);
    }

    raiz->altura = 1 + max(altura(raiz->esquerda), altura(raiz->direita));
    int balance = fator_balanceamento(raiz);

    if (balance > 1 && fator_balanceamento(raiz->esquerda) >= 0)
        return rotacao_direita(raiz);
    if (balance > 1 && fator_balanceamento(raiz->esquerda) < 0) {
        raiz->esquerda = rotacao_esquerda(raiz->esquerda);
        return rotacao_direita(raiz);
    }
    if (balance < -1 && fator_balanceamento(raiz->direita) <= 0)
        return rotacao_esquerda(raiz);
    if (balance < -1 && fator_balanceamento(raiz->direita) > 0) {
        raiz->direita = rotacao_direita(raiz->direita);
        return rotacao_esquerda(raiz);
    }

    return raiz;
}

// ----------- Filtros por igualdade -----------
int busca_por_data(Medicao m, void* valor) { return strstr(m.data, (char*)valor) != NULL; }
int busca_insolacao_igual(Medicao m, void* valor) { return m.insolacao == *(float*)valor; }
int busca_precipitacao_igual(Medicao m, void* valor) { return m.precipitacao == *(float*)valor; }
int busca_temp_maxima_igual(Medicao m, void* valor) { return m.temp_max == *(float*)valor; }
int busca_temp_minima_igual(Medicao m, void* valor) { return m.temp_min == *(float*)valor; }
int busca_umidade_igual(Medicao m, void* valor) { return m.umidade == *(float*)valor; }
int busca_vento_igual(Medicao m, void* valor) { return m.vento == *(float*)valor; }

void buscar_e_salvar(AVLNode* raiz, FiltroBuscaValor filtro, void* valor, int* contador) {
    if (!raiz || *contador >= MAX_RESULTADOS) return;

    buscar_e_salvar(raiz->esquerda, filtro, valor, contador);

    if (*contador < MAX_RESULTADOS && filtro(raiz->medicao, valor)) {
        strcpy(datas_encontradas[*contador], raiz->medicao.data);
        (*contador)++;
    }

    buscar_e_salvar(raiz->direita, filtro, valor, contador);
}

AVLNode* remover_resultados(AVLNode* raiz) {
  for (int i = 0; i < total_resultados; i++) {
        raiz = remover(raiz, datas_encontradas[i]);
        amostras_atuais--;  // decrementa para cada remoção
    }
    total_resultados = 0;
    return raiz;
}
void mostrar_menu() {
    printf("\n=========== MENU ===========\n");
    printf("Total de amostras coletadas: %d\n", total_amostras);
    printf("Amostras atuais: %d\n", amostras_atuais);
    printf("Memoria estimada (AVL): %.2f KB\n", (nodes_criados * sizeof(AVLNode)) / 1024.0);
    printf("============================\n");
    printf("1 - Buscar por campo com valor exato\n");
    printf("2 - Inserir nova medicao manual\n");
    printf("3 - Remover medicao por data\n");
    printf("4 - Visualizar dataset\n");
    printf("5 - Sobre o dataset\n");
    printf("6 - Limpar dataset\n");
    printf("7 - Estatisticas do dataset\n");
    printf("0 - Sair\n");
    printf("============================\n");
    printf("Opcao: ");
}


void carregar_csv(const char* caminho, AVLNode** raiz) {
    FILE* arquivo = fopen(caminho, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return;
    }
 	clock_t inicio = clock();  // <-- INÍCIO
    char linha[LINHA_MAX];
    fgets(linha, LINHA_MAX, arquivo); // ignora cabecalho

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
        *raiz = inserir(*raiz, m);
        total_amostras++;
		amostras_atuais++;

    }

    fclose(arquivo);
    clock_t fim = clock();  // <-- FIM
    printf("Tempo de insercao em massa (CSV): %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
    int memoria_estimativa = nodes_criados * sizeof(AVLNode);
	printf("Memoria estimada (AVL): %.2f KB\n", memoria_estimativa / 1024.0);

}

void imprimir_em_ordem(AVLNode* raiz) {
    if (!raiz) return;

    imprimir_em_ordem(raiz->esquerda);

    printf("Data: %s | Insolacao: %.1f | Precipitacao: %.1f | Temp Max: %.1f | Temp Min: %.1f | Umidade: %.1f | Vento: %.1f\n",
           raiz->medicao.data,
           raiz->medicao.insolacao,
           raiz->medicao.precipitacao,
           raiz->medicao.temp_max,
           raiz->medicao.temp_min,
           raiz->medicao.umidade,
           raiz->medicao.vento);

    imprimir_em_ordem(raiz->direita);
}

	void liberar_arvore(AVLNode* raiz) {
    	if (!raiz) return;
    	liberar_arvore(raiz->esquerda);
    	liberar_arvore(raiz->direita);
    	free(raiz);
}

void calcular_medias(AVLNode* raiz, float* soma_insol, float* soma_prec, float* soma_max, float* soma_min, float* soma_umid, float* soma_vento, int* cont) {
    if (!raiz) return;

    calcular_medias(raiz->esquerda, soma_insol, soma_prec, soma_max, soma_min, soma_umid, soma_vento, cont);

    *soma_insol += raiz->medicao.insolacao;
    *soma_prec  += raiz->medicao.precipitacao;
    *soma_max   += raiz->medicao.temp_max;
    *soma_min   += raiz->medicao.temp_min;
    *soma_umid  += raiz->medicao.umidade;
    *soma_vento += raiz->medicao.vento;
    (*cont)++;

    calcular_medias(raiz->direita, soma_insol, soma_prec, soma_max, soma_min, soma_umid, soma_vento, cont);
}

void calcular_estatisticas(
    AVLNode* raiz,
    float* s1, float* s2, float* s3, float* s4, float* s5, float* s6,
    float* s1q, float* s2q, float* s3q, float* s4q, float* s5q, float* s6q,
    float* min1, float* min2, float* min3, float* min4, float* min5, float* min6,
    float* max1, float* max2, float* max3, float* max4, float* max5, float* max6,
    int* cont
) {
    if (!raiz) return;

    Medicao m = raiz->medicao;
    
    if (*cont == 0) {
        *min1 = *max1 = m.insolacao;
        *min2 = *max2 = m.precipitacao;
        *min3 = *max3 = m.temp_max;
        *min4 = *max4 = m.temp_min;
        *min5 = *max5 = m.umidade;
        *min6 = *max6 = m.vento;
    }

    *s1 += m.insolacao; *s1q += m.insolacao * m.insolacao;
    *s2 += m.precipitacao; *s2q += m.precipitacao * m.precipitacao;
    *s3 += m.temp_max; *s3q += m.temp_max * m.temp_max;
    *s4 += m.temp_min; *s4q += m.temp_min * m.temp_min;
    *s5 += m.umidade; *s5q += m.umidade * m.umidade;
    *s6 += m.vento; *s6q += m.vento * m.vento;

    if (m.insolacao < *min1) *min1 = m.insolacao;
    if (m.insolacao > *max1) *max1 = m.insolacao;

    if (m.precipitacao < *min2) *min2 = m.precipitacao;
    if (m.precipitacao > *max2) *max2 = m.precipitacao;

    if (m.temp_max < *min3) *min3 = m.temp_max;
    if (m.temp_max > *max3) *max3 = m.temp_max;

    if (m.temp_min < *min4) *min4 = m.temp_min;
    if (m.temp_min > *max4) *max4 = m.temp_min;

    if (m.umidade < *min5) *min5 = m.umidade;
    if (m.umidade > *max5) *max5 = m.umidade;

    if (m.vento < *min6) *min6 = m.vento;
    if (m.vento > *max6) *max6 = m.vento;

    (*cont)++;

    calcular_estatisticas(raiz->esquerda, s1, s2, s3, s4, s5, s6,
                          s1q, s2q, s3q, s4q, s5q, s6q,
                          min1, min2, min3, min4, min5, min6,
                          max1, max2, max3, max4, max5, max6,
                          cont);
    calcular_estatisticas(raiz->direita, s1, s2, s3, s4, s5, s6,
                          s1q, s2q, s3q, s4q, s5q, s6q,
                          min1, min2, min3, min4, min5, min6,
                          max1, max2, max3, max4, max5, max6,
                          cont);
}

void mostrar_estatistica(const char* nome, float soma, float soma2, float min, float max, int cont) {
    if (cont == 0) {
        printf("%s: Nenhum dado para calcular.\n", nome);
        return;
    }

    float media = soma / cont;
    float variancia = (soma2 / cont) - (media * media);
    float desvio = sqrt(variancia);

    printf("📊 %-13s → min: %6.2f | média: %6.2f | max: %6.2f | desvio: %6.2f\n",
           nome, min, media, max, desvio);
}

int main() {
    AVLNode* raiz = NULL;
    carregar_csv("d1dos_meteorologicos_tratado (1).csv", &raiz);

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

    		AVLNode* atual = raiz;
    		while (atual) {
        	int cmp = strcmp(valor, atual->medicao.data);
        	if (cmp == 0) {
            printf("\n--- Medicao encontrada ---\n");
            printf("Data: %s\n", atual->medicao.data);
            printf("Insolacao: %.1f h\n", atual->medicao.insolacao);
            printf("Precipitacao: %.1f mm\n", atual->medicao.precipitacao);
            printf("Temp Max: %.1f C\n", atual->medicao.temp_max);
            printf("Temp Min: %.1f C\n", atual->medicao.temp_min);
            printf("Umidade: %.1f %%\n", atual->medicao.umidade);
            printf("Vento: %.1f m/s\n", atual->medicao.vento);
            strcpy(datas_encontradas[0], atual->medicao.data);
            total_resultados = 1;
            break;
        } else if (cmp < 0)
            atual = atual->esquerda;
        else
            atual = atual->direita;
    }

    if (total_resultados == 0)
        printf("Data nao encontrada.\n");
}
 else {
                float valor;
                printf("Digite o valor exato a buscar: ");
                scanf("%f", &valor);
                
                clock_t inicio = clock();  // INÍCIO da medição
                switch (campo) {
                    case 2: buscar_e_salvar(raiz, busca_insolacao_igual, &valor, &total_resultados); break;
                    case 3: buscar_e_salvar(raiz, busca_precipitacao_igual, &valor, &total_resultados); break;
                    case 4: buscar_e_salvar(raiz, busca_temp_maxima_igual, &valor, &total_resultados); break;
                    case 5: buscar_e_salvar(raiz, busca_temp_minima_igual, &valor, &total_resultados); break;
                    case 6: buscar_e_salvar(raiz, busca_umidade_igual, &valor, &total_resultados); break;
                    case 7: buscar_e_salvar(raiz, busca_vento_igual, &valor, &total_resultados); break;
                    default: printf("Campo invalido.\n"); 
					break;
                }
                clock_t fim = clock();  // FIM da medição
                printf("Tempo de busca: %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
            }
	
        	printf("\nTotal de registros encontrados: %d\n", total_resultados);

if (total_resultados > 0) {
    printf("\n==== Registros encontrados ====\n");
    for (int i = 0; i < total_resultados; i++) {
        AVLNode* atual = raiz;
        while (atual) {
            int cmp = strcmp(datas_encontradas[i], atual->medicao.data);
            if (cmp == 0) {
                printf("Data: %s | Insolacao: %.1f | Precipitacao: %.1f | Temp Max: %.1f | Temp Min: %.1f | Umidade: %.1f | Vento: %.1f\n",
                       atual->medicao.data,
                       atual->medicao.insolacao,
                       atual->medicao.precipitacao,
                       atual->medicao.temp_max,
                       atual->medicao.temp_min,
                       atual->medicao.umidade,
                       atual->medicao.vento);
                break;
            } else if (cmp < 0) {
                atual = atual->esquerda;
            } else {
                atual = atual->direita;
            }
        }
    }

	printf("\nTotal de registros encontrados: %d\n", total_resultados);
    printf("\nApagar todos os registros listados acima? (Digite 'sim' para confirmar): ");
    char resposta[10];
    scanf("%s", resposta);
    if (strcmp(resposta, "sim") == 0 || strcmp(resposta, "SIM") == 0) {
    int nodes_antes = nodes_criados;

    clock_t inicio = clock();
    raiz = remover_resultados(raiz);
    clock_t fim = clock();

    int nodes_depois = nodes_criados;
    int removidos = nodes_antes - nodes_depois;
    float memoria_liberada_kb = (removidos * sizeof(AVLNode)) / 1024.0;

    printf("Tempo de remocao em massa: %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
    printf("Nos removidos: %d\n", removidos);
    printf("Memoria liberada nesta remocao: %.2f KB\n", memoria_liberada_kb);

    printf("Registros apagados com sucesso.\n");
}
 else 
		{
        	printf("Nenhum registro foi apagado.\n");
    	}
	}
}   	
		else if (opcao == 2) {
    	Medicao m;
    	printf("Digite a data (YYYY-MM-DD): ");
    		scanf("%s", m.data);
    	printf("Insolacao (h): ");
    		scanf("%f", &m.insolacao);
    	printf("Precipitacao (mm): ");
    		scanf("%f", &m.precipitacao);
    	printf("Temperatura maxima (C): ");
    		scanf("%f", &m.temp_max);
    	printf("Temperatura minima (C): ");
    		scanf("%f", &m.temp_min);
    	printf("Umidade relativa (%%): ");
   			scanf("%f", &m.umidade);
    	printf("Velocidade do vento (m/s): ");
    		scanf("%f", &m.vento);

    	int nodes_antes = nodes_criados;
		raiz = inserir(raiz, m);
		amostras_atuais++;
		int nodes_depois = nodes_criados;
		int dif = nodes_depois - nodes_antes;
		float memoria_usada_kb = (dif * sizeof(AVLNode)) / 1024.0;
		
		printf("Nos criados na operacao: %d\n", dif);
		printf("Memoria estimada usada nesta insercao: %.2f KB\n", memoria_usada_kb);

    	printf("\nMedicao inserida com sucesso:\n");
    	printf("Data: %s | Insolacao: %.1f | Precipitacao: %.1f | Temp Max: %.1f | Temp Min: %.1f | Umidade: %.1f | Vento: %.1f\n",
        m.data, m.insolacao, m.precipitacao, m.temp_max, m.temp_min, m.umidade, m.vento);
        
        printf("Nos totais na árvore: %d\n", nodes_criados);
		printf("Memoria estimada (AVL): %.2f KB\n", (nodes_criados * sizeof(AVLNode)) / 1024.0);
}
        else if (opcao == 3) {
    		char data_remover[20];
    		printf("Digite a data da medicao que deseja remover (YYYY-MM-DD): ");
    		scanf("%s", data_remover);

    // Verifica se a data existe antes de remover
    		AVLNode* atual = raiz;
    		int encontrada = 0;
    		while (atual) {
        	int cmp = strcmp(data_remover, atual->medicao.data);
        	if (cmp == 0) {
            	encontrada = 1;
            	break;
        	} else if (cmp < 0)
            	atual = atual->esquerda;
        	else
            	atual = atual->direita;
    	}

    if (encontrada) {
    int nodes_antes = nodes_criados;

    clock_t inicio = clock();
    raiz = remover(raiz, data_remover);
    clock_t fim = clock();

    int nodes_depois = nodes_criados;
    int removidos = nodes_antes - nodes_depois;
    float memoria_liberada_kb = (removidos * sizeof(AVLNode)) / 1024.0;

    printf("Tempo para remover medicao: %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
    printf("Nos removidos: %d\n", removidos);
    printf("Memoria liberada nesta operacao: %.2f KB\n", memoria_liberada_kb);

    amostras_atuais--;

    printf("Medicao removida com sucesso.\n");
}

		   else {
        	printf("Data nao encontrada. Nenhuma medicao removida.\n");
   		}	
	}

		else if (opcao == 4) {
			clock_t inicio = clock();  // INÍCIO da medição
			imprimir_em_ordem(raiz);
			
			 clock_t fim = clock();     // FIM da medição
    		printf("Tempo para visualizar o dataset: %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
	}
		
		else if (opcao == 5) {
    		printf("\n============= SOBRE O DATASET =============\n");
    		printf("Local de coleta: Manaus - AM (INMET)\n");
    		printf("Periodo dos dados: 1995-01-01 a 2025-01-01\n");
    		printf("Total de amostras coletadas: %d\n", total_amostras);
    		printf("Variaveis disponiveis:\n");
    		printf(" - Data\n");
    		printf(" - Insolacao (horas de sol por dia)\n");
    		printf(" - Precipitacao (mm)\n");
    		printf(" - Temperatura Maxima (C)\n");
    		printf(" - Temperatura Minima (C)\n");
    		printf(" - Umidade Relativa (%%)\n");
    		printf(" - Velocidade do Vento (m/s)\n");
    		printf("===========================================\n\n");
	}
	
	else if (opcao == 6) {
    	char confirmacao[10];
    	printf("Tem certeza que deseja apagar todo o dataset? (Digite 'sim' para confirmar): ");
    	scanf("%s", confirmacao);
    		for (int i = 0; confirmacao[i]; i++) confirmacao[i] = tolower(confirmacao[i]);

    	if (strcmp(confirmacao, "sim") == 0) {
    		
    		clock_t inicio = clock();

        	liberar_arvore(raiz);
        	raiz = NULL;
        	amostras_atuais = 0;
        	total_resultados = 0;
        	nodes_criados = 0;
        	clock_t fim = clock();
    		printf("Tempo para limpar o dataset: %.3f ms\n", 1000.0 * (fim - inicio) / CLOCKS_PER_SEC);
        	
        	printf("Todos os dados foram apagados do dataset.\n");
    	} 
		else {
        	printf("Operacao de limpeza cancelada.\n");
    	}
}

	else if (opcao == 7) {
    if (raiz == NULL) {
        printf("Dataset vazio. Nenhuma estatistica disponivel.\n");
    } else {
    	
    	clock_t inicio = clock();  // <-- INÍCIO
        float s1=0,s2=0,s3=0,s4=0,s5=0,s6=0;
        float s1q=0,s2q=0,s3q=0,s4q=0,s5q=0,s6q=0;
        float min1=9999,max1=-9999,min2=9999,max2=-9999,min3=9999,max3=-9999;
        float min4=9999,max4=-9999,min5=9999,max5=-9999,min6=9999,max6=-9999;
        int cont = 0;

        calcular_estatisticas(raiz, &s1, &s2, &s3, &s4, &s5, &s6,
                                   &s1q, &s2q, &s3q, &s4q, &s5q, &s6q,
                                   &min1, &min2, &min3, &min4, &min5, &min6,
                                   &max1, &max2, &max3, &max4, &max5, &max6,
                                   &cont);

		clock_t fim = clock();     // <-- FIM
		printf("Tempo para calcular estatísticas: %.6f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
		
        printf("\n=========== ESTATISTICAS DO DATASET ===========\n");
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
}
		else if (opcao == 0) {
    		printf("Programa encerrado\n");
		}
    } while (opcao != 0);

    return 0;
}
