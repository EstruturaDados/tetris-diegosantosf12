#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CAP_FILA 5

// Representa uma peça do Tetris: tipo (I,O,T,L) e id único
typedef struct {
    char nome;   // Tipo da peça: 'I', 'O', 'T', 'L'
    int id;      // Identificador único incremental
} Peca;

// Fila circular de peças futuras
typedef struct {
    Peca itens[CAP_FILA];
    int inicio;    // Índice do primeiro elemento (frente)
    int fim;       // Índice da posição de inserção (após o último)
    int tamanho;   // Quantos elementos estão na fila (0..CAP_FILA)
} Fila;

// Gera uma peça automaticamente com tipo aleatório e id sequencial
Peca gerarPeca() {
    static int nextId = 1;
    const char tipos[] = {'I', 'O', 'T', 'L'};
    int idx = rand() % 4;
    Peca p;
    p.nome = tipos[idx];
    p.id = nextId++;
    return p;
}

// Inicializa a fila circular já com CAP_FILA peças (fila cheia)
void inicializarFila(Fila* f) {
    f->inicio = 0;
    f->fim = 0;          // Quando tamanho == CAP_FILA, fim aponta para a próxima posição lógica
    f->tamanho = 0;
    // Preenche com 5 peças geradas automaticamente
    for (int i = 0; i < CAP_FILA; i++) {
        // enqueue manual respeitando circularidade
        f->itens[f->fim] = gerarPeca();
        f->fim = (f->fim + 1) % CAP_FILA;
        f->tamanho++;
    }
}

// Enfileira (enqueue) uma nova peça no final da fila (se houver espaço)
int enqueue(Fila* f, Peca p) {
    if (f->tamanho == CAP_FILA) return 0; // Fila cheia
    f->itens[f->fim] = p;
    f->fim = (f->fim + 1) % CAP_FILA;
    f->tamanho++;
    return 1;
}

// Desenfileira (dequeue) a peça da frente (se houver peça)
int dequeue(Fila* f, Peca* removida) {
    if (f->tamanho == 0) return 0; // Fila vazia
    *removida = f->itens[f->inicio];
    f->inicio = (f->inicio + 1) % CAP_FILA;
    f->tamanho--;
    return 1;
}

// Exibe o estado atual da fila em ordem (frente -> fim)
void exibirFila(const Fila* f) {
    printf("\nFila de pecas (frente -> fim) [tamanho=%d]:\n", f->tamanho);
    for (int i = 0, idx = f->inicio; i < f->tamanho; i++, idx = (idx + 1) % CAP_FILA) {
        printf("(%c, id=%d)%s", f->itens[idx].nome, f->itens[idx].id, (i < f->tamanho - 1) ? "  ->  " : "\n");
    }
}

int main() {
    srand((unsigned)time(NULL));

    Fila fila;
    inicializarFila(&fila);

    int opcao;
    do {
        printf("\n--- Nível Novato: Fila de Peças Futuras ---\n");
        printf("1 - Visualizar fila\n");
        printf("2 - Jogar (remover) peça da frente\n");
        printf("3 - Inserir nova peça automaticamente no final\n");
        printf("0 - Sair\n");
        printf("Escolha: ");
        if (scanf("%d", &opcao) != 1) { opcao = 0; }
        
        if (opcao == 1) {
            // Visualização da fila
            exibirFila(&fila);
        } else if (opcao == 2) {
            // Remove a peça da frente
            Peca r;
            if (dequeue(&fila, &r)) {
                printf("Jogou a peça da frente: (%c, id=%d)\n", r.nome, r.id);
            } else {
                printf("Fila vazia, nada para jogar.\n");
            }
        } else if (opcao == 3) {
            // Insere uma nova peça gerada automaticamente no final
            if (fila.tamanho == CAP_FILA) {
                printf("Fila ja esta cheia. Primeiro jogue uma peca.\n");
            } else {
                Peca nova = gerarPeca();
                enqueue(&fila, nova);
                printf("Inserida nova peca no final: (%c, id=%d)\n", nova.nome, nova.id);
            }
        } else if (opcao == 0) {
            printf("Saindo...\n");
        } else {
            printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    return 0;
}