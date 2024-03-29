/*********************************************************************************************
* EDA 2017/2 - ESTRUTURAS DE DADOS E ALGORITMOS (Prof. Fernando W. Cruz)
* Projeto  - Arvores e funcoes de hash
* Verifica corretude de palavras de um arquivo-texto segundo um dicionario carregado em RAM.
 *********************************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <stdbool.h>

/* Tamanho maximo de uma palavra*/
#define TAM_MAX 50
/* Tamanho do vetor dicionario */
#define TAM_DICIO 1000003
/* Máximo de colisões */
#define MAX_COLIS 100
/* dicionario default */
#define NOME_DICIONARIO "dicioPadrao"

/* retornos desse programa */
#define SUCESSO                 0
#define NUM_ARGS_INCORRETO      1
#define ARQDICIO_NAOCARREGADO   2
#define ARQTEXTO_ERROABERTURA   3
#define ARQTEXTO_ERROLEITURA    4
#define ERRO_DICIO_NAOCARREGADO 5


/* structs */
typedef struct palavra{
  char *palavra;
  struct palavra *prox;
  struct palavra *ant;
}palavra;

/* Varável global para dicionario */
palavra dicionario[TAM_DICIO] = { [0 ... 1000002] = NULL };

/* Hash para ser usado no dicionário */
unsigned int DEKHash(const char* str, unsigned int length)
{
   unsigned int hash = length;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
   }

   return hash%TAM_DICIO;
}
/* Fim-hash */

/* Retorna true se a palavra esta no dicionario. Do contrario, retorna false */
bool conferePalavra(const char *pal) {
  palavra *p;
  int i;

  i = DEKHash(pal, strlen(pal));
  p = &dicionario[i];
  if(dicionario[i].palavra != NULL){
    if(!strcmp(dicionario[i].palavra, pal)){
      return true;
    }
    else{
      do{
        p = p->prox;
        if(p!= NULL && !strcmp(p->palavra, pal)){
          return true;
        }
      }while(p != NULL);
    }
  }
  return false;
} /* fim-conferePalavra */

/* Procedimento de inserção caso a posição da hastable já esteja ocupada. */
void insereNaoNula(palavra *dic, char *temp){
  palavra *ant, *novo, *p;
  int cont;

  cont = 0;
  p = dic;
  novo = (palavra *) malloc (sizeof(palavra));
  novo->palavra = (char *) malloc (sizeof(char)*TAM_MAX);
  do{
    ant = p;
    p = p->prox;
    ant->prox = p;
    if(p != NULL){
      p->ant = ant;
    }
  }while(p != NULL);
  strcpy(novo->palavra, temp);
  ant->prox = novo;
  novo->ant = ant;
} /* fim-insereNaoNula */

/* Carrega dicionario na memoria. Retorna true se sucesso; senao retorna false. */
bool carregaDicionario(){
  int i;
  FILE *fd;
  char *temp;
  palavra *novo, *p;

  fd = fopen(NOME_DICIONARIO, "r");
  if(fd != NULL){
    temp = (char *) malloc (sizeof(char)*TAM_MAX);
    while(fgets(temp, TAM_MAX, fd)){
      temp[strlen(temp)-1] = '\0';
      i = DEKHash(temp, strlen(temp));
      if(dicionario[i].palavra != NULL){
        p = &dicionario[i];
        insereNaoNula(p, temp);
      }
      else if (dicionario[i].palavra == NULL){
        dicionario[i].palavra = (char *) malloc (sizeof(char)*TAM_MAX);
        strcpy(dicionario[i].palavra, temp);
        dicionario[i].prox = NULL;
        dicionario[i].ant = NULL;
      }
    }
    free(temp);
    fclose(fd);
    return true;
  }
  return false;
} /* fim-carregaDicionario */


/* Retorna qtde palavras do dicionario, se carregado; senao carregado retorna zero */
unsigned int contaPalavrasDic(void) {
  FILE *fd;
  char temp[TAM_MAX];
  int i;

  i = 0;
  fd = fopen(NOME_DICIONARIO, "r");
  if(fd != NULL){
    while(fgets(temp, TAM_MAX, fd)){
      i++;
    }
    fclose(fd);
    return i;
  }
  return 0;
} /* fim-contaPalavrasDic */


