#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CAP_FILA 5
#define CAP_PILHA 3
#define CAP_HIST 50   // Capacidade de histórico (desfazer)

typedef struct {
    char nome;  // 'I','O','T','L'
    int id;
} Peca;

typedef struct {
    Peca itens[CAP_FILA];
    int inicio, fim, tamanho;
} Fila;

typedef struct {
    Peca itens[CAP_PILHA];
    int topo;
} Pilha;

// Tipos de ação para desfazer
typedef enum {
    AC_PLAY,        // Jogar peça (dequeue + auto enqueue)
    AC_RESERVE,     // Reservar peça (push)
    AC_USE_RESERVE, // Usar peça reservada (pop -> frente)
    AC_SWAP,        // Trocar topo da pilha com frente da fila
    AC_INVERT       // Inverter fila
} TipoAcao;

// Registro de ação para desfazer
typedef struct {
    TipoAcao tipo;
    // Dados auxiliares para desfazer
    Peca a;           // Peça principal
    Peca b;           // Peça secundária (quando necessário)
    // Snapshot para inversão
    Peca filaSnap[CAP_FILA];
    int inicioSnap, fimSnap, tamanhoSnap;
    Peca pilhaSnap[CAP_PILHA];
    int topoSnap;
} Acao;

// Pilha de histórico de ações
typedef struct {
    Acao itens[CAP_HIST];
    int topo;
} Historico;

// --------- Utilidades ---------

Peca gerarPeca() {
    static int nextId = 1;
    const char tipos[] = {'I','O','T','L'};
    Peca p; p.nome = tipos[rand()%4]; p.id = nextId++;
    return p;
}

void inicializarFila(Fila* f) {
    f->inicio = 0; f->fim = 0; f->tamanho = 0;
    for (int i=0;i<CAP_FILA;i++) {
        f->itens[f->fim] = gerarPeca();
        f->fim = (f->fim + 1) % CAP_FILA;
        f->tamanho++;
    }
}

void inicializarPilha(Pilha* p) { p->topo = 0; }
void inicializarHistorico(Historico* h) { h->topo = 0; }

int enqueue(Fila* f, Peca p) {
    if (f->tamanho == CAP_FILA) return 0;
    f->itens[f->fim] = p;
    f->fim = (f->fim + 1) % CAP_FILA;
    f->tamanho++;
    return 1;
}

int dequeue(Fila* f, Peca* rem) {
    if (f->tamanho == 0) return 0;
    *rem = f->itens[f->inicio];
    f->inicio = (f->inicio + 1) % CAP_FILA;
    f->tamanho--;
    return 1;
}

int push(Pilha* p, Peca x) {
    if (p->topo == CAP_PILHA) return 0;
    p->itens[p->topo++] = x; return 1;
}

int pop(Pilha* p, Peca* x) {
    if (p->topo == 0) return 0;
    *x = p->itens[--p->topo]; return 1;
}

void exibirFila(const Fila* f) {
    printf("\nFila [tamanho=%d]:\n", f->tamanho);
    for (int i=0, idx=f->inicio; i<f->tamanho; i++, idx=(idx+1)%CAP_FILA) {
        printf("(%c,id=%d)%s", f->itens[idx].nome, f->itens[idx].id, (i<f->tamanho-1)?" -> ":"\n");
    }
}

void exibirPilha(const Pilha* p) {
    printf("Pilha reserva [tamanho=%d]:\n", p->topo);
    for (int i=0;i<p->topo;i++) {
        printf("[%d](%c,id=%d)%s", i, p->itens[i].nome, p->itens[i].id, (i<p->topo-1)?" | ":"\n");
    }
}

// Salva estado no histórico (para inversão)
void snapshot(Historico* h, const Fila* f, const Pilha* p) {
    if (h->topo == CAP_HIST) return;
    Acao *ac = &h->itens[h->topo++];
    ac->tipo = AC_INVERT;
    // Snapshot da fila
    ac->tamanhoSnap = f->tamanho;
    ac->inicioSnap = f->inicio;
    ac->fimSnap = f->fim;
    for (int i=0;i<CAP_FILA;i++) ac->filaSnap[i] = f->itens[i];
    // Snapshot da pilha
    ac->topoSnap = p->topo;
    for (int i=0;i<CAP_PILHA;i++) ac->pilhaSnap[i] = p->itens[i];
}

