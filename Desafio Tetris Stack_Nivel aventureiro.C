#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CAP_FILA 5
#define CAP_PILHA 3

// Representa uma peça do Tetris
typedef struct {
    char nome;  // 'I', 'O', 'T', 'L'
    int id;     // id único
} Peca;

// Fila circular
typedef struct {
    Peca itens[CAP_FILA];
    int inicio;
    int fim;
    int tamanho;
} Fila;

// Pilha linear de reserva (topo aponta para próxima posição livre)
typedef struct {
    Peca itens[CAP_PILHA];
    int topo;   // 0..CAP_PILHA (quantos elementos)
} Pilha;

// Gera peça automática
Peca gerarPeca() {
    static int nextId = 1;
    const char tipos[] = {'I', 'O', 'T', 'L'};
    int idx = rand() % 4;
    Peca p;
    p.nome = tipos[idx];
    p.id = nextId++;
    return p;
}

// Inicializa fila com 5 peças
void inicializarFila(Fila* f) {
    f->inicio = 0; f->fim = 0; f->tamanho = 0;
    for (int i = 0; i < CAP_FILA; i++) {
        f->itens[f->fim] = gerarPeca();
        f->fim = (f->fim + 1) % CAP_FILA;
        f->tamanho++;
    }
}

// Inicializa pilha vazia
void inicializarPilha(Pilha* p) { p->topo = 0; }

// Enfileira no final
int enqueue(Fila* f, Peca p) {
    if (f->tamanho == CAP_FILA) return 0;
    f->itens[f->fim] = p;
    f->fim = (f->fim + 1) % CAP_FILA;
    f->tamanho++;
    return 1;
}

// Desenfileira da frente
int dequeue(Fila* f, Peca* removida) {
    if (f->tamanho == 0) return 0;
    *removida = f->itens[f->inicio];
    f->inicio = (f->inicio + 1) % CAP_FILA;
    f->tamanho--;
    return 1;
}

// Exibe fila
void exibirFila(const Fila* f) {
    printf("\nFila [tamanho=%d]:\n", f->tamanho);
    for (int i = 0, idx = f->inicio; i < f->tamanho; i++, idx = (idx + 1) % CAP_FILA) {
        printf("(%c, id=%d)%s", f->itens[idx].nome, f->itens[idx].id, (i < f->tamanho - 1) ? " -> " : "\n");
    }
}

// Exibe pilha (base -> topo-1)
void exibirPilha(const Pilha* p) {
    printf("Pilha reserva [tamanho=%d]:\n", p->topo);
    for (int i = 0; i < p->topo; i++) {
        printf("[%d] (%c, id=%d)%s", i, p->itens[i].nome, p->itens[i].id, (i < p->topo - 1) ? " | " : "\n");
    }
}

// Push na pilha
int push(Pilha* p, Peca x) {
    if (p->topo == CAP_PILHA) return 0;
    p->itens[p->topo++] = x;
    return 1;
}

// Pop da pilha
int pop(Pilha* p, Peca* x) {
    if (p->topo == 0) return 0;
    *x = p->itens[--p->topo];
    return 1;
}

int main() {
    srand((unsigned)time(NULL));

    Fila fila; Pilha reserva;
    inicializarFila(&fila);
    inicializarPilha(&reserva);

    int opcao;
    do {
        printf("\n--- Nível Aventureiro: Fila + Pilha de Reserva ---\n");
        printf("1 - Jogar peça (dequeue + auto enqueue)\n");
        printf("2 - Reservar peça (push da frente da fila)\n");
        printf("3 - Usar peça reservada (pop substitui a frente da fila)\n");
        printf("0 - Sair\n");
        printf("Escolha: ");
        if (scanf("%d", &opcao) != 1) { opcao = 0; }

        if (opcao == 1) {
            // Jogar: remove da frente e insere nova peça automaticamente para manter fila cheia
            Peca jogada;
            if (dequeue(&fila, &jogada)) {
                printf("Jogou: (%c, id=%d)\n", jogada.nome, jogada.id);
                Peca nova = gerarPeca();
                enqueue(&fila, nova);
                printf("Nova gerada e inserida no final: (%c, id=%d)\n", nova.nome, nova.id);
            } else {
                printf("Fila vazia.\n");
            }
            exibirFila(&fila);
            exibirPilha(&reserva);
        } else if (opcao == 2) {
            // Reservar: tira a frente da fila e empilha; repõe automaticamente na fila
            Peca frente;
            if (dequeue(&fila, &frente)) {
                if (push(&reserva, frente)) {
                    printf("Reservou: (%c, id=%d)\n", frente.nome, frente.id);
                    Peca nova = gerarPeca();
                    enqueue(&fila, nova);
                    printf("Reposicao automatica: (%c, id=%d)\n", nova.nome, nova.id);
                } else {
                    // Se pilha cheia, devolve peça à frente (undo simples local)
                    fila.inicio = (fila.inicio - 1 + CAP_FILA) % CAP_FILA;
                    fila.itens[fila.inicio] = frente;
                    fila.tamanho++;
                    printf("Pilha cheia. Reserva cancelada.\n");
                }
            }
            exibirFila(&fila);
            exibirPilha(&reserva);
        } else if (opcao == 3) {
            // Usar reserva: pop e coloca a peça reservada na frente da fila (substitui sem alterar tamanho)
            Peca usada;
            if (pop(&reserva, &usada)) {
                fila.itens[fila.inicio] = usada; // Substitui a próxima peça a ser jogada
                printf("Usou reserva na frente da fila: (%c, id=%d)\n", usada.nome, usada.id);
            } else {
                printf("Pilha vazia. Nao ha reserva.\n");
            }
            exibirFila(&fila);
            exibirPilha(&reserva);
        } else if (opcao == 0) {
            printf("Saindo...\n");
        } else {
            printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    return 0;
}