/* Descarrega dicionario da memoria. Retorna true se ok e false se algo deu errado */
bool descarregaDicionario(void) {
  FILE *fd;
  int i;
  char temp[TAM_MAX];
  palavra *p, *ant;

  fd = fopen(NOME_DICIONARIO, "r");
  if(fd != NULL){
      while(fgets(temp, TAM_MAX, fd)){
        i = DEKHash(temp, strlen(temp));
        if(dicionario[i].palavra != NULL){
          p = &dicionario[i];
          do{
            ant = p;
            p = p->prox;
            free(ant->palavra);
          }while(p != NULL);
        }
      }
      return true;
  }
  return false;
} /* fim-descarregaDicionario */

/* Retorna o numero de segundos entre a e b */
double calcula_tempo(const struct rusage *b, const struct rusage *a) {
    if (b == NULL || a == NULL)
        return 0;
    else
        return ((((a->ru_utime.tv_sec * 1000000 + a->ru_utime.tv_usec) -
                 (b->ru_utime.tv_sec * 1000000 + b->ru_utime.tv_usec)) +
                ((a->ru_stime.tv_sec * 1000000 + a->ru_stime.tv_usec) -
                 (b->ru_stime.tv_sec * 1000000 + b->ru_stime.tv_usec)))
                / 1000000.0);
} /* fim-calcula_tempo */