// Registra ação simples com até duas peças
void registrar(Historico* h, TipoAcao tipo, Peca a, Peca b) {
    if (h->topo == CAP_HIST) return;
    Acao *ac = &h->itens[h->topo++];
    ac->tipo = tipo;
    ac->a = a;
    ac->b = b;
}

// Desfaz última ação
void desfazer(Historico* h, Fila* f, Pilha* p) {
    if (h->topo == 0) {
        printf("Nada para desfazer.\n");
        return;
    }
    Acao ac = h->itens[--h->topo];

    switch (ac.tipo) {
        case AC_PLAY: {
            // Reverte: remove a peça auto-inserida do final e recoloca a peça jogada na frente
            if (f->tamanho > 0) {
                // Remove último (ajustar fim e tamanho)
                f->fim = (f->fim - 1 + CAP_FILA) % CAP_FILA;
                f->tamanho--;
            }
            // Reinsere jogada na frente (ajusta inicio para trás)
            f->inicio = (f->inicio - 1 + CAP_FILA) % CAP_FILA;
            f->itens[f->inicio] = ac.a; // peça jogada originalmente
            f->tamanho++;
            printf("Desfeito: play\n");
            break;
        }
        case AC_RESERVE: {
            // Reverte reserva: tira do topo da pilha e devolve à frente da fila
            Peca top;
            if (pop(p, &top)) {
                f->inicio = (f->inicio - 1 + CAP_FILA) % CAP_FILA;
                f->itens[f->inicio] = top;
                f->tamanho++;
                // Remove a reposição automática do final
                if (f->tamanho > 0) {
                    f->fim = (f->fim - 1 + CAP_FILA) % CAP_FILA;
                    f->tamanho--;
                }
                printf("Desfeito: reserva\n");
            } else {
                printf("Falha ao desfazer reserva (pilha vazia inesperada).\n");
            }
            break;
        }
        case AC_USE_RESERVE: {
            // Reverte uso de reserva: restaura peça anterior na frente e coloca a usada de volta no topo
            // ac.a = peça anterior da frente, ac.b = peça reservada usada
            f->itens[f->inicio] = ac.a;
            push(p, ac.b);
            printf("Desfeito: uso de reserva\n");
            break;
        }
        case AC_SWAP: {
            // Reverte troca: troca novamente topo e frente
            if (p->topo > 0 && f->tamanho > 0) {
                Peca tmp = p->itens[p->topo - 1];
                p->itens[p->topo - 1] = f->itens[f->inicio];
                f->itens[f->inicio] = tmp;
                printf("Desfeito: troca\n");
            }
            break;
        }
        case AC_INVERT: {
            // Restaura snapshot da fila e pilha
            f->tamanho = ac.tamanhoSnap;
            f->inicio  = ac.inicioSnap;
            f->fim     = ac.fimSnap;
            for (int i=0;i<CAP_FILA;i++) f->itens[i] = ac.filaSnap[i];
            p->topo = ac.topoSnap;
            for (int i=0;i<CAP_PILHA;i++) p->itens[i] = ac.pilhaSnap[i];
            printf("Desfeito: inversao\n");
            break;
        }
        default:
            printf("Acao desconhecida.\n");
    }
}

// Inverte a ordem da fila usando uma pilha auxiliar (capacidade 5)
void inverterFila(Fila* f) {
    // Pilha auxiliar
    Peca aux[CAP_FILA];
    int topo = 0;

    // Empilha todos na ordem atual (frente -> fim)
    for (int i=0;i<f->tamanho;i++) {
        int idx = (f->inicio + i) % CAP_FILA;
        aux[topo++] = f->itens[idx];
    }

    // Desempilha de volta para a fila (invertendo ordem)
    for (int i=0;i<f->tamanho;i++) {
        f->itens[(f->inicio + i) % CAP_FILA] = aux[--topo];
    }
}

