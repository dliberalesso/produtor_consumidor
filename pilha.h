#define TAM 256

typedef struct {
  int memoria[TAM];
  int topo;
} Pilha;

void initPilha(Pilha *p) {
  p->topo = -1;
}

int pilhaVazia(Pilha *p) {
  if(p->topo == -1) return 1;
  return 0;
}

int pilhaCheia(Pilha *p) {
  int limite = TAM - 1;
  if(p->topo == limite) return 1;
  return 0;
}

void empilha(Pilha *p, int elem) {
  if(!pilhaCheia(p)) {
    p->topo++;
    p->memoria[p->topo] = elem;
  }
}

void desempilha(Pilha *p) {
  if(!pilhaVazia(p)) p->topo--;
}

int topoPilha(Pilha *p) {
  return p->memoria[p->topo];
}