int main(int argc, char *argv[]) {
    struct rusage tempo_inicial, tempo_final; /* structs para dados de tempo do processo */
    double tempo_carga = 0.0, tempo_check = 0.0, tempo_calc_tamanho_dic = 0.0, tempo_limpeza_memoria = 0.0;
    /* determina qual dicionario usar; o default eh usar o arquivo dicioPadrao */
    int  indice, totPalErradas, totPalavras, c;
    char *palavra;
    bool palavraErrada, descarga, carga;
    unsigned int qtdePalavrasDic;
    char *arqTexto;
    FILE *fd;

    /* Confere se o numero de argumentos de chamada estah correto */
    if (argc != 2 && argc != 3) {
        printf("Uso: %s [nomeArquivoDicionario] nomeArquivoTexto\n", argv[0]);
        return NUM_ARGS_INCORRETO;
    } /* fim-if */

    /* carrega o dicionario na memoria, c/ anotacao de tempos inicial e final */
    getrusage(RUSAGE_SELF, &tempo_inicial);
       carga = carregaDicionario();
    getrusage(RUSAGE_SELF, &tempo_final);

    /* aborta se o dicionario nao estah carregado */
    if (!carga) {
        printf("Dicionario nao carregado!\n");
        return ARQDICIO_NAOCARREGADO;
    } /* fim-if */

    /* calcula_tempo para carregar o dicionario */
    tempo_carga = calcula_tempo(&tempo_inicial, &tempo_final);

    /* tenta abrir um arquivo-texto para corrigir */
    arqTexto = (argc == 3) ? argv[2] : argv[1];
    fd = fopen(arqTexto, "r");
    if (fd == NULL) {
        printf("Nao foi possivel abrir o arquivo de texto %s.\n", arqTexto);
        descarregaDicionario();
        return ARQTEXTO_ERROABERTURA;
    } /* fim-if */

    /* Reportando palavras erradas de acordo com o dicionario */
    printf("\nPALAVRAS NAO ENCONTRADAS NO DICIONARIO \n\n");
    /* preparando para checar cada uma das palavras do arquivo-texto */
    indice = 0, totPalErradas = 0, totPalavras = 0;

    /* aloca memoria para palavra */
    palavra = (char *) malloc (sizeof(char)*TAM_MAX);

    /* checa cada palavra do arquivo-texto  */
    for (c = fgetc(fd); c != EOF; c = fgetc(fd)) {
        /* permite apenas palavras c/ caracteres alfabeticos e apostrofes */
        if (isalpha(c) || (c == '\'' && indice > 0)) {
            c = tolower(c);
            /* recupera um caracter do arquivo e coloca no vetor palavra */
            palavra[indice] = c;
            indice++;
            /* ignora palavras longas demais (acima de 45) */
            if (indice > TAM_MAX) {
                /* nesse caso, consome o restante da palavra e "reseta" o indice */
                while ((c = fgetc(fd)) != EOF && isalpha(c));
                indice = 0;
            } /* fim-if */
        } /* fim-if */
        /* ignora palavras que contenham numeros */
        else if (isdigit(c)) {
            /* nesse caso, consome o restante da palavra e "reseta" o indice */
            while ((c = fgetc(fd)) != EOF && isalnum(c));
            indice = 0;
        } /* fim-if */
        /* encontra uma palavra completa */
        else if (indice > 0) { /* termina a palavra corrente */
            palavra[indice] = '\0';
            totPalavras++;
            /* confere o tempo de busca da palavra */
            getrusage(RUSAGE_SELF, &tempo_inicial);
            palavraErrada = !conferePalavra(palavra);
            getrusage(RUSAGE_SELF, &tempo_final);
            /* atualiza tempo de checagem */
            tempo_check += calcula_tempo(&tempo_inicial, &tempo_final);
            /* imprime palavra se nao encontrada no dicionario */
            if (palavraErrada) {
                totPalErradas++;
            } /* fim-if */
            /* faz "reset" no indice para recuperar nova palavra no arquivo-texto*/
            indice = 0;
        } /* fim-if */
    } /* fim-for */

    /* libera memoria de string de checagem */
    free(palavra);
    /* verifica se houve um erro na leitura do arquivo-texto */
    if (ferror(fd)) {
        fclose(fd);
        printf("Erro de leitura %s.\n", arqTexto);
        descarregaDicionario();
        return ARQTEXTO_ERROLEITURA;
    } /* fim-if */

    /* fecha arquivo */
    fclose(fd);

    /* determina a quantidade de palavras presentes no dicionario, c anotacao de tempos */
    getrusage(RUSAGE_SELF, &tempo_inicial);
      qtdePalavrasDic = contaPalavrasDic();
    getrusage(RUSAGE_SELF, &tempo_final);

    /* calcula tempo p determinar o tamanho do dicionario */
    tempo_calc_tamanho_dic = calcula_tempo(&tempo_inicial, &tempo_final);

    /* limpa a memoria - descarrega o dicionario, c anotacao de tempos */
    getrusage(RUSAGE_SELF, &tempo_inicial);
      descarga = descarregaDicionario();
    getrusage(RUSAGE_SELF, &tempo_final);

    /* aborta se o dicionario nao estiver carregado */
    if (!descarga) {
        printf("Nao foi necessario fazer limpeza da memoria\n");
        return ERRO_DICIO_NAOCARREGADO;
    } /* fim-if */

    /* calcula tempo para descarregar o dicionario */
    tempo_limpeza_memoria = calcula_tempo(&tempo_inicial, &tempo_final);

    /* RESULTADOS ENCONTRADOS */
    printf("\nTOTAL DE PALAVRAS ERRADAS NO TEXTO    : %d\n",   totPalErradas);
    printf("TOTAL DE PALAVRAS DO DICIONARIO         : %d\n",   qtdePalavrasDic);
    printf("TOTAL DE PALAVRAS DO TEXTO              : %d\n",   totPalavras);
    printf("TEMPO GASTO COM CARGA DO DICIONARIO     : %.2f\n", tempo_carga);
    printf("TEMPO GASTO COM CHECK DO ARQUIVO        : %.2f\n", tempo_check);
    printf("TEMPO GASTO P CALCULO TAMANHO DICIONARIO: %.2f\n", tempo_calc_tamanho_dic);
    printf("TEMPO GASTO COM LIMPEZA DA MEMORIA      : %.2f\n", tempo_limpeza_memoria);
    printf("------------------------------------------------------\n");
    printf("T E M P O   T O T A L                   : %.2f\n\n",
     tempo_carga + tempo_check + tempo_calc_tamanho_dic + tempo_limpeza_memoria);
    printf("------------------------------------------------------\n");

   return SUCESSO;
} /* fim-main */