int main() {
    srand((unsigned)time(NULL));

    Fila fila; Pilha pilha; Historico hist;
    inicializarFila(&fila);
    inicializarPilha(&pilha);
    inicializarHistorico(&hist);

    int opcao;
    do {
        printf("\n--- Nível Mestre: Integração Total ---\n");
        printf("1 - Jogar peça\n");
        printf("2 - Reservar peça\n");
        printf("3 - Usar peça reservada\n");
        printf("4 - Trocar topo da pilha com frente da fila\n");
        printf("5 - Desfazer última jogada\n");
        printf("6 - Inverter fila com pilha\n");
        printf("7 - Visualizar estado\n");
        printf("0 - Sair\n");
        printf("Escolha: ");
        if (scanf("%d", &opcao) != 1) { opcao = 0; }

        if (opcao == 1) {
            // Jogar: remove frente e insere nova automaticamente (mantendo cheia)
            Peca jogada;
            if (dequeue(&fila, &jogada)) {
                Peca nova = gerarPeca();
                enqueue(&fila, nova);
                registrar(&hist, AC_PLAY, jogada, nova);
                printf("Jogou (%c,id=%d). Nova (%c,id=%d) inserida.\n", jogada.nome, jogada.id, nova.nome, nova.id);
            } else {
                printf("Fila vazia.\n");
            }
        } else if (opcao == 2) {
            // Reservar: tira frente, empilha, repõe nova peça automática
            Peca frente;
            if (dequeue(&fila, &frente)) {
                if (push(&pilha, frente)) {
                    Peca nova = gerarPeca();
                    enqueue(&fila, nova);
                    registrar(&hist, AC_RESERVE, frente, nova);
                    printf("Reservou (%c,id=%d). Reposicao (%c,id=%d).\n", frente.nome, frente.id, nova.nome, nova.id);
                } else {
                    // Se pilha cheia, devolve frente
                    fila.inicio = (fila.inicio - 1 + CAP_FILA) % CAP_FILA;
                    fila.itens[fila.inicio] = frente;
                    fila.tamanho++;
                    printf("Pilha cheia. Reserva cancelada.\n");
                }
            } else {
                printf("Fila vazia.\n");
            }
        } else if (opcao == 3) {
            // Usar reserva: pop e substitui frente
            Peca usada;
            if (pop(&pilha, &usada)) {
                Peca anterior = fila.itens[fila.inicio];
                fila.itens[fila.inicio] = usada;
                registrar(&hist, AC_USE_RESERVE, anterior, usada);
                printf("Usou reserva na frente: (%c,id=%d)\n", usada.nome, usada.id);
            } else {
                printf("Sem reserva.\n");
            }
        } else if (opcao == 4) {
            // Trocar topo da pilha com frente da fila
            if (pilha.topo > 0 && fila.tamanho > 0) {
                Peca topo = pilha.itens[pilha.topo - 1];
                Peca frente = fila.itens[fila.inicio];
                pilha.itens[pilha.topo - 1] = frente;
                fila.itens[fila.inicio] = topo;
                registrar(&hist, AC_SWAP, topo, frente);
                printf("Trocou topo (%c,id=%d) com frente (%c,id=%d).\n", topo.nome, topo.id, frente.nome, frente.id);
            } else {
                printf("Nao ha elementos suficientes para trocar.\n");
            }
        } else if (opcao == 5) {
            // Desfazer última
            desfazer(&hist, &fila, &pilha);
        } else if (opcao == 6) {
            // Inverter fila com snapshot para desfazer
            snapshot(&hist, &fila, &pilha);
            inverterFila(&fila);
            printf("Fila invertida.\n");
        } else if (opcao == 7) {
            // Visualizar
            exibirFila(&fila);
            exibirPilha(&pilha);
        } else if (opcao == 0) {
            printf("Saindo...\n");
        } else {
            printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    return 0;
}