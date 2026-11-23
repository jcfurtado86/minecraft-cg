/*
 * Projeto Minecraft
 *
 * Reúne o que foi visto nas semanas 1 a 8 e na aula de texturas e
 * transparência:
 *   Semana 1: janela, laço principal, tempo de quadro, FPS no título, ESC
 *   Semana 2: primitivas (GL_QUADS) e cor por face
 *   Semana 3: cubo 3D, buffer de profundidade, descarte de faces,
 *             transformações e instâncias com glPushMatrix/glPopMatrix
 *   Semana 4: perspectiva, correção de proporção, câmera com gluLookAt
 *             e navegação pelo teclado
 *   Semana 5: teclado lido por estado, movimento escalado pelo tempo de
 *             quadro, mouse com yaw e pitch e captura do cursor, colisão
 *             por caixa alinhada aos eixos, câmera em primeira e
 *             terceira pessoa e aproximação pela roda do mouse
 *   Iluminação (OpenGL 1.0): luz ambiente, difusa e especular,
 *             materiais, normais por face, normalização e Gouraud.
 *   Demonstração: tecla I abre/fecha cubo e coluna giratórios;
 *             teclas 1/2/3 escolhem componentes da luz, F/G escolhem
 *             sombreamento plano/Gouraud, O alterna luz pontual/direcional.
 *   Texturas e transparência: atlas PPM e texturas procedurais em memória,
 *             coordenadas por face, filtro GL_NEAREST,
 *             GL_MODULATE com iluminação por material, canal alfa
 *             com cor-chave, recorte (teste de alfa) nas folhas,
 *             mistura no vidro e na água, desenho em três passadas e
 *             remoção das faces que um vizinho opaco esconde
 *  Semana 8: incorporação de plantas como plano cruzado, itens largados
 *             no chão como sprites, céu como skydome, com sol e lua,
 *             neblina e ciclo de dia e noite.
 *
 *   Semana 9: gameplay. Raio da mira com percurso em grade (Amanatides e
 *             Woo) e contorno do bloco selecionado; quebrar e colocar
 *             blocos pelo mouse (glutMouseFunc); itens soltos que caem e
 *             são recolhidos; gravidade, pulo e colisão vertical; inimigos
 *             articulados que surgem à noite, perseguem, atacam e são
 *             atacados (raio contra caixa). Os inimigos usam o ciclo de
 *             dia e noite da semana 8: só surgem à noite.
 *
 * Cenário: terreno 16 x 16 com uma montanha, uma praia e o mar,
 * uma árvore, uma casinha de vidro e duas nuvens. Tudo é feito com um
 * único cubo de lado 1, desenhado várias vezes em posições diferentes.
 *
 * Texturas: o projeto combina atlas.ppm com texturas procedurais RGBA.
 * O atlas é lido da pasta atual, então precisa ser executado da pasta
 * do projeto.
 *
 * Astros: são carregados a partir dos arquivos sol.ppm e lua.ppm. O jogo
 * lê o arquivo da pasta atual, então precisa ser rodado da pasta do
 * projeto.
 *
 * Eixos do mundo:
 *   x  esquerda (0) para a direita (15)
 *   y  altura: 0 é o fundo, cresce para cima
 *   z  fundo do mapa (0) para a frente (15), onde fica o mar
 *
 * Controles:
 *   W / S        andar para frente / para trás
 *   A / D        andar para a esquerda / para a direita
 *   R            correr (segurando, junto com W, A, S, D)
 *   Setas < >    girar o olhar
 *   Espaço       pular (só apoiado no chão)
 *   Botão esq.   quebrar o bloco sob a mira, ou atacar um inimigo
 *   Botão dir.   colocar o bloco escolhido, encostado na face mirada
 *   1 a 8        escolher o tipo de bloco a colocar
 *   N            alternar dia / noite (para a demonstração)
 *   Mouse        direção do olhar (já começa capturado)
 *   Roda ou + -  aproximar / afastar (campo de visão)
 *   Tab          liberar / capturar o cursor; clique também captura
 *   F5 ou V      alternar terceira (início) / primeira pessoa
 *   P            alternar perspectiva / ortográfica
 *   T            acelera ciclo de dia e noite
 *   ESC          abrir / fechar o menu (pausa, instruções)
 *   I            abrir/fechar a demonstração de iluminação
 *   1 / 2 / 3    somente ambiente / ambiente+difusa / todas
 *   F / G        sombreamento plano / Gouraud
 *   O            alternar luz pontual / direcional
 *   Q            sair (com o menu aberto)
 *
 * Iluminação: usa o pipeline fixo clássico e GL_SEPARATE_SPECULAR_COLOR.
 * Água e fogo usam shaders GLSL quando disponíveis; as texturas
 * procedurais usam RGBA para permitir recorte correto nas folhas.
 *
 * Organização do arquivo, de cima para baixo:
 *   constantes, declarações antecipadas, texturas e transparência
 *   (tudo o que essa aula trouxe, num bloco só), mundo, colisão,
 *   câmera, entrada, menu e, no fim, desenho do quadro e main()
 *
 * Compilar e rodar (Linux):
 *   gcc mine.c -o mine -lGLEW -lGL -lGLU -lglut -lm && ./mine
 *
 * Compilar e rodar (Windows, MinGW-w64 com freeglut e GLEW):
 *   gcc mine.c -o mine.exe -lglew32 -lfreeglut -lopengl32 -lglu32 -lm
 *   mine.exe
 *
 * Compilar e rodar (macOS, com a GLUT da Apple; sem roda do mouse e
 * sem Shift para descer):
 *   clang mine.c -o mine -framework OpenGL -framework GLUT \
 *         -framework ApplicationServices -lm && ./mine
 */

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
/* macOS: usa a GLUT e o OpenGL nativos da Apple, sem GLEW; essa GLUT
   não tem a roda do mouse nem o Shift como tecla especial */
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#include <ApplicationServices/ApplicationServices.h>   /* mouse no macOS,
                                                          ver movimentoMouse */
#else
#include <GL/glew.h>            /* carrega as funções do OpenGL */
#include <GL/freeglut.h>        /* janela, teclado, mouse e laço principal;
                                   a versão freeglut traz a roda do mouse */
#include <GL/glu.h>             /* gluPerspective e gluLookAt */
#endif

#include <stdio.h>              /* printf, sprintf */
#include <stdlib.h>             /* exit */
#include <math.h>               /* sinf, cosf, floorf, sqrtf, ceilf */
#include <ctype.h>              /* tolower */
#include <time.h>               /* time, para a semente do rand */

#define PI 3.14159265f
#define INFINITO 1e30f          /* "nunca": distância de um eixo paralelo ao raio */

/* ------------------------------------------------------------------ */
/* Tipos de bloco                                                     */
/* Cada tipo é só um número com nome. O número não muda a forma do    */
/* bloco (todos são o mesmo cubo); ele só escolhe a aparência: a      */
/* célula do atlas de cada face, em celulaDaFace(), ou a cor sólida,  */
/* em corDaFace(). Para criar um tipo novo: um #define aqui e um case */
/* em uma das duas.                                                   */
/* VAZIO marca uma posição sem bloco na matriz do mundo. Os tipos de  */
/* PELE a OLHO não aparecem no mundo: são as cores do personagem.     */
/* ------------------------------------------------------------------ */

#define VAZIO   0
#define GRAMA   1
#define TERRA   2
#define PEDRA   3
#define TRONCO  4
#define FOLHA   5
#define AREIA   6
#define AGUA    7
#define NUVEM   8
#define NEVE    9
#define PELE    10
#define CABELO  11
#define CAMISA  12
#define CALCA   13
#define SAPATO  14
#define OLHO_BRANCO 15
#define OLHO    16
#define VIDRO   17      /* semitransparente, da casinha de vidro */
#define PLANTA  18      /* planta em plano cruzado (semana 8) */
#define ZUMBI_PELE   19 /* cores do inimigo; ZUMBI_PELE também é o item */
#define ZUMBI_CAMISA 20 /* que ele deixa ao morrer ("carne")            */
#define ZUMBI_CALCA  21
#define ZUMBI_DANO   22 /* o inimigo todo fica vermelho ao ser atingido */
#define MACA         23 /* item: maçã, só aparece solta no chão          */
#define NUM_TIPOS    24 /* tamanho dos vetores indexados pelo tipo */

/* Em que passada de desenho cada tipo entra (ver desenhaOpacos):
   opacos primeiro, depois o recorte (folhas, com vazados) e por último
   os semitransparentes (vidro e água) */
#define PASSADA_OPACA       0
#define PASSADA_RECORTE     1
#define PASSADA_TRANSLUCIDA 2

/* ------------------------------------------------------------------ */
/* Tamanho do mundo                                                   */
/* ------------------------------------------------------------------ */

#define LADO        16  /* o chão tem LADO x LADO quadrados (colunas) */
#define ALTURA_MUNDO 16 /* andares em y, de 0 a 15 */
#define NIVEL_AGUA  1   /* colunas com altura até este valor são praia ou
                           mar; acima do chão delas, até esta altura, vai
                           água */
#define NIVEL_NEVE  6   /* colunas com altura a partir deste valor têm
                           neve no topo em vez de grama */
#define ALTURA_NUVEM 14 /* altura (y) das nuvens; bem acima da neve, para
                           não se confundir com ela */

/* ------------------------------------------------------------------ */
/* Atlas de texturas                                                  */
/* Uma única imagem com a aparência de todas as faces, dividida em    */
/* CELULAS_POR_LADO x CELULAS_POR_LADO células de mesmo tamanho. Cada */
/* CEL_ é o índice de uma célula, contado da esquerda para a direita  */
/* e de cima para baixo.                                              */
/* ------------------------------------------------------------------ */

#define ARQUIVO_ATLAS    "atlas.ppm"
#define CELULAS_POR_LADO 4      /* N: o atlas tem N x N células */
#define TAMANHO_CELULA   16     /* pixels de lado de cada célula */

#define CEL_GRAMA_TOPO  0
#define CEL_GRAMA_LADO  1       /* terra com faixa de grama no alto */
#define CEL_TERRA       2
#define CEL_PEDRA       3
#define CEL_ANEIS       4       /* topo e base do tronco */
#define CEL_CASCA       5       /* laterais do tronco */
#define CEL_FOLHA       6       /* com vazados em magenta (cor-chave) */
#define CEL_VIDRO       7
#define CEL_AGUA        8
#define CEL_AREIA       9
#define CEL_NEVE        10
#define SEM_TEXTURA     (-1)    /* nuvem e personagem: cor sólida */

/* As 6 faces de um bloco, uma por bit, para dizer quais desenhar:
   FACE_TOPO | FACE_BASE são só o topo e a base */
#define FACE_FRENTE   1         /* z = 1 */
#define FACE_TRAS     2         /* z = 0 */
#define FACE_DIREITA  4         /* x = 1 */
#define FACE_ESQUERDA 8         /* x = 0 */
#define FACE_TOPO     16        /* y = 1 */
#define FACE_BASE     32        /* y = 0 */
#define TODAS_FACES   63

/* opacidade (alfa, de 0 a 255) dada às células semitransparentes no
   carregamento: o PPM não tem canal alfa */
#define ALFA_VIDRO 102          /* 0,4 */
#define ALFA_AGUA  153          /* 0,6 */

/* ------------------------------------------------------------------ */
/* Personagem                                                         */
/* Para a colisão, o personagem é uma caixa de LARGURA_JOGADOR x      */
/* ALTURA_JOGADOR x LARGURA_JOGADOR, centrada em x e z e apoiada nos  */
/* pés. Os olhos ficam ALTURA_OLHOS acima dos pés.                    */
/* ------------------------------------------------------------------ */

#define LARGURA_JOGADOR 0.6f    /* lado da caixa em x e z */
#define ALTURA_JOGADOR  1.8f    /* dos pés ao topo da cabeça */
#define ALTURA_OLHOS    1.6f    /* dos pés aos olhos */

#define VELOCIDADE       5.0f   /* blocos por segundo */
#define VELOCIDADE_GIRO  90.0f  /* graus por segundo (setas) */
#define DT_MAXIMO        0.1f   /* maior tempo de quadro aceito, em s */
#define FATOR_CORRIDA    2.0f   /* segurando R, a velocidade dobra */
#define PASSO_MAXIMO     0.5f   /* maior deslocamento testado de uma vez */

#define SENSIBILIDADE 0.1f      /* graus de giro por pixel de mouse */
#define LIMITE_PITCH  89.0f     /* inclinação máxima, para cima e para baixo */

/* ------------------------------------------------------------------ */
/* Câmera                                                             */
/* ------------------------------------------------------------------ */

#define PRIMEIRA_PESSOA 0
#define TERCEIRA_PESSOA 1

#define DISTANCIA_CAMERA 4.0f   /* recuo da câmera em terceira pessoa */
#define ELEVACAO_CAMERA  1.5f   /* altura da câmera acima da cabeça */

/* ------------------------------------------------------------------ */
/* Boneco (desenho do personagem em terceira pessoa)                  */
/* Medidas em blocos, no estilo do Minecraft: pernas, tronco e cabeça */
/* empilhados até ALTURA_JOGADOR, braços dos lados do tronco. Tudo    */
/* cabe num círculo de raio 0.3 em volta do eixo vertical, para o     */
/* boneco girado não sair da caixa de colisão (ver desenhaJogador).   */
/* ------------------------------------------------------------------ */

#define COMPRIMENTO_MEMBRO 0.7f /* braços e pernas */
#define PROFUNDIDADE_CORPO 0.2f /* espessura (z) de tronco e membros */
#define LARGURA_PERNA      0.15f
#define LARGURA_TRONCO     0.3f
#define LARGURA_BRACO      0.1f
#define LADO_CABECA        0.38f
#define BALANCO_MAXIMO     30.0f    /* graus de balanço de braços e pernas */
#define FASE_POR_BLOCO     3.9f     /* radianos de balanço por bloco andado */
#define VELOCIDADE_AMPLITUDE 6.0f   /* quão rápido o balanço começa e para */

#define FOV_MIN 20.0f
#define FOV_MAX 90.0f

/* ------------------------------------------------------------------ */
/* Semana 9: seleção, edição, física, itens, inimigos e dia e noite   */
/* ------------------------------------------------------------------ */

/* seleção e edição de blocos */
#define ALCANCE        5.0f     /* até onde a mira alcança, em blocos */
#define FOLGA_CONTORNO 0.003f   /* o contorno é maior que o bloco por isso,
                                   para as linhas não brigarem com as faces
                                   no buffer de profundidade (semana 4) */
#define CONSOME_ITENS  0        /* 1: colocar gasta um item do tipo (precisa
                                   tê-lo recolhido); 0: blocos ilimitados */

/* início do personagem (também usado ao reaparecer) */
#define INICIO_X 8.5f
#define INICIO_Y 3.0f
#define INICIO_Z 10.5f

/* física: blocos e segundos */
#define GRAVIDADE        20.0f  /* aceleração para baixo, blocos/s^2 */
#define VELOCIDADE_PULO   7.0f  /* altura do pulo = v^2 / 2g = 1,22 bloco */
#define VELOCIDADE_QUEDA_MAXIMA 30.0f   /* limite da queda, blocos/s */
#define VIDA_JOGADOR     20
#define DANO_CONTATO      2     /* vida perdida a cada contato com um inimigo */
#define INVULNERAVEL_TEMPO 1.0f /* s sem perder vida depois de um contato */

/* itens soltos */
#define MAX_ITENS     128
#define LADO_ITEM     0.25f     /* o item é um cubo pequeno */
#define RAIO_COLETA   1.5f      /* distância para recolher, em blocos */
#define ATRASO_COLETA 0.5f      /* s antes de poder recolher o item solto */

/* inimigos */
#define MAX_INIMIGOS       5
#define LARGURA_INIMIGO    0.6f     /* a caixa é igual à do personagem */
#define ALTURA_INIMIGO     1.8f
#define VELOCIDADE_INIMIGO 2.5f     /* blocos por segundo */
#define PARADA_INIMIGO     0.55f    /* distância mínima até o jogador */
#define VIDA_INIMIGO       6
#define DANO_ATAQUE        2        /* vida tirada por golpe do jogador */
#define RECUO_HORIZONTAL   7.0f     /* velocidade do empurrão, blocos/s */
#define RECUO_VERTICAL     4.5f
#define INTERVALO_SPAWN    3.0f     /* s entre tentativas de surgir */
#define DISTANCIA_SPAWN    6.0f     /* surgem a pelo menos isto do jogador */
#define DESAPARECEM_DE_DIA 1        /* 1: somem ao amanhecer, sem deixar item */



/* ================================================================== */
/* Declarações antecipadas                                            */
/* Funções e variáveis usadas antes do ponto do arquivo em que        */
/* são definidas. Servem também de índice do que cada parte faz.      */
/* ================================================================== */

/*
 * Texturas e transparência: o atlas, a célula de cada face, as faces
 * ocultas, a casinha de vidro e o desenho em três passadas. Tudo
 * definido na seção TEXTURAS E TRANSPARÊNCIA.
 */
/* 1. atlas: leitura do arquivo e envio à placa de vídeo */
extern GLuint atlas;                /* objeto de textura do atlas */
unsigned char *lerPPM(const char *nome, int *l, int *a);
unsigned char *converteRGBA(const unsigned char *rgb, int l, int a);
void carregaAtlas(void);

/* 2. célula do atlas de cada face */
int  celulaDaFace(int tipo, int face);
void coordenadasDaCelula(int celula, float *s0, float *t0, float *lado);

/* 3. passada de cada tipo e faces que um vizinho esconde */
int  passadaDoBloco(int tipo);
int  blocoOpaco(int tipo);
int  faceVisivel(int tipo, int vizinho);
int  facesVisiveis(int x, int y, int z);    /* soma de bits FACE_ */

/* 4. casinha de vidro */
void casinhaEm(int x, int z);

/* 5. desenho da cena, em três passadas (ver desenhaOpacos) */
typedef struct {
    int x, y, z;                    /* posição do bloco na matriz */
    float distancia;                /* ao olho, ao quadrado */
} BlocoTransparente;
#define MAX_TRANSPARENTES (LADO * ALTURA_MUNDO * LADO)

void desenhaOpacos(void);           /* passadas 1 (opacos) e 2 (recorte) */
int  listaTransparentes(float olhoX, float olhoY, float olhoZ,
                        BlocoTransparente *lista);
void desenhaTransparentes(void);    /* passada 3 (vidro e água) */

/* 6. posição do olho da câmera */
void posicaoDoOlho(float *x, float *y, float *z);

/*
 * O cenário: o mapa de alturas, a matriz 3D com o tipo de cada bloco,
 * a construção do mundo a partir do mapa e o desenho dos blocos.
 */
/* tipo de cada bloco, indexado como mundo[x][y][z] (ver MUNDO) */
extern unsigned char mundo[LADO][ALTURA_MUNDO][LADO];

/* desenho de um bloco */
void corDaFace(int tipo, float brilho, float tom);
void desenhaBloco(int tipo, float tom, int faces);  /* cubo de lado 1 na origem */
void blocoEm(int tipo, int x, int y, int z);

/* Iluminação fixa (OpenGL 1.0) e demonstração isolada do Bloco 3. */
void inicializaIluminacao(void);
void atualizaComponentesLuz(void);
void defineLuzDoCenario(void);
void aplicaMaterial(int tipo);
void desenhaDemonstracao(void);
extern int modoDemonstracao, modoComponentesLuz, modoSombreamento;
extern float anguloDemonstracao;

/* Recursos acrescentados: texturas procedurais e shaders de agua/fogo. */
static GLuint texturasBlocos[18] = {0};
static int usaTexturasProcedurais = 0;  /* tecla X; 0: atlas */
extern float brilhoDoCeu;
static GLuint programaAgua = 0, programaFogo = 0;
static GLint tempoAguaLoc = -1, tempoFogoLoc = -1, luzAguaLoc = -1;
static int shadersDisponiveis = 0;
static int emissaoDemo = 0;
static int texturaDoBloco(int tipo);
static void inicializaTexturas(void);
static void inicializaShaders(void);
static void desenhaAgua(void);
static void desenhaFogo(void);

/* acesso à matriz */
int  alturaDoTerreno(int x, int z);
int  dentroDoMundo(int x, int y, int z);
void colocaBloco(int tipo, int x, int y, int z);
int  blocoNoMundo(int x, int y, int z);

/* construção (uma vez, no início) */
void colunaEm(int x, int z);
void arvoreEm(int x, int z);
void nuvemEm(int x, int z, int largura, int fundo);
void construirMundo(void);

/*
 * A posição do personagem e a colisão dele com o mundo.
 */
/* posição do personagem; jogadorY é a altura dos PÉS (ver COLISÃO) */
extern float jogadorX, jogadorY, jogadorZ;

extern float velocidadeY;           /* velocidade vertical, blocos/s */
extern int   jogadorNoChao;         /* 1 se está apoiado em um bloco */

/* resultado de moveCaixa: eixos em que o movimento foi barrado */
#define BLOQ_X     1
#define BLOQ_Z     2
#define BLOQ_BAIXO 4                /* desceu e achou chão */
#define BLOQ_CIMA  8                /* subiu e bateu a cabeça */

int  blocoSolido(int x, int y, int z);
int  caixaLivre(float x, float y, float z, float largura, float altura);
int  posicaoLivre(float x, float y, float z);
int  moveCaixa(float *x, float *y, float *z, float largura, float altura,
               float dx, float dy, float dz);
void moveJogador(float dx, float dy, float dz);

/*
 * Orientação do olhar, lente (projeção), posição da câmera em primeira
 * e terceira pessoa e desenho do personagem.
 */
extern float yaw, pitch;            /* orientação do olhar, em graus */
extern int   modoCamera;            /* PRIMEIRA_PESSOA ou TERCEIRA_PESSOA */

extern int   larguraJanela, alturaJanela;
extern int   projecaoOrtogonal;     /* 0 perspectiva, 1 ortográfica */
extern float campoDeVisao;          /* graus */

void aplicaProjecao(void);
void redimensiona(int largura, int altura);
void direcaoDoOlhar(float *dx, float *dy, float *dz);
void desenhaJogador(void);
void posicionaCamera(void);

/*
 * Semana 9. Seleção do bloco sob a mira: percurso em grade, contorno e
 * a edição do mundo (quebrar e colocar).
 */
typedef struct {
    int   acertou;                  /* 1 se há bloco dentro do alcance */
    int   x, y, z;                  /* célula sólida atingida (quebrar) */
    int   ax, ay, az;               /* célula vazia anterior (colocar) */
    float distancia;                /* do início do raio até a célula, em blocos */
} Alvo;

extern Alvo alvo;                   /* resultado do quadro corrente */
extern int  inimigoMirado;          /* índice do inimigo sob a mira, ou -1 */
extern int  tipoSelecionado;        /* tipo colocado pelo botão direito */

int  blocoSelecionavel(int tipo);
int  percursoEmGrade(float ox, float oy, float oz, float dx, float dy, float dz,
                     float inicio, float fim, Alvo *resultado);
void raioDaMira(float *ox, float *oy, float *oz, float *dx, float *dy, float *dz,
                float *inicio, float *fim);
void atualizaMira(void);
void desenhaContorno(void);
void quebraBloco(void);
void colocaBlocoMirado(void);
void cliqueEdicao(int botao);

/*
 * Semana 9. Itens soltos e a contagem recolhida de cada tipo.
 */
typedef struct {
    float x, y, z;                  /* y é a base do cubo */
    float vy;                       /* velocidade vertical */
    float idade;                    /* s desde que apareceu */
    int   tipo;
} Item;

extern Item itens[MAX_ITENS];
extern int  numItens;
extern int  contagem[NUM_TIPOS];    /* itens recolhidos, por tipo */

void criaItem(int tipo, float x, float y, float z);
void atualizaItens(float dt);
void desenhaItens(void);

/*
 * Semana 9. Inimigos: estrutura, atualização, desenho e combate.
 */
typedef struct {
    float x, y, z;                  /* pés, centro da caixa */
    float vy;                       /* velocidade vertical */
    float kx, kz;                   /* velocidade do recuo, decai sozinha */
    float yaw;                      /* orientação, como a da câmera */
    float fase, amplitude;          /* animação das pernas */
    float ferido;                   /* s restantes de vermelho */
    int   vida;
} Inimigo;

extern Inimigo inimigos[MAX_INIMIGOS];
extern int     numInimigos;
extern int     vidaJogador;
extern float   invulnerabilidade;   /* s restantes sem levar dano */

int  raioContraCaixa(float ox, float oy, float oz, float dx, float dy, float dz,
                     const float minimo[3], const float maximo[3], float *entrada);
int  criaInimigo(float x, float y, float z);
void atualizaInimigo(Inimigo *e, float dt);
void atualizaInimigos(float dt);
void desenhaInimigo(const Inimigo *e);
void atacaInimigo(int i);
void reapareceJogador(void);

/*
 * Dia e noite: o relógio é o ângulo do sol (semana 8).
 */
extern float anguloSol;             /* graus: 0 nasce, 90 meio-dia, 270 meia-noite */
int  ehNoite(void);

/*
 * Teclado, mouse e roda, e a atualização do movimento a cada quadro.
 * As funções de callback são registradas na GLUT por main().
 */
extern int ultimoTempo;             /* tempo da atualização anterior, em ms */

/* animação de caminhada, atualizada em atualiza() e usada pelo desenho
   do personagem */
extern float faseCaminhada;         /* radianos; avança com a distância */
extern float amplitudeCaminhada;    /* 0 parado, 1 andando */

/* teclado */
void teclado(unsigned char tecla, int x, int y);
void tecladoSolto(unsigned char tecla, int x, int y);
void especial(int tecla, int x, int y);
void especialSolto(int tecla, int x, int y);

/* mouse */
void movimentoMouse(int x, int y);
void capturaCursor(void);
void liberaCursor(void);
void alternaCaptura(void);
void cliqueMouse(int botao, int estado, int x, int y);
void entradaJanela(int estado);
void mudaZoom(int direcao);
void rodaMouse(int roda, int direcao, int x, int y);

/* atualização */
void atualiza(void);

/*
 * Menu de pausa, aberto e fechado com ESC, com as instruções de
 * movimentação. Enquanto ele está aberto o jogo fica parado e o cursor
 * fica livre.
 */
extern int menuAberto;              /* 1 enquanto o menu está na tela */

void abreMenu(void);
void fechaMenu(void);
void alternaMenu(void);
void desenhaMenu(void);             /* chamada por desenha(), depois da cena */

/*
 * Mira no centro da tela e informações provisórias no título da janela
 * (a interface completa fica para a semana 10).
 */
extern int tituloSujo;              /* 1: refazer o título no próximo quadro */

void desenhaMira(void);
void atualizaTitulo(int fps);
const char *nomeDoTipo(int tipo);


/* ================================================================== */
/* TEXTURAS E TRANSPARÊNCIA                                           */
/*                                                                    */
/* Tudo o que entrou com a aula de texturas e transparência, reunido  */
/* num lugar só, na ordem em que acontece:                            */
/*   1. atlas: leitura do PPM, canal alfa, envio à placa e filtro     */
/*   2. célula do atlas de cada face e suas coordenadas (s, t)        */
/*   3. passada de cada tipo de bloco e faces que um vizinho esconde  */
/*   4. casinha de vidro                                              */
/*   5. desenho da cena em três passadas                              */
/*   6. posição do olho, para ordenar os semitransparentes            */
/* desenhaBloco(), na seção MUNDO, usa as funções daqui para pôr a    */
/* textura em cada face.                                              */
/* ================================================================== */


/* ------------------------------------------------------------------ */
/* Leitura do atlas: PPM binário                                      */
/* Nenhuma das bibliotecas da disciplina lê arquivos de imagem. O PPM */
/* resolve com poucas linhas: um cabeçalho em texto seguido dos bytes */
/* de cor, R, G, B de cada pixel, linha por linha, de cima para baixo:*/
/*                                                                    */
/*   P6                                                               */
/*   64 64          largura e altura                                  */
/*   255            valor máximo de cada componente                   */
/*   RGB RGB RGB ...                                                  */
/*                                                                    */
/* O leitor assume um cabeçalho sem linhas de comentário (#), como o  */
/* do atlas.ppm. Devolve NULL se o arquivo não abrir ou               */
/* não for um P6 completo, com no máximo 4096 pixels de lado.        */
/* ------------------------------------------------------------------ */

unsigned char *lerPPM(const char *nome, int *l, int *a)
{
    FILE *f = fopen(nome, "rb");
    int max;

    if (f == NULL) return NULL;
    /* o teto nas medidas impede que a conta do malloc estoure com um
       cabeçalho absurdo; o atlas tem 64 x 64 */
    if (fscanf(f, "P6 %d %d %d", l, a, &max) != 3 || max != 255 ||
        *l <= 0 || *a <= 0 || *l > 4096 || *a > 4096) {
        fclose(f);
        return NULL;
    }
    fgetc(f);                               /* o espaço após o cabeçalho */

    unsigned char *rgb = malloc(3 * *l * *a);
    if (rgb != NULL && fread(rgb, 3, *l * *a, f) != (size_t)(*l * *a)) {
        free(rgb);                          /* arquivo cortado */
        rgb = NULL;
    }
    fclose(f);
    return rgb;
}

/* ------------------------------------------------------------------ */
/* RGB para RGBA: o canal alfa (a quarta componente da cor, presente  */
/* desde a semana 1) diz a opacidade de cada texel. O PPM não tem     */
/* alfa, então ele é decidido aqui:                                   */
/*   magenta (255, 0, 255)  alfa 0: a cor-chave, os vazados da folha  */
/*   células de vidro/água  alfa parcial (ALFA_VIDRO, ALFA_AGUA)      */
/*   o resto                alfa 255, opaco                           */
/* O texel transparente também recebe o verde da folha no lugar do    */
/* magenta: com GL_NEAREST não faz diferença, mas um filtro que faça  */
/* média com os vizinhos deixaria uma franja rosa nas bordas.         */
/* ------------------------------------------------------------------ */

unsigned char *converteRGBA(const unsigned char *rgb, int l, int a)
{
    unsigned char *rgba = malloc(4 * l * a);
    int i;

    for (i = 0; i < l * a; i++) {
        unsigned char r = rgb[3*i], g = rgb[3*i+1], b = rgb[3*i+2];
        int x = i % l, y = i / l;           /* coluna e linha do texel */
        int celula = (y / TAMANHO_CELULA) * CELULAS_POR_LADO + x / TAMANHO_CELULA;

        rgba[4*i]   = r;
        rgba[4*i+1] = g;
        rgba[4*i+2] = b;
        rgba[4*i+3] = 255;

        if (r == 255 && g == 0 && b == 255) {
            rgba[4*i] = 59; rgba[4*i+1] = 122; rgba[4*i+2] = 33;   /* verde da folha */
            rgba[4*i+3] = 0;
        }
        else if (celula == CEL_VIDRO) rgba[4*i+3] = ALFA_VIDRO;
        else if (celula == CEL_AGUA)  rgba[4*i+3] = ALFA_AGUA;
    }
    return rgba;
}

/* ------------------------------------------------------------------ */
/* Da imagem lida à textura ativa, uma vez só, na inicialização:      */
/*   glGenTextures   reserva um identificador de textura              */
/*   glBindTexture   torna essa textura parte do estado corrente,     */
/*                   como a cor corrente (semana 1)                   */
/*   glTexImage2D    copia os pixels RGBA para a placa de vídeo       */
/*   GL_NEAREST      o texel mais próximo: o visual pixelado; e sem   */
/*                   misturar células vizinhas do atlas nas arestas   */
/*   GL_MODULATE     cor final = textura x cor do vértice, para o     */
/*                   iluminação calculada pelo pipeline fixo           */
/* Como só existe essa textura, ela fica ativa o tempo todo: nenhum   */
/* bloco exige troca de textura entre um desenho e outro.             */
/* ------------------------------------------------------------------ */

GLuint atlas;

void carregaAtlas(void)
{
    int l, a;
    unsigned char *rgb = lerPPM(ARQUIVO_ATLAS, &l, &a);

    if (rgb == NULL || l != CELULAS_POR_LADO * TAMANHO_CELULA ||
                       a != CELULAS_POR_LADO * TAMANHO_CELULA) {
        printf("Erro: não consegui ler %s (rode o jogo de dentro da pasta do projeto)\n",
               ARQUIVO_ATLAS);
        exit(1);
    }
    unsigned char *rgba = converteRGBA(rgb, l, a);

    glGenTextures(1, &atlas);
    glBindTexture(GL_TEXTURE_2D, atlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, l, a, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);

    free(rgb);                              /* a placa de vídeo já tem a cópia */
    free(rgba);
}

/* ------------------------------------------------------------------ */
/* Células por face (atlas)                                           */
/* Cada face de bloco tem a aparência de uma célula do atlas. A       */
/* maioria dos blocos usa a mesma célula nas 6 faces; a grama e o     */
/* tronco usam uma no topo, outra nas laterais e outra na base:       */
/*                                                                    */
/*   bloco    topo         laterais        base                       */
/*   GRAMA    grama topo   grama lateral   terra                      */
/*   TRONCO   anéis        casca           anéis                      */
/*                                                                    */
/* face é um dos bits FACE_ (ver desenhaBloco). Nuvem e personagem    */
/* não têm célula: devolvem SEM_TEXTURA e ficam com cor sólida.       */
/* ------------------------------------------------------------------ */

int celulaDaFace(int tipo, int face)
{
    switch (tipo) {
        case GRAMA:
            if (face == FACE_TOPO) return CEL_GRAMA_TOPO;
            if (face == FACE_BASE) return CEL_TERRA;
            return CEL_GRAMA_LADO;
        case TRONCO:
            if (face == FACE_TOPO || face == FACE_BASE) return CEL_ANEIS;
            return CEL_CASCA;
        case TERRA: return CEL_TERRA;
        case PEDRA: return CEL_PEDRA;
        case FOLHA: return CEL_FOLHA;
        case AGUA:  return CEL_AGUA;
        case VIDRO: return CEL_VIDRO;
        case AREIA: return CEL_AREIA;
        case NEVE:  return CEL_NEVE;
        default:    return SEM_TEXTURA;
    }
}

/* ------------------------------------------------------------------ */
/* Do índice da célula às coordenadas de textura (s, t).              */
/* Em um atlas de N x N células, a célula i fica na coluna i % N e na */
/* linha i / N (divisão inteira), começa em (s0, t0) e mede 1/N em    */
/* cada direção:                                                      */
/*                                                                    */
/*   s0 = coluna / N      t0 = linha / N      lado = 1 / N            */
/*                                                                    */
/* Exemplo, folha (i = 6, N = 4): coluna 2, linha 1, s de 0,50 a      */
/* 0,75 e t de 0,25 a 0,50.                                           */
/* ------------------------------------------------------------------ */

void coordenadasDaCelula(int celula, float *s0, float *t0, float *lado)
{
    *lado = 1.0f / CELULAS_POR_LADO;
    *s0 = (celula % CELULAS_POR_LADO) * *lado;
    *t0 = (celula / CELULAS_POR_LADO) * *lado;
}

/* ------------------------------------------------------------------ */
/* Transparência: dois casos distintos                                */
/*   recorte            folhas: cada texel é totalmente opaco ou      */
/*                      totalmente transparente (os vazados)          */
/*   semitransparência  vidro e água: a cor do bloco é misturada à    */
/*                      cor do que está atrás, na proporção do alfa   */
/* O resto é opaco. Cada caso é desenhado numa passada própria.       */
/* ------------------------------------------------------------------ */

int passadaDoBloco(int tipo)
{
    if (tipo == FOLHA || tipo == PLANTA)
        return PASSADA_RECORTE;

    if (tipo == VIDRO || tipo == AGUA)
        return PASSADA_TRANSLUCIDA;

    return PASSADA_OPACA;
}

/* 1 se o bloco esconde por completo o que está atrás dele */
int blocoOpaco(int tipo)
{
    return tipo != VAZIO && passadaDoBloco(tipo) == PASSADA_OPACA;
}

/* ------------------------------------------------------------------ */
/* Faces ocultas: uma face encostada num bloco opaco nunca aparece,   */
/* então nem é desenhada. Através do vidro, da água e das folhas, a   */
/* face do vizinho fica à mostra e precisa ser desenhada:             */
/*                                                                    */
/*   face de...     encostada em...          desenhar?                */
/*   qualquer       bloco opaco              não, continua oculta     */
/*   qualquer       vazio, vidro, água,      sim                      */
/*                  folha                                             */
/*   água           água                     não (exceção)            */
/*   vidro          vidro                    não (exceção)            */
/*                                                                    */
/* As exceções evitam as paredes internas de um mar ou de uma janela  */
/* feitos de vários blocos. Entre folhas a face continua: pelos       */
/* vazados se vê o interior da copa. Fora do mundo conta como vazio   */
/* (blocoNoMundo), para as bordas do terreno aparecerem.              */
/* ------------------------------------------------------------------ */

int faceVisivel(int tipo, int vizinho)
{
    if (blocoOpaco(vizinho)) return 0;
    if (vizinho == tipo && (tipo == AGUA || tipo == VIDRO)) return 0;
    return 1;
}

/* As faces do bloco em (x, y, z) que precisam ser desenhadas, como
   soma de bits FACE_, olhando o vizinho de cada lado */
int facesVisiveis(int x, int y, int z)
{
    int tipo = blocoNoMundo(x, y, z);
    int faces = 0;

    if (faceVisivel(tipo, blocoNoMundo(x, y, z + 1))) faces |= FACE_FRENTE;
    if (faceVisivel(tipo, blocoNoMundo(x, y, z - 1))) faces |= FACE_TRAS;
    if (faceVisivel(tipo, blocoNoMundo(x + 1, y, z))) faces |= FACE_DIREITA;
    if (faceVisivel(tipo, blocoNoMundo(x - 1, y, z))) faces |= FACE_ESQUERDA;
    if (faceVisivel(tipo, blocoNoMundo(x, y + 1, z))) faces |= FACE_TOPO;
    if (faceVisivel(tipo, blocoNoMundo(x, y - 1, z))) faces |= FACE_BASE;
    return faces;
}

/* ------------------------------------------------------------------ */
/* Casinha de vidro, oca, com 4 x 4 blocos de base e o canto em       */
/* (x, z). Serve para mostrar a semitransparência: de fora, vê-se o   */
/* terreno através dela; de dentro, o mundo em volta. Vista de cima,  */
/* em cada andar das paredes (V vidro, . vazio, P porta):             */
/*                                                                    */
/*   z           V  V  V  V                                           */
/*   z+1         V  .  .  V                                           */
/*   z+2         V  .  .  V                                           */
/*   z+3         V  P  V  V      parede virada para +z, para o        */
/*               x        x+3    personagem, que começa mais à frente */
/*                                                                    */
/* As paredes vão de chao+1 a chao+3 e o telhado, inteiro, fica em    */
/* chao+4. A porta tem 1 bloco de largura e 2 de altura, para o       */
/* personagem (1.8 de altura) passar.                                 */
/* ------------------------------------------------------------------ */

void casinhaEm(int x, int z)
{
    int chao = alturaDoTerreno(x, z);
    int y, dx, dz;

    for (y = chao + 1; y <= chao + 3; y++)
        for (dx = 0; dx < 4; dx++)
            for (dz = 0; dz < 4; dz++)
                if (dx == 0 || dx == 3 || dz == 0 || dz == 3)   /* só o perímetro */
                    colocaBloco(VIDRO, x + dx, y, z + dz);

    for (dx = 0; dx < 4; dx++)              /* telhado */
        for (dz = 0; dz < 4; dz++)
            colocaBloco(VIDRO, x + dx, chao + 4, z + dz);

    colocaBloco(VAZIO, x + 1, chao + 1, z + 3);     /* porta */
    colocaBloco(VAZIO, x + 1, chao + 2, z + 3);
}

/* ------------------------------------------------------------------ */
/* Itens como imagem e plantas. A função desenhaItem() desenha um     */
/* item que não é bloco (a muda e a maçã) como uma imagem que gira    */
/* conforme a direção horizontal da câmera (yaw), criando um          */
/* billboard. A posição e a flutuação ficam por conta de quem chama   */
/* (desenhaItens, na seção ITENS). O item pode usar uma célula do     */
/* atlas ou uma representação provisória, desenhada com cor sólida.   */
/*                                                                    */
/* A função desenhaPlanta() utiliza a textura de uma folha para        */
/* construir dois planos diagonais cruzados, formando um modelo       */
/* tridimensional simples. O descarte de faces traseiras é desativado */
/* temporariamente para que a planta seja visível de ambos os lados.  */
/* Ao final, os estados gráficos utilizados são restaurados.           */
/* ------------------------------------------------------------------ */

/* Desenha um item como uma imagem que acompanha a direção da câmera */
void desenhaItem(int celula)
{
    float s0, t0, lado;
    float largura = 0.45f;

    glPushMatrix();

    /* Gira o item para acompanhar a direção horizontal da câmera */
    glRotatef(-yaw, 0.0f, 1.0f, 0.0f);

    if (celula != SEM_TEXTURA) {
        coordenadasDaCelula(celula, &s0, &t0, &lado);

        glEnable(GL_TEXTURE_2D);
        glColor3f(1.0f, 1.0f, 1.0f);

        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.5f);

        glBegin(GL_QUADS);
            glTexCoord2f(s0, t0 + lado);
            glVertex3f(-largura, 0.0f, 0.0f);

            glTexCoord2f(s0 + lado, t0 + lado);
            glVertex3f(largura, 0.0f, 0.0f);

            glTexCoord2f(s0 + lado, t0);
            glVertex3f(largura, 0.7f, 0.0f);

            glTexCoord2f(s0, t0);
            glVertex3f(-largura, 0.7f, 0.0f);
        glEnd();
        glDisable(GL_ALPHA_TEST);
    }
    else {
        /* Maçã provisória: um desenho vermelho */
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.85f, 0.05f, 0.05f);

        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.5f);

        glBegin(GL_QUADS);
            glVertex3f(-largura, 0.0f, 0.0f);
            glVertex3f(largura, 0.0f, 0.0f);
            glVertex3f(largura, 0.7f, 0.0f);
            glVertex3f(-largura, 0.7f, 0.0f);
        glEnd();
        glDisable(GL_ALPHA_TEST);
    }

    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

void desenhaPlanta(int x, int y, int z)
{
    float s0, t0, lado;

    /* Obtém as coordenadas da textura da folha no atlas */
    coordenadasDaCelula(CEL_FOLHA, &s0, &t0, &lado);

    /* Desliga o descarte de faces traseiras.
       Assim, a planta aparece dos dois lados. */
    glDisable(GL_CULL_FACE);

    /* Ativa a textura e o teste de transparência */
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5f);

    /* Cor branca para não alterar as cores originais da textura */
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef((float)x, (float)y, (float)z);

    /* Primeiro plano: diagonal de um canto ao outro */
    glBegin(GL_QUADS);

        glTexCoord2f(s0,       t0 + lado);
        glVertex3f(0.15f, 0.0f, 0.15f);

        glTexCoord2f(s0 + lado, t0 + lado);
        glVertex3f(0.85f, 0.0f, 0.85f);

        glTexCoord2f(s0 + lado, t0);
        glVertex3f(0.85f, 0.9f, 0.85f);

        glTexCoord2f(s0,       t0);
        glVertex3f(0.15f, 0.9f, 0.15f);

    glEnd();

    /* Segundo plano: diagonal cruzada com o primeiro */
    glBegin(GL_QUADS);

        glTexCoord2f(s0,       t0 + lado);
        glVertex3f(0.15f, 0.0f, 0.85f);

        glTexCoord2f(s0 + lado, t0 + lado);
        glVertex3f(0.85f, 0.0f, 0.15f);

        glTexCoord2f(s0 + lado, t0);
        glVertex3f(0.85f, 0.9f, 0.15f);

        glTexCoord2f(s0,       t0);
        glVertex3f(0.15f, 0.9f, 0.85f);

    glEnd();

    glPopMatrix();

    /* Restaura os estados usados pelo restante do cenário */
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_CULL_FACE);
}


/* ------------------------------------------------------------------ */
/* A cena inteira, em três passadas. Percorre a matriz do mundo, que  */
/* construirMundo() já preencheu, e desenha cada bloco na passada do  */
/* seu tipo (passadaDoBloco):                                         */
/*                                                                    */
/*   1. opacos            estado normal                               */
/*   2. recorte (folhas)  teste de alfa: o fragmento com alfa até 0,5 */
/*                        é descartado, e o resto é desenhado como    */
/*                        opaco. Sem mistura, a ordem não importa     */
/*   3. semitransparentes vidro e água, com mistura (desenhaTranspa-  */
/*                        rentes), do mais distante ao mais próximo   */
/*                                                                    */
/* A mistura usa a cor que JÁ está no buffer, então tudo o que fica   */
/* atrás do vidro precisa ter sido desenhado antes dele. Por isso os  */
/* opacos vêm primeiro, e o personagem é desenhado entre as passadas  */
/* 2 e 3 (ver desenha). Se o vidro viesse antes, a mistura usaria o   */
/* azul do fundo, e o terreno atrás falharia no teste de profundidade */
/* contra o vidro: desapareceria.                                     */
/* ------------------------------------------------------------------ */

/* desenha todos os blocos de uma passada, na ordem da matriz */
static void desenhaPassada(int passada)
{
    int x, y, z;
    int tipo;

    for (x = 0; x < LADO; x++)
        for (y = 0; y < ALTURA_MUNDO; y++)
            for (z = 0; z < LADO; z++) {

                tipo = mundo[x][y][z];

                if (tipo == VAZIO)
                    continue;

                if (passadaDoBloco(tipo) != passada)
                    continue;

                if (tipo == PLANTA) {
                    desenhaPlanta(x, y, z);
                }
                else {
                    blocoEm(tipo, x, y, z);
                }
            }
}
/* passadas 1 e 2 */
void desenhaOpacos(void)
{
    glDisable(GL_BLEND);                    /* o menu deixa a mistura ligada */

    desenhaPassada(PASSADA_OPACA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5f);          /* só passa o que tem alfa > 0,5 */
    desenhaPassada(PASSADA_RECORTE);
    glDisable(GL_ALPHA_TEST);
}

/* ------------------------------------------------------------------ */
/* Ordem dos semitransparentes. Um vidro desenhado antes de outro que */
/* está atrás dele não se mistura com ele, então os blocos de vidro e */
/* água são ordenados do mais distante ao mais próximo do olho, a     */
/* cada quadro (o olho muda). A distância é a do centro do bloco, ao  */
/* quadrado: para comparar, a raiz não faz diferença.                 */
/* ------------------------------------------------------------------ */

static int maisDistantePrimeiro(const void *a, const void *b)
{
    float da = ((const BlocoTransparente *)a)->distancia;
    float db = ((const BlocoTransparente *)b)->distancia;

    return (da < db) - (da > db);           /* maior distância antes */
}

int listaTransparentes(float olhoX, float olhoY, float olhoZ,
                       BlocoTransparente *lista)
{
    int x, y, z, n = 0;

    for (x = 0; x < LADO; x++)
        for (y = 0; y < ALTURA_MUNDO; y++)
            for (z = 0; z < LADO; z++)
                if (passadaDoBloco(mundo[x][y][z]) == PASSADA_TRANSLUCIDA &&
                    !(mundo[x][y][z] == AGUA && shadersDisponiveis && programaAgua)) {
                    float dx = x + 0.5f - olhoX;
                    float dy = y + 0.5f - olhoY;
                    float dz = z + 0.5f - olhoZ;

                    lista[n].x = x;
                    lista[n].y = y;
                    lista[n].z = z;
                    lista[n].distancia = dx * dx + dy * dy + dz * dz;
                    n++;
                }

    qsort(lista, n, sizeof(BlocoTransparente), maisDistantePrimeiro);
    return n;
}

/* ------------------------------------------------------------------ */
/* Passada 3: mistura proporcional ao alfa,                           */
/*                                                                    */
/*   cor = alfa x nova + (1 - alfa) x existente                       */
/*                                                                    */
/* (glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)). A escrita no  */
/* buffer de profundidade fica desligada (glDepthMask), para um bloco */
/* transparente não esconder o que está atrás dele; o TESTE de        */
/* profundidade continua ligado, para o vidro atrás de uma montanha   */
/* continuar escondido. Ao final, a escrita volta.                    */
/* ------------------------------------------------------------------ */

static BlocoTransparente transparentes[MAX_TRANSPARENTES];

void desenhaTransparentes(void)
{
    float olhoX, olhoY, olhoZ;
    int n, i;

    posicaoDoOlho(&olhoX, &olhoY, &olhoZ);
    n = listaTransparentes(olhoX, olhoY, olhoZ, transparentes);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (i = 0; i < n; i++)
        blocoEm(blocoNoMundo(transparentes[i].x, transparentes[i].y, transparentes[i].z),
                transparentes[i].x, transparentes[i].y, transparentes[i].z);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

/* ------------------------------------------------------------------ */
/* Onde está o olho da câmera (ver CÂMERA): na altura dos olhos em    */
/* primeira pessoa; atrás e acima do personagem em terceira. Usada    */
/* pelo gluLookAt e para ordenar os semitransparentes do mais         */
/* distante ao mais próximo.                                          */
/* ------------------------------------------------------------------ */

void posicaoDoOlho(float *x, float *y, float *z)
{
    *x = jogadorX;                                      /* olhos do personagem */
    *y = jogadorY + ALTURA_OLHOS;
    *z = jogadorZ;

    if (modoCamera == TERCEIRA_PESSOA) {
        float ry = yaw * PI / 180.0f;
        float frenteX = sinf(ry), frenteZ = -cosf(ry);  /* só o plano do chão */

        *x -= frenteX * DISTANCIA_CAMERA;               /* atrás */
        *y += ELEVACAO_CAMERA;                          /* acima, altura fixa */
        *z -= frenteZ * DISTANCIA_CAMERA;
    }
}


/* ================================================================== */
/* ILUMINAÇÃO FIXA E DEMONSTRAÇÃO (BLOCOS 3 E 4)                      */
/*                                                                    */
/* Tudo aqui usa a API clássica do OpenGL 1.0: luzes, materiais,      */
/* normais e glShadeModel. A posição da luz do mundo é enviada depois */
/* de gluLookAt, portanto fica fixa no cenário, não na câmera.        */
/* ================================================================== */

int modoDemonstracao = 0;          /* 0: mundo; 1: cubo e coluna isolados */
int modoComponentesLuz = 3;        /* 1: ambiente; 2: ambiente+difusa; 3: todas */
int modoLuzPontual = 0;             /* 1: lâmpada pontual sobre o mapa; 0: sol ou lua (direcional) */
int modoSombreamento = GL_SMOOTH;  /* GL_FLAT ou GL_SMOOTH */
float anguloDemonstracao = 0.0f;

void atualizaComponentesLuz(void)
{
    /* a luz ambiente vem do modelo de iluminação, que o ciclo de dia e
       noite muda a cada quadro (atualizaCeuELuz); a da LIGHT0 fica zerada
       para a noite não ficar clara */
    const GLfloat ambiente[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const GLfloat difusa[]   = { 0.90f, 0.90f, 0.90f, 1.0f };
    const GLfloat especular[] = { 0.85f, 0.85f, 0.85f, 1.0f };
    const GLfloat desligada[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,
              modoComponentesLuz >= 2 ? difusa : desligada);
    glLightfv(GL_LIGHT0, GL_SPECULAR,
              modoComponentesLuz >= 3 ? especular : desligada);
}

void inicializaIluminacao(void)
{
    const GLfloat ambienteGlobal[] = { 0.12f, 0.12f, 0.12f, 1.0f };
    const GLfloat especularLuz[] = { 0.85f, 0.85f, 0.85f, 1.0f };
    const GLfloat atenuacao[] = { 1.0f, 0.015f, 0.002f };

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambienteGlobal);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
#ifdef GL_LIGHT_MODEL_COLOR_CONTROL
    /* Mantem o brilho especular separado da modulacao da textura. */
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#else
    fprintf(stderr, "Aviso: GL_SEPARATE_SPECULAR_COLOR nao esta disponivel nos headers.\n");
#endif
    glLightfv(GL_LIGHT0, GL_SPECULAR, especularLuz);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, atenuacao[0]);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, atenuacao[1]);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, atenuacao[2]);
    glShadeModel(modoSombreamento);
    atualizaComponentesLuz();
}

/* Chamada depois de posicionaCamera()/gluLookAt: a luz pertence ao mundo. */
void defineLuzDoCenario(void)
{
    const GLfloat posicao[] = { 8.0f, 12.0f, 8.0f,
                                modoLuzPontual ? 1.0f : 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, posicao);
}

/* Material branco difuso preserva as cores do atlas com GL_MODULATE.
   Água e vidro recebem uma componente especular mais forte; no OpenGL
   1.0 ela continua sendo modulada pela textura, sem separação posterior. */
void aplicaMaterial(int tipo)
{
    const GLfloat ambiente[] = { 0.28f, 0.28f, 0.28f, 1.0f };
    const GLfloat difusa[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat especular[] = { 0.12f, 0.12f, 0.12f, 1.0f };
    GLfloat brilho = 12.0f;

    if (tipo == VIDRO) {
        especular[0] = especular[1] = especular[2] = 0.75f;
        brilho = 72.0f;
    } else if (tipo == AGUA) {
        especular[0] = 0.65f; especular[1] = 0.75f; especular[2] = 0.85f;
        brilho = 64.0f;
    } else if (tipo == PEDRA) {
        especular[0] = especular[1] = especular[2] = 0.22f;
        brilho = 18.0f;
    } else if (tipo == NUVEM) {
        especular[0] = especular[1] = especular[2] = 0.18f;
        brilho = 18.0f;
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difusa);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, brilho);
}

/* Coluna de 12 lados. Em GL_SMOOTH, a normal varia em cada vértice;
   em GL_FLAT, cada quadrilátero recebe uma normal constante. */
static void desenhaColunaDemonstracao(float raio, float altura, int lados)
{
    int i;
    if (modoSombreamento == GL_SMOOTH) {
        glBegin(GL_QUAD_STRIP);
        for (i = 0; i <= lados; i++) {
            float a = 2.0f * PI * (float)i / (float)lados;
            float nx = cosf(a), nz = sinf(a);
            glNormal3f(nx, 0.0f, nz);
            glVertex3f(raio * nx, 0.0f, raio * nz);
            glVertex3f(raio * nx, altura, raio * nz);
        }
        glEnd();
    } else {
        for (i = 0; i < lados; i++) {
            float a0 = 2.0f * PI * (float)i / (float)lados;
            float a1 = 2.0f * PI * (float)(i + 1) / (float)lados;
            float am = (a0 + a1) * 0.5f;
            glNormal3f(cosf(am), 0.0f, sinf(am));
            glBegin(GL_QUADS);
            glVertex3f(raio * cosf(a0), 0.0f, raio * sinf(a0));
            glVertex3f(raio * cosf(a0), altura, raio * sinf(a0));
            glVertex3f(raio * cosf(a1), altura, raio * sinf(a1));
            glVertex3f(raio * cosf(a1), 0.0f, raio * sinf(a1));
            glEnd();
        }
    }
}

void desenhaDemonstracao(void)
{
    const GLfloat especularColuna[] = { 0.90f, 0.90f, 0.90f, 1.0f };
    const GLfloat materialAmbiente[] = { 0.22f, 0.22f, 0.22f, 1.0f };
    const GLfloat materialDifuso[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    const GLfloat emissaoAmarela[] = {0.75f,0.48f,0.08f,1.0f};
    const GLfloat semEmissao[] = {0,0,0,1};
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glShadeModel(modoSombreamento);

    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissaoDemo ? emissaoAmarela : semEmissao);
    /* Cubo: normais constantes por face; FLAT e SMOOTH ficam iguais. */
    glPushMatrix();
    glTranslatef(-1.45f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.5f, 0.0f);
    glRotatef(anguloDemonstracao, 0.6f, 1.0f, 0.3f);
    glTranslatef(0.0f, -0.5f, 0.0f);
    glColor3f(0.85f, 0.38f, 0.20f);
    glutSolidCube(1.0);
    glPopMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, semEmissao);

    /* Coluna: normais por vértice em Gouraud, por face em FLAT. */
    glPushMatrix();
    glTranslatef(1.35f, 0.0f, 0.0f);
    glTranslatef(0.0f, 1.25f, 0.0f);
    glRotatef(anguloDemonstracao, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, -1.25f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDifuso);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especularColuna);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 72.0f);
    glColor3f(0.20f, 0.68f, 0.82f);
    desenhaColunaDemonstracao(0.62f, 2.5f, 12);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
}


/* ================================================================== */
/* TEXTURAS PROCEDURAIS E SHADERS GLSL                                */
/* ================================================================== */

static int texturaDoBloco(int tipo)
{
    switch (tipo) {
        case GRAMA: case TERRA: case PEDRA: case TRONCO:
        case FOLHA: case AREIA: case NEVE: case NUVEM:
            return tipo;
        default: return -1;
    }
}

static void corBaseTextura(int tipo, int *r, int *g, int *b)
{
    switch (tipo) {
        case GRAMA: *r=75; *g=145; *b=48; break;
        case TERRA: *r=125; *g=78; *b=43; break;
        case PEDRA: *r=112; *g=116; *b=122; break;
        case TRONCO: *r=112; *g=72; *b=35; break;
        case FOLHA: *r=45; *g=112; *b=38; break;
        case AREIA: *r=218; *g=199; *b=135; break;
        case NEVE: *r=225; *g=238; *b=248; break;
        default: *r=220; *g=232; *b=240; break;
    }
}

static void inicializaTexturas(void)
{
    const int tipos[] = {GRAMA,TERRA,PEDRA,TRONCO,FOLHA,AREIA,NEVE,NUVEM};
    unsigned char pixels[16*16*4];
    int t,x,y,k;
    for (t=0; t<(int)(sizeof(tipos)/sizeof(tipos[0])); ++t) {
        int tipo=tipos[t], br,bg,bb;
        corBaseTextura(tipo,&br,&bg,&bb);
        for (y=0; y<16; ++y) for (x=0; x<16; ++x) {
            int variacao=((x*17+y*29+tipo*13)%23)-11;
            if (tipo==TRONCO) variacao += ((x%4)==0 ? 18 : -3);
            if (tipo==GRAMA && y<4) variacao += 12;
            if (tipo==FOLHA && ((x+y)%5)==0) variacao -= 22;
            if (tipo==PEDRA && ((x*3+y*5)%11)==0) variacao -= 18;
            if (tipo==AREIA) variacao /= 2;
            if (tipo==NEVE || tipo==NUVEM) variacao /= 3;
            k=(y*16+x)*4;
            pixels[k]=(unsigned char)fmaxf(0,fminf(255,br+variacao));
            pixels[k+1]=(unsigned char)fmaxf(0,fminf(255,bg+variacao));
            pixels[k+2]=(unsigned char)fmaxf(0,fminf(255,bb+variacao));
            /* Mascara alfa irregular para os vazados da folhagem. */
            pixels[k+3]=(unsigned char)(tipo==FOLHA &&
                (((x*7+y*11+tipo*3)%9)==0 || ((x+y*2)%13)==0) ? 0 : 255);
        }
        glGenTextures(1,&texturasBlocos[tipo]);
        glBindTexture(GL_TEXTURE_2D,texturasBlocos[tipo]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,16,16,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    }
    glBindTexture(GL_TEXTURE_2D,0);
}

static GLuint compilaShader(GLenum tipo, const char *fonte)
{
    GLuint shader=glCreateShader(tipo);
    GLint sucesso=GL_FALSE, tamanhoLog=0;
    GLchar log[2048];
    glShaderSource(shader,1,&fonte,NULL);
    glCompileShader(shader);
    glGetShaderiv(shader,GL_COMPILE_STATUS,&sucesso);
    if (!sucesso) {
        glGetShaderInfoLog(shader,sizeof(log),&tamanhoLog,log);
        fprintf(stderr,"Erro ao compilar shader: %s\n",log);
        glDeleteShader(shader); return 0;
    }
    return shader;
}

static GLuint criaPrograma(const char *vert,const char *frag)
{
    GLuint vs=compilaShader(GL_VERTEX_SHADER,vert);
    GLuint fs=compilaShader(GL_FRAGMENT_SHADER,frag);
    GLuint programa; GLint sucesso=GL_FALSE, tamanhoLog=0; GLchar log[2048];
    if (!vs || !fs) { if(vs)glDeleteShader(vs); if(fs)glDeleteShader(fs); return 0; }
    programa=glCreateProgram(); glAttachShader(programa,vs); glAttachShader(programa,fs);
    glLinkProgram(programa); glGetProgramiv(programa,GL_LINK_STATUS,&sucesso);
    if (!sucesso) {
        glGetProgramInfoLog(programa,sizeof(log),&tamanhoLog,log);
        fprintf(stderr,"Erro ao vincular programa: %s\n",log);
        glDeleteProgram(programa); programa=0;
    }
    glDeleteShader(vs); glDeleteShader(fs); return programa;
}

static void inicializaShaders(void)
{
    const char *vertAgua=
        "#version 120\n"
        "uniform float uTempo; varying vec2 vUV; varying vec3 vNormal;\n"
        "void main(){ vec4 p=gl_Vertex; float a=p.x*2.2+uTempo*1.5;"
        " float b=p.z*2.8-uTempo*1.1; float o=sin(a)*0.07+cos(b)*0.05;"
        " float dx=0.154*cos(a); float dz=-0.14*sin(b); p.y+=o;"
        " vNormal=normalize(vec3(-dx,1.0,-dz));"
        " gl_Position=gl_ModelViewProjectionMatrix*p; vUV=gl_MultiTexCoord0.st; }\n";
    const char *fragAgua=
        "#version 120\n"
        "uniform float uTempo; uniform float uLuz; varying vec2 vUV; varying vec3 vNormal;"
        "void main(){ float b=0.5+0.5*sin(vUV.x*18.0+vUV.y*13.0+uTempo*2.0);"
        " vec3 c=mix(vec3(0.025,0.20,0.48),vec3(0.12,0.62,0.88),b);"
        " float d=0.45+0.55*max(dot(normalize(vNormal),normalize(vec3(0.3,0.8,0.4))),0.0);"
        " gl_FragColor=vec4(c*d*uLuz,0.62); }\n";
    const char *vertFogo=
        "#version 120\n varying vec2 vUV; void main(){"
        " gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex; vUV=gl_MultiTexCoord0.st; }\n";
    const char *fragFogo=
        "#version 120\n uniform float uTempo; varying vec2 vUV; void main(){"
        " float s=0.08*sin(uTempo*5.0+vUV.y*8.0);"
        " float w=(0.48*(1.0-vUV.y)+0.04)*(1.0+s);"
        " float f=1.0-smoothstep(w*0.55,w,abs(vUV.x-0.5));"
        " float p=0.82+0.18*sin(uTempo*8.0+vUV.y*12.0);"
        " float a=f*(1.0-vUV.y)*p;"
        " vec3 c=mix(vec3(1.0,0.08,0.01),vec3(1.0,0.88,0.12),1.0-vUV.y);"
        " gl_FragColor=vec4(c,a); }\n";
    programaAgua=criaPrograma(vertAgua,fragAgua);
    programaFogo=criaPrograma(vertFogo,fragFogo);
    if (programaAgua && programaFogo) {
        tempoAguaLoc=glGetUniformLocation(programaAgua,"uTempo");
        luzAguaLoc=glGetUniformLocation(programaAgua,"uLuz");
        tempoFogoLoc=glGetUniformLocation(programaFogo,"uTempo");
        shadersDisponiveis=1;
        printf("Shaders de agua e fogo compilados com sucesso.\n");
    } else fprintf(stderr,"Aviso: nao foi possivel iniciar todos os shaders.\n");
}

static void desenhaAgua(void)
{
    int x,y,z,i,j;
    float tempo=glutGet(GLUT_ELAPSED_TIME)/1000.0f;
    if (!shadersDisponiveis || !programaAgua) return;
    glUseProgram(programaAgua); glUniform1f(tempoAguaLoc,tempo);
    glUniform1f(luzAguaLoc,brilhoDoCeu);
    glDisable(GL_LIGHTING); glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE);
    for(x=0;x<LADO;x++) for(y=0;y<ALTURA_MUNDO;y++) for(z=0;z<LADO;z++)
        if(mundo[x][y][z]==AGUA &&
           (y+1>=ALTURA_MUNDO || mundo[x][y+1][z]!=AGUA)) {
            glBegin(GL_QUADS);
            for(i=0;i<4;i++) for(j=0;j<4;j++) {
                float x0=x+i/4.0f,x1=x+(i+1)/4.0f;
                float z0=z+j/4.0f,z1=z+(j+1)/4.0f;
                glTexCoord2f(i/4.0f,j/4.0f); glVertex3f(x0,y+1,z0);
                glTexCoord2f(i/4.0f,(j+1)/4.0f); glVertex3f(x0,y+1,z1);
                glTexCoord2f((i+1)/4.0f,(j+1)/4.0f); glVertex3f(x1,y+1,z1);
                glTexCoord2f((i+1)/4.0f,j/4.0f); glVertex3f(x1,y+1,z0);
            }
            glEnd();
        }
    glDepthMask(GL_TRUE); glDisable(GL_BLEND); glUseProgram(0); glEnable(GL_LIGHTING);
}

static void desenhaFogo(void)
{
    float tempo=glutGet(GLUT_ELAPSED_TIME)/1000.0f;
    float x=10.5f,y=(float)alturaDoTerreno(10,10)+1.0f,z=10.5f;
    int plano;
    if(!shadersDisponiveis || !programaFogo) return;
    glUseProgram(programaFogo); glUniform1f(tempoFogoLoc,tempo);
    glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glDisable(GL_CULL_FACE); glDepthMask(GL_FALSE); glColor4f(1,1,1,1);
    glPushMatrix(); glTranslatef(x,y,z);
    for(plano=0;plano<2;plano++) {
        glPushMatrix(); glRotatef(plano*90.0f,0,1,0); glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex3f(-0.42f,0,0);
        glTexCoord2f(1,0); glVertex3f(0.42f,0,0);
        glTexCoord2f(1,1); glVertex3f(0.42f,1.65f,0);
        glTexCoord2f(0,1); glVertex3f(-0.42f,1.65f,0);
        glEnd(); glPopMatrix();
    }
    glPopMatrix(); glDepthMask(GL_TRUE); glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE); glUseProgram(0); glEnable(GL_LIGHTING);
}

/* ================================================================== */
/* MUNDO                                                              */
/*                                                                    */
/* O cenário: cores e desenho dos blocos, o mapa de alturas, a matriz */
/* 3D do mundo e a construção do terreno, da árvore e das nuvens. A   */
/* casinha de vidro e o desenho da cena em três passadas ficam em     */
/* TEXTURAS E TRANSPARÊNCIA.                                          */
/* ================================================================== */


/* ------------------------------------------------------------------ */
/* Cores sólidas: só para o que não tem textura (nuvem e as partes do */
/* personagem). Escolhe a cor de UMA face a partir de:                */
/*   tipo    qual peça é, define a cor base                           */
/*   brilho  parâmetro mantido por compatibilidade com a função antiga */
/*   tom     pequena variação entre blocos vizinhos (xadrez), para    */
/*           dar para enxergar os blocos de uma nuvem                 */
/* A iluminação real substitui o antigo brilho manual por face.       */
/* ------------------------------------------------------------------ */

void corDaFace(int tipo, float brilho, float tom)
{
    float r, g, b;                          /* vermelho, verde, azul, de 0 a 1 */

    switch (tipo) {
        case NUVEM:   r = 1.00f; g = 1.00f; b = 1.00f; break;
        case PELE:    r = 0.80f; g = 0.60f; b = 0.47f; break;   /* personagem: */
        case CABELO:  r = 0.30f; g = 0.20f; b = 0.12f; break;
        case CAMISA:  r = 0.00f; g = 0.62f; b = 0.66f; break;
        case CALCA:   r = 0.22f; g = 0.22f; b = 0.55f; break;
        case SAPATO:  r = 0.35f; g = 0.35f; b = 0.35f; break;
        case OLHO_BRANCO: r = 0.95f; g = 0.95f; b = 0.95f; break;
        case OLHO:    r = 0.30f; g = 0.25f; b = 0.65f; break;
        case ZUMBI_PELE:   r = 0.30f; g = 0.55f; b = 0.25f; break;  /* inimigo: */
        case ZUMBI_CAMISA: r = 0.15f; g = 0.35f; b = 0.45f; break;
        case ZUMBI_CALCA:  r = 0.25f; g = 0.20f; b = 0.45f; break;
        case ZUMBI_DANO:   r = 0.90f; g = 0.15f; b = 0.15f; break;
        default:     r = 1.00f; g = 0.00f; b = 1.00f; break;   /* rosa = tipo desconhecido */
    }

    (void)brilho; /* a iluminação real substitui o brilho manual por face */
    glColor3f(r * tom, g * tom, b * tom);
}

/* ------------------------------------------------------------------ */
/* Um bloco: cubo de lado 1, do canto (0,0,0) ao canto (1,1,1).       */
/* São 6 faces, cada uma um GL_QUADS de 4 vértices, descritas na      */
/* tabela abaixo. Os vértices de cada face estão em sentido           */
/* anti-horário quando vistos de fora do cubo, começando pelo canto   */
/* de baixo à esquerda. É assim que o OpenGL sabe qual lado da face é */
/* a frente, e o GL_CULL_FACE pode pular as faces viradas de costas   */
/* para a câmera (semanas 2 e 3).                                     */
/*                                                                    */
/* faces diz quais faces desenhar (soma de bits FACE_): blocoEm()     */
/* pula as que um vizinho esconde; o personagem pede TODAS_FACES.     */
/*                                                                    */
/* Iluminação: cada face informa sua normal geométrica. O pipeline    */
/* fixo calcula a intensidade a partir da normal e da fonte de luz.   */
/*                                                                    */
/* Textura: antes de cada glVertex3f vai um glTexCoord2f, com o canto */
/* correspondente da célula da face. A coordenada de textura é estado */
/* corrente aplicado ao próximo vértice, como a cor. Com GL_MODULATE, */
/* cor final = texel x cor iluminada do vértice. O vértice recebe    */
/* branco para preservar a cor original do atlas sob a iluminação.   */
/* A primeira linha do arquivo de imagem corresponde a t = 0, então o */
/* alto da célula é t0 e o pé é t0 + lado: invertido em relação ao y  */
/* da face (t' = 1 - t). Sem isso a grama ficaria embaixo da lateral. */
/* ------------------------------------------------------------------ */

static const int bitDaFace[6] = {
    FACE_FRENTE, FACE_TRAS, FACE_DIREITA, FACE_ESQUERDA, FACE_TOPO, FACE_BASE
};

static const float normaisDaFace[6][3] = {
    { 0.0f, 0.0f, 1.0f },   /* frente */
    { 0.0f, 0.0f,-1.0f },   /* trás */
    { 1.0f, 0.0f, 0.0f },   /* direita */
    {-1.0f, 0.0f, 0.0f },   /* esquerda */
    { 0.0f, 1.0f, 0.0f },   /* topo */
    { 0.0f,-1.0f, 0.0f }    /* base */
};

static const float verticesDaFace[6][4][3] = {
    { {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1} },     /* frente (z = 1) */
    { {1,0,0}, {0,0,0}, {0,1,0}, {1,1,0} },     /* trás (z = 0) */
    { {1,0,1}, {1,0,0}, {1,1,0}, {1,1,1} },     /* direita (x = 1) */
    { {0,0,0}, {0,0,1}, {0,1,1}, {0,1,0} },     /* esquerda (x = 0) */
    { {0,1,1}, {1,1,1}, {1,1,0}, {0,1,0} },     /* topo (y = 1) */
    { {0,0,0}, {1,0,0}, {1,0,1}, {0,0,1} },     /* base (y = 0) */
};

void desenhaBloco(int tipo, float tom, int faces)
{
    int tipoProcedural = usaTexturasProcedurais ? texturaDoBloco(tipo) : -1;
    int texturizado = (tipoProcedural >= 0) ||
                      (celulaDaFace(tipo, FACE_TOPO) != SEM_TEXTURA);
    int f, v;

    aplicaMaterial(tipo);
    if (tipoProcedural >= 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturasBlocos[tipoProcedural]);
    } else if (texturizado) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, atlas);
    } else glDisable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    for (f = 0; f < 6; f++) {
        if (!(faces & bitDaFace[f])) continue;
        /* A normal é estado corrente e vale para os quatro vértices. */
        glNormal3f(normaisDaFace[f][0], normaisDaFace[f][1],
                   normaisDaFace[f][2]);

        if (texturizado) {
            float s[4], t[4];
            if (tipoProcedural >= 0) {
                s[0]=0; s[1]=1; s[2]=1; s[3]=0;
                t[0]=1; t[1]=1; t[2]=0; t[3]=0;
            } else {
                float s0, t0, lado;
                coordenadasDaCelula(celulaDaFace(tipo, bitDaFace[f]), &s0, &t0, &lado);
                s[0]=s0; s[1]=s0+lado; s[2]=s0+lado; s[3]=s0;
                t[0]=t0+lado; t[1]=t0+lado; t[2]=t0; t[3]=t0;
            }
            glColor3f(1.0f, 1.0f, 1.0f);
            for (v = 0; v < 4; v++) {
                glTexCoord2f(s[v], t[v]);
                glVertex3f(verticesDaFace[f][v][0], verticesDaFace[f][v][1], verticesDaFace[f][v][2]);
            }
        } else {
            corDaFace(tipo, 1.0f, tom);
            for (v = 0; v < 4; v++)
                glVertex3f(verticesDaFace[f][v][0], verticesDaFace[f][v][1],
                           verticesDaFace[f][v][2]);
        }
    }
    glEnd();
}


/* ------------------------------------------------------------------ */
/* Instância (semana 3): desenha um bloco do tipo pedido na posição   */
/* (x, y, z) do mundo. Todos os blocos da cena acabam aqui.           */
/*                                                                    */
/*   glPushMatrix   guarda a transformação atual                      */
/*   glTranslatef   desloca a origem para (x, y, z)                   */
/*   desenhaBloco   desenha o cubo de sempre, agora na nova origem    */
/*   glPopMatrix    desfaz o deslocamento, para o próximo bloco       */
/*                  partir do mesmo ponto                             */
/*                                                                    */
/* Como o cubo tem lado 1 e as posições são inteiras, cada bloco      */
/* termina exatamente onde o vizinho começa: eles se encaixam numa    */
/* grade, sem buracos.                                                */
/* ------------------------------------------------------------------ */

void blocoEm(int tipo, int x, int y, int z)
{
    /* soma par: tom normal; soma ímpar: um pouco mais escuro (xadrez).
       Só vale para o que tem cor sólida: a textura já mostra onde cada
       bloco termina */
    float tom = ((x + y + z) % 2 == 0) ? 1.0f : 0.92f;
    int faces = facesVisiveis(x, y, z);     /* sem as que um vizinho esconde */

    if (faces == 0) return;                 /* bloco enterrado: nada a desenhar */

    glPushMatrix();
    glTranslatef((float)x, (float)y, (float)z);
    desenhaBloco(tipo, tom, faces);
    glPopMatrix();
}

/* ------------------------------------------------------------------ */
/* Mapa: o mundo visto de cima. Cada número é a altura de uma coluna  */
/* de blocos. Cada linha é um valor de z (de trás para a frente) e    */
/* cada número na linha é um valor de x (da esquerda para a direita). */
/*                                                                    */
/*   0     mar (areia no fundo, água por cima)                        */
/*   1     praia                                                      */
/*   2+    terra firme com grama no topo; quanto maior, mais alto     */
/*   6+    (NIVEL_NEVE) topo coberto de neve                          */
/*                                                                    */
/* O mapa guarda só a ALTURA de cada coluna. O tipo de cada bloco da  */
/* coluna (grama, terra, pedra...) é decidido depois, em colunaEm().  */
/*                                                                    */
/* O quadrado de x 4 a 7 e z 7 a 10 é todo de altura 2: um chão plano */
/* para a casinha de vidro (ver casinhaEm).                           */
/*                                                                    */
/* Para mudar o cenário, basta editar os números. Experimente pôr um  */
/* 0 no meio do gramado (lago) ou um 9 (torre).                       */
/* ------------------------------------------------------------------ */

int mapa[LADO][LADO] = {
/*  x: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15                 */
    {  3, 3, 4, 4, 5, 5, 5, 4, 4, 3, 3, 2, 2, 2, 3, 3 },   /* z =  0 */
    {  3, 4, 5, 5, 6, 6, 5, 5, 4, 3, 2, 2, 2, 3, 3, 3 },   /* z =  1 */
    {  4, 5, 6, 6, 7, 7, 6, 5, 4, 3, 2, 2, 2, 2, 3, 4 },   /* z =  2 */
    {  4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 2, 2, 2, 2, 3 },   /* z =  3  pico (8) */
    {  4, 5, 6, 7, 7, 7, 6, 5, 3, 3, 2, 2, 2, 2, 2, 2 },   /* z =  4 */
    {  3, 4, 5, 6, 6, 6, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2 },   /* z =  5 */
    {  3, 4, 4, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 2, 2, 2 },   /* z =  6 */
    {  3, 3, 4, 4, 2, 2, 2, 2, 2, 2, 1, 0, 0, 1, 2, 2 },   /* z =  7  laguinho */
    {  2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 0, 0, 1, 2, 2 },   /* z =  8 */
    {  2, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2 },   /* z =  9 */
    {  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },   /* z = 10 */
    {  1, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2 },   /* z = 11 */
    {  1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1 },   /* z = 12  praia */
    {  0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0 },   /* z = 13 */
    {  0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },   /* z = 14  mar */
    {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },   /* z = 15 */
};

/* Consulta a altura de um quadrado do mapa. Atenção à ordem dos
   índices: primeiro a linha (z), depois a coluna (x). É por isso que a
   tabela acima aparece no código do mesmo jeito que o mundo visto de
   cima. */
int alturaDoTerreno(int x, int z)
{
    return mapa[z][x];
}

/* ------------------------------------------------------------------ */
/* Matriz do mundo: o tipo de cada bloco, indexado como               */
/* mundo[x][y][z] (o mapa é [z][x]). A célula que contém um ponto é   */
/* (floor(x), floor(y), floor(z)), por isso a colisão é uma consulta  */
/* direta à matriz. Todo acesso passa por colocaBloco() e             */
/* blocoNoMundo(), que testam os limites.                             */
/* ------------------------------------------------------------------ */

unsigned char mundo[LADO][ALTURA_MUNDO][LADO];

/* 1 se (x, y, z) é uma posição válida da matriz                      */
int dentroDoMundo(int x, int y, int z)
{
    return x >= 0 && x < LADO &&
           y >= 0 && y < ALTURA_MUNDO &&
           z >= 0 && z < LADO;
}

/* Grava um bloco na matriz; posições fora do mundo são ignoradas     */
void colocaBloco(int tipo, int x, int y, int z)
{
    if (dentroDoMundo(x, y, z))
        mundo[x][y][z] = (unsigned char)tipo;
}

/* Lê um bloco da matriz; fora do mundo não há bloco                  */
int blocoNoMundo(int x, int y, int z)
{
    if (!dentroDoMundo(x, y, z)) return VAZIO;
    return mundo[x][y][z];
}

/* ------------------------------------------------------------------ */
/* Uma coluna do terreno na posição (x, z) do chão.                   */
/* Lê a altura h no mapa e coloca blocos de y = 0 até o topo. Para    */
/* cada andar y, a sequência de if escolhe o tipo comparando y com h: */
/*                                                                    */
/*   y acima do chão (y > h)      AGUA   só acontece em colunas mais  */
/*                                       baixas que NIVEL_AGUA (mar)  */
/*   coluna baixa (h <= NIVEL)    AREIA  praia e fundo do mar         */
/*   topo de coluna alta          NEVE   y == h e h >= NIVEL_NEVE     */
/*   último andar (y == h)        GRAMA  o topo das demais colunas    */
/*   2 andares abaixo do topo     TERRA                               */
/*   o resto, mais embaixo        PEDRA                               */
/*                                                                    */
/* A ordem dos if importa: o primeiro que for verdadeiro decide. Por  */
/* isso a NEVE vem antes da GRAMA; se viesse depois, nunca seria      */
/* escolhida, porque o topo já teria virado grama.                    */
/*                                                                    */
/* Exemplo, coluna de altura 6: PEDRA de y = 0 a 3, TERRA em 4 e 5,   */
/* NEVE em 6. Coluna de altura 4: PEDRA em 0 e 1, TERRA em 2 e 3,     */
/* GRAMA em 4. Coluna do mar (altura 0): AREIA em 0, AGUA em 1.       */
/* ------------------------------------------------------------------ */

void colunaEm(int x, int z)
{
    int h = alturaDoTerreno(x, z);
    int y;

    /* sobe até o topo da coluna, ou até o nível da água se ela for mais
       baixa, para preencher o mar */
    for (y = 0; y <= h || y <= NIVEL_AGUA; y++) {
        if (y > h)                colocaBloco(AGUA,  x, y, z);
        else if (h <= NIVEL_AGUA) colocaBloco(AREIA, x, y, z);
        else if (y == h && h >= NIVEL_NEVE) colocaBloco(NEVE, x, y, z);
        else if (y == h)          colocaBloco(GRAMA, x, y, z);
        else if (y >= h - 2)      colocaBloco(TERRA, x, y, z);
        else                      colocaBloco(PEDRA, x, y, z);
    }
}

/* ------------------------------------------------------------------ */
/* Árvore no formato do Minecraft, plantada no quadrado (x, z).       */
/* Primeiro descobre a altura do chão ali, para a árvore nascer em    */
/* cima da grama. Depois empilha, a partir do chão:                   */
/*                                                                    */
/*   chao+7          F            1 folha no topo                     */
/*   chao+6       F  F  F         camada 3 x 3                        */
/*   chao+5    F  F  T  F  F      camada 5 x 5, tronco no meio        */
/*   chao+4    F  F  T  F  F      camada 5 x 5, tronco no meio        */
/*   chao+3          T                                                */
/*   chao+2          T                                                */
/*   chao+1          T            tronco: 5 blocos                    */
/*   chao      (grama do mapa)                                        */
/*                                                                    */
/* dx e dz são deslocamentos em relação ao tronco: dx = -2 é dois     */
/* blocos à esquerda, dx = 2 é dois à direita, e assim por diante.    */
/* ------------------------------------------------------------------ */

void arvoreEm(int x, int z)
{
    int chao = alturaDoTerreno(x, z);
    int y, dx, dz;

    /* tronco: mesma posição (x, z), subindo 5 andares */
    for (y = chao + 1; y <= chao + 5; y++)
        colocaBloco(TRONCO, x, y, z);

    /* duas camadas largas de folhas, de 2 para cada lado do tronco */
    for (y = chao + 4; y <= chao + 5; y++)
        for (dx = -2; dx <= 2; dx++)
            for (dz = -2; dz <= 2; dz++)
                if (dx != 0 || dz != 0)     /* pula o centro: ali está o tronco */
                    colocaBloco(FOLHA, x + dx, y, z + dz);

    /* camada menor, de 1 para cada lado; o tronco já acabou, então o
       centro também recebe folha */
    for (dx = -1; dx <= 1; dx++)
        for (dz = -1; dz <= 1; dz++)
            colocaBloco(FOLHA, x + dx, chao + 6, z + dz);

    /* folha do topo, bem em cima do tronco */
    colocaBloco(FOLHA, x, chao + 7, z);
}

/* ------------------------------------------------------------------ */
/* Nuvem: uma placa retangular de blocos brancos, sempre na altura    */
/* y = ALTURA_NUVEM, como as nuvens achatadas do Minecraft.           */
/*   (x, z)    canto da nuvem no mapa                                 */
/*   largura   quantos blocos ela ocupa em x                          */
/*   fundo     quantos blocos ela ocupa em z                          */
/* Exemplo: nuvemEm(1, 2, 5, 3) ocupa x de 1 a 5 e z de 2 a 4.        */
/* ------------------------------------------------------------------ */

void nuvemEm(int x, int z, int largura, int fundo)
{
    int dx, dz;

    for (dx = 0; dx < largura; dx++)
        for (dz = 0; dz < fundo; dz++)
            colocaBloco(NUVEM, x + dx, ALTURA_NUVEM, z + dz);
}

/* ------------------------------------------------------------------ */
/* Preenche a matriz uma vez, no início: zera tudo e coloca o         */
/* terreno, a árvore, a casinha de vidro e as nuvens. A árvore e a   */
/* casinha vêm depois do terreno e escrevem por cima.                 */
/* ------------------------------------------------------------------ */

void construirMundo(void)
{
    int x, y, z;

    for (x = 0; x < LADO; x++)
        for (y = 0; y < ALTURA_MUNDO; y++)
            for (z = 0; z < LADO; z++)
                mundo[x][y][z] = VAZIO;

    for (x = 0; x < LADO; x++)
        for (z = 0; z < LADO; z++)
            colunaEm(x, z);

    arvoreEm(13, 6);                        /* no gramado, à direita da montanha */
    casinhaEm(4, 7);                        /* à esquerda de onde o personagem começa */
 
    colocaBloco(PLANTA, 2, 3, 10);          /* Plantas espalhadas pelo gramado */
    colocaBloco(PLANTA, 3, 3, 10);
    colocaBloco(PLANTA, 9, 3, 10);
    colocaBloco(PLANTA, 10, 3, 9);          /* (10, 10) é da fogueira */
    colocaBloco(PLANTA, 8, 3, 12);
    colocaBloco(PLANTA, 14, 3, 10);
   
    /* itens soltos no gramado, para recolher (semana 8) */
    criaItem(PLANTA, 11.5f, 3.05f, 10.5f);  /* muda */
    criaItem(TERRA,  12.5f, 3.05f, 10.5f);
    criaItem(MACA,   13.5f, 3.05f, 10.5f);

    nuvemEm(1, 2, 5, 3);                    /* sobre a montanha */
    nuvemEm(10, 0, 4, 2);                   /* sobre a árvore, mais ao fundo */
}


/* ================================================================== */
/* COLISÃO                                                            */
/*                                                                    */
/* A posição do personagem e a colisão com o mundo: que blocos são    */
/* sólidos, se a caixa do personagem cabe numa posição e como aplicar */
/* um deslocamento sem atravessar paredes.                            */
/* ================================================================== */


/* ------------------------------------------------------------------ */
/* Posição do personagem                                              */
/* jogadorY é a altura dos PÉS; os olhos ficam ALTURA_OLHOS acima.    */
/* Só moveJogador() a altera, sempre depois de testar a colisão.      */
/* ------------------------------------------------------------------ */

float jogadorX = INICIO_X;  /* começa no gramado, no meio em x,    */
float jogadorY = INICIO_Y;  /* com os pés sobre a grama (coluna de */
float jogadorZ = INICIO_Z;  /* altura 2, chão em y = 3); com yaw 0, */
                            /* olha para a montanha                */

/* Física vertical (semana 9). A altura deixa de ser controlada por
   teclas: velocidadeY (blocos/s) muda com a gravidade e jogadorNoChao
   diz se há um bloco sob os pés. Ambos são atualizados em moveJogador()
   e lidos em atualiza(). */
float velocidadeY = 0.0f;
int   jogadorNoChao = 0;

/* ------------------------------------------------------------------ */
/* Uma célula impede a passagem? Fora do mundo conta como sólido      */
/* (paredes nas bordas, piso e teto); dentro, VAZIO e AGUA deixam     */
/* passar.                                                            */
/* ------------------------------------------------------------------ */

int blocoSolido(int x, int y, int z)
{
    int tipo;

    if (!dentroDoMundo(x, y, z)) return 1;  /* fora do mundo: parede */

    tipo = mundo[x][y][z];
    return tipo != VAZIO && tipo != AGUA && tipo != PLANTA;
}

/* ------------------------------------------------------------------ */
/* Detecção de colisão: a caixa do personagem (AABB, alinhada aos     */
/* eixos, sem girar) com os pés em (x, y, z) toca algum bloco sólido? */
/* Em cada eixo a caixa cobre as células de floor(min) a floor(max),  */
/* e basta consultar essas células na matriz (no máximo 12).          */
/*                                                                    */
/* floorf, e não (int), porque o cast erra para coordenadas           */
/* negativas. FOLGA para que encostar numa face não conte como        */
/* sobrepor.                                                          */
/* ------------------------------------------------------------------ */

#define FOLGA 0.001f    /* encostar não é sobrepor */

/* Caixa qualquer, de largura x altura x largura, com os pés em
   (x, y, z). Serve ao personagem, aos inimigos e aos itens. */
int caixaLivre(float x, float y, float z, float largura, float altura)
{
    float meia = largura / 2.0f;
    int x0 = (int)floorf(x - meia), x1 = (int)floorf(x + meia - FOLGA);
    int y0 = (int)floorf(y),        y1 = (int)floorf(y + altura - FOLGA);
    int z0 = (int)floorf(z - meia), z1 = (int)floorf(z + meia - FOLGA);
    int cx, cy, cz;

    for (cx = x0; cx <= x1; cx++)
        for (cy = y0; cy <= y1; cy++)
            for (cz = z0; cz <= z1; cz++)
                if (blocoSolido(cx, cy, cz))
                    return 0;               /* a caixa toca um bloco sólido */
    return 1;
}

/* A caixa do personagem */
int posicaoLivre(float x, float y, float z)
{
    return caixaLivre(x, y, z, LARGURA_JOGADOR, ALTURA_JOGADOR);
}

/* ------------------------------------------------------------------ */
/* Resposta à colisão: o movimento é testado antes de ser aplicado,   */
/* um eixo por vez (x, z, y). Se um eixo bloqueia, os outros          */
/* continuam, e a caixa desliza rente à parede em vez de travar       */
/* na diagonal.                                                       */
/*                                                                    */
/* Deslocamentos maiores que PASSO_MAXIMO são divididos em pedaços,   */
/* para nenhum teste saltar por cima de uma parede (tunelamento).     */
/*                                                                    */
/* No eixo vertical (semana 9) o teste é o mesmo, mas a resposta diz  */
/* mais: descendo e bloqueado, há chão sob os pés, e a caixa é        */
/* apoiada no topo do bloco; subindo e bloqueado, a cabeça bateu no   */
/* teto. moveCaixa devolve os eixos bloqueados (BLOQ_), e quem chama  */
/* zera a velocidade vertical.                                        */
/* ------------------------------------------------------------------ */

/* Aplica um pedaço do deslocamento, já pequeno o bastante para não
   atravessar paredes */
static int movePedaco(float *x, float *y, float *z, float largura, float altura,
                      float dx, float dy, float dz)
{
    int bloqueios = 0;

    if (caixaLivre(*x + dx, *y, *z, largura, altura)) *x += dx;
    else if (dx != 0.0f) bloqueios |= BLOQ_X;

    if (caixaLivre(*x, *y, *z + dz, largura, altura)) *z += dz;
    else if (dz != 0.0f) bloqueios |= BLOQ_Z;

    if (caixaLivre(*x, *y + dy, *z, largura, altura)) {
        *y += dy;
    } else if (dy < 0.0f) {
        /* o passo é menor que 1 bloco, então os pés entraram só na célula
           de y = floor(y + dy): o topo dela é o novo chão */
        *y = floorf(*y + dy) + 1.0f;
        bloqueios |= BLOQ_BAIXO;
    } else if (dy > 0.0f) {
        bloqueios |= BLOQ_CIMA;
    }
    return bloqueios;
}

int moveCaixa(float *x, float *y, float *z, float largura, float altura,
              float dx, float dy, float dz)
{
    float comprimento = sqrtf(dx * dx + dy * dy + dz * dz);
    int pedacos = (int)ceilf(comprimento / PASSO_MAXIMO);
    int bloqueios = 0, i;

    if (pedacos < 1) pedacos = 1;           /* parado: um pedaço nulo */

    for (i = 0; i < pedacos; i++)
        bloqueios |= movePedaco(x, y, z, largura, altura,
                                dx / pedacos, dy / pedacos, dz / pedacos);
    return bloqueios;
}

void moveJogador(float dx, float dy, float dz)
{
    int b = moveCaixa(&jogadorX, &jogadorY, &jogadorZ,
                      LARGURA_JOGADOR, ALTURA_JOGADOR, dx, dy, dz);

    jogadorNoChao = (b & BLOQ_BAIXO) != 0;
    if (b & (BLOQ_BAIXO | BLOQ_CIMA))
        velocidadeY = 0.0f;                 /* pisou ou bateu a cabeça */
}

/* ================================================================== */
/* CÂMERA                                                             */
/*                                                                    */
/* Tudo o que decide COMO a cena é vista: a lente (projeção), a       */
/* direção do olhar a partir de yaw e pitch, a posição da câmera em   */
/* primeira e terceira pessoa e o desenho do personagem, que só aparece */
/* em terceira pessoa.                                                */
/* ================================================================== */


/* ------------------------------------------------------------------ */
/* Orientação e modo de câmera                                        */
/* yaw é o giro em torno do eixo vertical (0 olha para -z) e pitch a  */
/* inclinação (0 olha na horizontal, positivo para cima). O teclado   */
/* e o mouse os alteram; posicionaCamera() os usa a cada quadro.      */
/* ------------------------------------------------------------------ */

float yaw   = 0.0f;
float pitch = 0.0f;
int modoCamera = TERCEIRA_PESSOA;    /* começa vendo o personagem */

/* ------------------------------------------------------------------ */
/* Janela e projeção                                                  */
/* ------------------------------------------------------------------ */

int larguraJanela = 1024;       /* atualizados em redimensiona() */
int alturaJanela  = 768;
int projecaoOrtogonal = 0;      /* 0 perspectiva, 1 ortográfica (tecla P) */
float campoDeVisao = 60.0f;     /* graus; alterado pela roda do mouse */

/* ------------------------------------------------------------------ */
/* Projeção: monta a "lente" da câmera na matriz GL_PROJECTION.       */
/* Perspectiva (gluPerspective): campo de visão, proporção da janela, */
/* plano próximo e distante; o que está longe fica menor. Ortográfica */
/* (glOrtho): uma caixa, em que o tamanho não depende da distância. A */
/* proporção evita a imagem esticada. O plano próximo é 0.1 nos dois  */
/* modos: positivo, senão blocos atrás do olho entrariam na imagem.   */
/* Chamada ao redimensionar, na tecla P e no zoom.                    */
/* ------------------------------------------------------------------ */

void aplicaProjecao(void)
{
    double aspecto = (double)larguraJanela / alturaJanela;

    glMatrixMode(GL_PROJECTION);            /* as próximas operações mexem na lente */
    glLoadIdentity();

    if (projecaoOrtogonal)
        glOrtho(-10.0 * aspecto, 10.0 * aspecto, -10.0, 10.0, 0.1, 200.0);
    else
        gluPerspective(campoDeVisao, aspecto, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);             /* volta para a matriz dos objetos */
}

/* ------------------------------------------------------------------ */
/* Redimensionamento (semana 4)                                       */
/* A GLUT chama esta função ao abrir a janela e sempre que ela muda   */
/* de tamanho. Guarda o tamanho (a projeção e o centro do mouse       */
/* dependem dele), ajusta a área de desenho e refaz a lente.          */
/* ------------------------------------------------------------------ */

void redimensiona(int largura, int altura)
{
    if (altura == 0) altura = 1;            /* evita divisão por zero */

    larguraJanela = largura;
    alturaJanela  = altura;

    glViewport(0, 0, largura, altura);      /* desenha na janela inteira */
    aplicaProjecao();
}

/* ------------------------------------------------------------------ */
/* Direção do olhar a partir de yaw e pitch (coordenadas esféricas).  */
/* O pitch divide o vetor unitário entre vertical, sen(pitch), e      */
/* horizontal, cos(pitch); o yaw distribui a parte horizontal entre x */
/* e z, com yaw 0 olhando para -z. Quem chama soma o vetor ao olho    */
/* para obter o alvo do gluLookAt.                                    */
/* ------------------------------------------------------------------ */

void direcaoDoOlhar(float *dx, float *dy, float *dz)
{
    float ry = yaw   * PI / 180.0f;
    float rp = pitch * PI / 180.0f;

    *dx =  sinf(ry) * cosf(rp);
    *dy =  sinf(rp);
    *dz = -cosf(ry) * cosf(rp);
}

/* ------------------------------------------------------------------ */
/* Personagem, desenhado só em terceira pessoa, como modelo           */
/* hierárquico. O corpo é levado aos pés do jogador e girado por      */
/* -yaw, para a frente local (-z) coincidir com a do olhar. Cada      */
/* braço e perna é desenhado pendurado abaixo da origem, girado ali e */
/* só então levado à articulação, para girar em torno do ombro ou do  */
/* quadril.                                                           */
/*                                                                    */
/* O balanço é BALANCO_MAXIMO x amplitude x sen(fase); a fase avança  */
/* com a distância andada (ver atualiza). Tudo fica a menos de 0.3 do */
/* eixo vertical, dentro da caixa de colisão em qualquer giro.        */
/* ------------------------------------------------------------------ */

/* Uma peça do boneco: caixa com o canto mínimo em (x, y, z) do sistema
   local atual e as medidas dadas */
static void caixa(int tipo, float x, float y, float z,
                  float largura, float altura, float profundidade)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(largura, altura, profundidade);
    desenhaBloco(tipo, 1.0f, TODAS_FACES);
    glPopMatrix();
}

/* Braço ou perna pendurado na articulação (x, y), girado 'angulo' graus
   em torno do eixo x (positivo: para a frente). A parte de cima tem a
   cor tipoCima; os últimos alturaBaixo, a cor tipoBaixo. */
static void membro(float x, float y, float angulo, float largura,
                   int tipoCima, int tipoBaixo, float alturaBaixo)
{
    float meiaLargura = largura / 2.0f;
    float meiaProf = PROFUNDIDADE_CORPO / 2.0f;
    float alturaCima = COMPRIMENTO_MEMBRO - alturaBaixo;

    glPushMatrix();
    glTranslatef(x, y, 0.0f);                   /* 3. até a articulação */
    glRotatef(angulo, 1.0f, 0.0f, 0.0f);        /* 2. gira em torno dela */
    caixa(tipoCima,  -meiaLargura, -alturaCima, -meiaProf,       /* 1. pendurado */
          largura, alturaCima, PROFUNDIDADE_CORPO);
    caixa(tipoBaixo, -meiaLargura, -COMPRIMENTO_MEMBRO, -meiaProf,
          largura, alturaBaixo, PROFUNDIDADE_CORPO);
    glPopMatrix();
}

void desenhaJogador(void)
{
    float quadril = COMPRIMENTO_MEMBRO;                 /* altura das articulações */
    float ombro   = 2.0f * COMPRIMENTO_MEMBRO;
    float meiaProf = PROFUNDIDADE_CORPO / 2.0f;
    float meiaCabeca = LADO_CABECA / 2.0f;
    float xPerna = LARGURA_PERNA / 2.0f;                /* centro de cada perna */
    float xBraco = (LARGURA_TRONCO + LARGURA_BRACO) / 2.0f;
    float angulo = BALANCO_MAXIMO * amplitudeCaminhada * sinf(faseCaminhada);

    glPushMatrix();
    glTranslatef(jogadorX, jogadorY, jogadorZ);         /* pés, centro da caixa */
    glRotatef(-yaw, 0.0f, 1.0f, 0.0f);                  /* frente do boneco = -z local */

    /* pernas (x negativo é a esquerda, x positivo a direita) */
    membro(-xPerna, quadril,  angulo, LARGURA_PERNA, CALCA, SAPATO, 0.1f);
    membro( xPerna, quadril, -angulo, LARGURA_PERNA, CALCA, SAPATO, 0.1f);

    /* tronco */
    caixa(CAMISA, -LARGURA_TRONCO / 2.0f, quadril, -meiaProf,
          LARGURA_TRONCO, COMPRIMENTO_MEMBRO, PROFUNDIDADE_CORPO);

    /* braços: manga curta e mão, em sentido oposto à perna do mesmo lado */
    membro(-xBraco, ombro, -angulo, LARGURA_BRACO, CAMISA, PELE, 0.45f);
    membro( xBraco, ombro,  angulo, LARGURA_BRACO, CAMISA, PELE, 0.45f);

    /* cabeça, de ombro até ombro + LADO_CABECA (perto de ALTURA_JOGADOR) */
    caixa(PELE, -meiaCabeca, ombro, -meiaCabeca,
          LADO_CABECA, LADO_CABECA, LADO_CABECA);

    /* cabelo: casca 0.01 maior dos lados, em cima e atrás, da altura dos
       olhos para cima; a frente dela fica dentro da cabeça, atrás do rosto */
    caixa(CABELO, -meiaCabeca - 0.01f, ombro + 0.15f, -meiaCabeca + 0.01f,
          LADO_CABECA + 0.02f, LADO_CABECA - 0.14f, LADO_CABECA);

    /* franja: faixa de cabelo no alto da testa */
    caixa(CABELO, -meiaCabeca, ombro + LADO_CABECA - 0.05f, -meiaCabeca - 0.01f,
          LADO_CABECA, 0.05f, 0.01f);

    /* olhos, na altura ALTURA_OLHOS: branco por fora, íris por dentro */
    caixa(OLHO_BRANCO, -0.14f, ALTURA_OLHOS - 0.03f, -meiaCabeca - 0.01f,
          0.1f, 0.05f, 0.01f);
    caixa(OLHO_BRANCO,  0.04f, ALTURA_OLHOS - 0.03f, -meiaCabeca - 0.01f,
          0.1f, 0.05f, 0.01f);
    caixa(OLHO, -0.09f, ALTURA_OLHOS - 0.03f, -meiaCabeca - 0.015f,
          0.05f, 0.05f, 0.01f);
    caixa(OLHO,  0.04f, ALTURA_OLHOS - 0.03f, -meiaCabeca - 0.015f,
          0.05f, 0.05f, 0.01f);

    glPopMatrix();
}

/* ------------------------------------------------------------------ */
/* Câmera: gluLookAt(olho, alvo, cima) com os mesmos dados nos dois   */
/* modos. Primeira pessoa: olho na altura dos olhos, alvo logo à      */
/* frente na direção do olhar. Terceira pessoa: olho DISTANCIA_CAMERA */
/* atrás, só no plano do chão, e ELEVACAO_CAMERA acima, para a câmera */
/* nunca entrar no chão; o alvo é a cabeça, deslocada na vertical     */
/* pelo pitch.                                                        */
/* ------------------------------------------------------------------ */

void posicionaCamera(void)
{
    float dx, dy, dz, olhoX, olhoY, olhoZ;
    float cabecaX = jogadorX;
    float cabecaY = jogadorY + ALTURA_OLHOS;
    float cabecaZ = jogadorZ;

    direcaoDoOlhar(&dx, &dy, &dz);
    posicaoDoOlho(&olhoX, &olhoY, &olhoZ);

    if (modoCamera == PRIMEIRA_PESSOA) {
        gluLookAt(olhoX, olhoY, olhoZ,
                  olhoX + dx, olhoY + dy, olhoZ + dz,
                  0.0, 1.0, 0.0);
    } else {
        gluLookAt(olhoX, olhoY, olhoZ,
                  cabecaX,                               /* olha o personagem, */
                  cabecaY + dy * DISTANCIA_CAMERA,       /* inclinado pelo pitch */
                  cabecaZ,
                  0.0, 1.0, 0.0);
    }
}

/* ------------------------------------------------------------------ */
/* Configuração do ciclo de dia e noite: define a estrutura Cor,       */
/* utilizada para representar cores RGB, e a função lerp(), que      */
/* realiza a interpolação entre duas cores. Também são definidos     */
/* o ângulo e a inclinação do percurso solar, a duração do ciclo e    */
/* as cores utilizadas para representar o céu e a iluminação durante */
/* o dia e a noite. Essas variáveis são atualizadas conforme a        */
/* passagem do tempo, permitindo a transição gradual entre os períodos.*/
/* ------------------------------------------------------------------ */

typedef struct { float r, g, b; } Cor;

static Cor lerp(Cor a, Cor b, float t)
{
Cor c = { a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
              a.b + (b.b - a.b) * t };
    return c;
}

#define INCLINACAO_SOL (25.0f * PI / 180.0f)    /* o caminho do sol é inclinado, para as faces +z e -z também receberem luz */

float brilhoDoCeu = 1.0f;       /* 0,25 noite a 1 dia; escurece a água */
float anguloSol = 60.0f;        /* graus: 0 nasce, 90 meio-dia, 180 se põe, 270 meia-noite */
float duracaoCiclo = 180.0f;    /* segundos por volta; a tecla T alterna com 20 */

Cor corHorizonte = { 0.53f, 0.81f, 0.92f };     /* a cada quadro, essas cores são atualizadas */
Cor corZenite    = { 0.25f, 0.50f, 0.90f };

static const Cor zenDia   = { 0.25f, 0.50f, 0.90f }, zenNoite = { 0.01f, 0.01f, 0.06f };
static const Cor horDia   = { 0.53f, 0.81f, 0.92f }, horNoite = { 0.04f, 0.05f, 0.12f };
static const Cor luzDia   = { 1.00f, 0.96f, 0.88f }, luzNoite = { 0.10f, 0.12f, 0.25f };
static const Cor ambDia   = { 0.35f, 0.35f, 0.40f }, ambNoite = { 0.07f, 0.07f, 0.14f };

/* ------------------------------------------------------------------ */
/* Carregamento dos astros: a função carregaAstro() lê as imagens do   */
/* sol e da lua no formato PPM e converte seus dados para RGBA.        */
/* Uma cor específica (magenta) é utilizada como transparência,       */
/* permitindo que apenas o desenho dos astros seja exibido, sem       */
/* o fundo da imagem. As texturas são enviadas para a GPU e utilizam   */
/* filtragem NEAREST para preservar a aparência pixelada do jogo.     */
/* ------------------------------------------------------------------ */

GLuint texSol, texLua;

static GLuint carregaAstro(const char *nome)
{
    int l, a, i;
    unsigned char *rgb = lerPPM(nome, &l, &a);
    unsigned char *rgba;
    GLuint tex;

    if (rgb == NULL) { printf("Erro: não consegui ler %s\n", nome); exit(1); }

    rgba = (unsigned char *)malloc(4 * l * a);
    for (i = 0; i < l * a; i++) {
        rgba[4*i]   = rgb[3*i];
        rgba[4*i+1] = rgb[3*i+1];
        rgba[4*i+2] = rgb[3*i+2];
        rgba[4*i+3] = (rgb[3*i] == 255 && rgb[3*i+1] == 0 && rgb[3*i+2] == 255) ? 0 : 255;
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, l, a, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   /* pixelado, como o resto */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, atlas);

    free(rgb);
    free(rgba);
    return tex;
}

/* ------------------------------------------------------------------ */
/* Cálculo da posição solar e atualização da iluminação: a função    */
/* direcaoDoSol() determina a direção da luz conforme o ângulo do sol, */
/* considerando a inclinação do seu percurso. A função suave()       */
/* suaviza as transições entre os valores, evitando mudanças bruscas. */
/* Já atualizaCeuELuz() utiliza a altura do sol para interpolar as     */
/* cores do céu, da luz ambiente e da iluminação direta. Também       */
/* atualiza a posição da fonte de luz, a cor da neblina e a cor usada  */
/* para limpar a tela, fazendo o ambiente acompanhar o ciclo.         */
/* ------------------------------------------------------------------ */

static void direcaoDoSol(float a, float *x, float *y, float *z)
{
    *x = cosf(a);
    *y = sinf(a) * cosf(INCLINACAO_SOL);
    *z = sinf(a) * sinf(INCLINACAO_SOL);
}

static float suave(float t)                     /* 0..1 com transição suave */
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

void atualizaCeuELuz(void)
{
    float a = anguloSol * PI / 180.0f;
    float altura = sinf(a);                             /* +1 meio-dia, -1 meia-noite */
    float t = suave((altura + 0.15f) / 0.35f);          /* 0 noite, 1 dia */
    float forca = suave(fabsf(altura) / 0.25f);         /* a luz direta some perto do horizonte */
    float sx, sy, sz, sinal = (altura >= 0.0f) ? 1.0f : -1.0f;
    Cor luz, amb;
    GLfloat pos[4], dif[4], esp[4], am4[4], neb[4];

    direcaoDoSol(a, &sx, &sy, &sz);

    corZenite    = lerp(zenNoite, zenDia, t);
    corHorizonte = lerp(horNoite, horDia, t);
    luz = lerp(luzNoite, luzDia, t);
    amb = lerp(ambNoite, ambDia, t);

    /* de dia a luz vem do sol; de noite, da lua (lado oposto) */
    pos[0] = sinal * sx; pos[1] = sinal * sy; pos[2] = sinal * sz; pos[3] = 0.0f;
    dif[0] = luz.r * forca; dif[1] = luz.g * forca; dif[2] = luz.b * forca; dif[3] = 1.0f;
    am4[0] = amb.r; am4[1] = amb.g; am4[2] = amb.b; am4[3] = 1.0f;
    neb[0] = corHorizonte.r; neb[1] = corHorizonte.g; neb[2] = corHorizonte.b; neb[3] = 1.0f;

    if (modoLuzPontual) {                               /* tecla O: a lâmpada */
        defineLuzDoCenario();
        dif[0] = dif[1] = dif[2] = 0.9f;
    } else {
        glLightfv(GL_LIGHT0, GL_POSITION, pos);         /* depois do gluLookAt! */
    }
    /* teclas 1, 2 e 3 na demonstração: só ambiente, + difusa, + especular */
    esp[0] = dif[0] * 0.85f; esp[1] = dif[1] * 0.85f; esp[2] = dif[2] * 0.85f; esp[3] = 1.0f;
    if (modoComponentesLuz < 3) esp[0] = esp[1] = esp[2] = 0.0f;
    if (modoComponentesLuz < 2) dif[0] = dif[1] = dif[2] = 0.0f;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, esp);
    brilhoDoCeu = 0.25f + 0.75f * t;                    /* para o shader da água */
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, am4);
    glFogfv(GL_FOG_COLOR, neb);                         /* neblina = cor do horizonte */
    glClearColor(neb[0], neb[1], neb[2], 1.0f);
}

/* ------------------------------------------------------------------ */
/* Representação dos astros: a função astro() desenha o sol ou a lua  */
/* utilizando uma textura aplicada sobre um quadrilátero. O tamanho e */
/* a distância do objeto são definidos pelos parâmetros da função.   */
/* O quadrilátero é desenhado em um plano perpendicular ao eixo X,    */
/* permitindo representar os astros no domo celeste.                 */
/* ------------------------------------------------------------------ */

static void astro(GLuint tex, float tam, float dist)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);                  /* quadrado no plano x = dist, de frente para a origem */
    glTexCoord2f(0, 1); glVertex3f(dist, -tam, -tam);
    glTexCoord2f(1, 1); glVertex3f(dist, -tam,  tam);
    glTexCoord2f(1, 0); glVertex3f(dist,  tam,  tam);
    glTexCoord2f(0, 0); glVertex3f(dist,  tam, -tam);
    glEnd();
}

/* ------------------------------------------------------------------ */
/* Renderização do domo celeste: a função desenhaDomo() constrói uma  */
/* esfera celeste simplificada utilizando faixas e fatias de          */
/* quadriláteros, com interpolação de cores entre o horizonte e o      */
/* zênite. O domo acompanha a posição da câmera, dando a impressão de  */
/* que o céu envolve o jogador.                                        */
/*                                                                    */
/* Também são desenhados o sol e a lua, posicionados conforme o       */
/* ângulo do ciclo. A lua aparece no lado oposto ao sol, e ambos       */
/* desaparecem parcialmente abaixo do horizonte. Para a renderização, */
/* são ajustados temporariamente os estados de profundidade,          */
/* iluminação, neblina, texturas e transparência.                     */
/* ------------------------------------------------------------------ */

void desenhaDomo(void)
{
    const int FAIXAS = 12, FATIAS = 24;
    const float ELEV_MIN = -PI / 8.0f;      /* um pouco abaixo do horizonte */
    float ox, oy, oz;
    int i, j, k;

    if (projecaoOrtogonal) return;          /* o domo não faz sentido na ortográfica */

    posicaoDoOlho(&ox, &oy, &oz);

    glPushMatrix();
    glTranslatef(ox, oy, oz);               /* regra 1: preso à câmera */

    glDepthMask(GL_FALSE);                  /* regra 2: sem profundidade */
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);

    for (i = 0; i < FAIXAS; i++) {          /* uma faixa por anel de altura */
        float e0 = ELEV_MIN + (PI / 2 - ELEV_MIN) * i / FAIXAS;
        float e1 = ELEV_MIN + (PI / 2 - ELEV_MIN) * (i + 1) / FAIXAS;
        float e[2];
        e[0] = e0; e[1] = e1;

        glBegin(GL_QUAD_STRIP);
        for (j = 0; j <= FATIAS; j++) {
            float az = 2.0f * PI * j / FATIAS;
            for (k = 0; k < 2; k++) {
                float u = sinf(e[k]);       /* 0 no horizonte, 1 no zênite */
                Cor c;
                if (u < 0.0f) u = 0.0f;
                u = powf(u, 0.6f);          /* faixa do horizonte mais larga */
                c = lerp(corHorizonte, corZenite, u);
                glColor3f(c.r, c.g, c.b);
                glVertex3f(cosf(e[k]) * cosf(az),
                           sinf(e[k]),
                           cosf(e[k]) * sinf(az));
            }
        }
        glEnd();
    }

    /* --- sol e lua: presos ao céu, não são billboards --- */
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(1.0f, 1.0f, 1.0f);

    if (sinf(anguloSol * PI / 180.0f) > -0.2f) {        /* some abaixo do horizonte */
        glPushMatrix();
        glRotatef(25.0f, 1, 0, 0);                      /* a mesma inclinação da luz */
        glRotatef(anguloSol, 0, 0, 1);                  /* o ângulo que gira a luz */
        astro(texSol, 0.08f, 0.9f);
        glPopMatrix();
    }
    if (sinf(anguloSol * PI / 180.0f) < 0.2f) {
        glPushMatrix();
        glRotatef(25.0f, 1, 0, 0);
        glRotatef(anguloSol + 180.0f, 0, 0, 1);         /* lua: lado oposto ao sol */
        astro(texLua, 0.06f, 0.9f);
        glPopMatrix();
    }

    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, atlas);

    glDepthMask(GL_TRUE);                   /* devolve o estado */
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

/* ================================================================== */
/* SELEÇÃO DE BLOCOS                                                  */
/*                                                                    */
/* Como o jogo sabe qual bloco o jogador quer quebrar: um raio parte  */
/* da câmera na direção do olhar, e a primeira célula sólida da       */
/* matriz ao longo dele, dentro do alcance, é a escolhida. A mira é o */
/* centro da tela.                                                    */
/* ================================================================== */


Alvo alvo;                          /* calculado a cada quadro, em atualiza() */
int  inimigoMirado = -1;

/* ------------------------------------------------------------------ */
/* Percurso em grade (Amanatides e Woo)                               */
/* O raio é origem + t x direção, com a direção de comprimento 1, então */
/* t é a distância em blocos. Em cada eixo o algoritmo guarda:        */
/*   tX   o t em que o raio cruza a PRÓXIMA fronteira de célula em x  */
/*   dX   quanto t cresce para cruzar uma célula inteira em x (1/|dx|) */
/* (idem em y e z). A cada passo, cruza-se a fronteira mais próxima,  */
/* a de menor t, e só aquele eixo avança: soma de dX em tX e uma      */
/* célula a mais em x. O laço usa apenas somas e comparações, e visita */
/* exatamente as células atravessadas, em ordem, cada uma uma vez.    */
/*                                                                    */
/* Devolve 1 se achou um bloco selecionável (blocoSelecionavel) com   */
/* entrada em t entre inicio e fim. resultado recebe a célula         */
/* atingida (a que se quebra) e a célula visitada logo antes dela, vazia */
/* e encostada na face atingida (onde se coloca): sem calcular qual   */
/* face foi atingida. Células fora da matriz são só atravessadas.     */
/* ------------------------------------------------------------------ */

/* A água não é alvo: o raio passa por ela, como no Minecraft */
int blocoSelecionavel(int tipo)
{
    return tipo != VAZIO && tipo != AGUA;
}

int percursoEmGrade(float ox, float oy, float oz, float dx, float dy, float dz,
                    float inicio, float fim, Alvo *resultado)
{
    int x = (int)floorf(ox), y = (int)floorf(oy), z = (int)floorf(oz);
    int passoX = dx > 0.0f ? 1 : -1;
    int passoY = dy > 0.0f ? 1 : -1;
    int passoZ = dz > 0.0f ? 1 : -1;
    float deltaX = dx != 0.0f ? fabsf(1.0f / dx) : INFINITO;
    float deltaY = dy != 0.0f ? fabsf(1.0f / dy) : INFINITO;
    float deltaZ = dz != 0.0f ? fabsf(1.0f / dz) : INFINITO;
    /* t até a primeira fronteira: distância até a face da célula, na
       direção em que o raio anda, dividida pela velocidade nesse eixo */
    float tX = dx > 0.0f ? (x + 1 - ox) / dx : dx < 0.0f ? (ox - x) / -dx : INFINITO;
    float tY = dy > 0.0f ? (y + 1 - oy) / dy : dy < 0.0f ? (oy - y) / -dy : INFINITO;
    float tZ = dz > 0.0f ? (z + 1 - oz) / dz : dz < 0.0f ? (oz - z) / -dz : INFINITO;
    float t;

    resultado->acertou = 0;

    for (;;) {
        int ax = x, ay = y, az = z;         /* a célula de onde estamos saindo */

        if (tX < tY && tX < tZ)  { t = tX; x += passoX; tX += deltaX; }
        else if (tY < tZ)        { t = tY; y += passoY; tY += deltaY; }
        else                     { t = tZ; z += passoZ; tZ += deltaZ; }

        if (t > fim) return 0;              /* passou do alcance */

        if (t >= inicio && dentroDoMundo(x, y, z) &&
            blocoSelecionavel(mundo[x][y][z])) {
            resultado->acertou = 1;
            resultado->x = x;  resultado->y = y;  resultado->z = z;
            resultado->ax = ax; resultado->ay = ay; resultado->az = az;
            resultado->distancia = t;
            return 1;
        }
    }
}

/* ------------------------------------------------------------------ */
/* O raio da mira é o que passa pelo centro da tela:                  */
/*   primeira pessoa  sai dos olhos, na direção do olhar              */
/*   terceira pessoa  sai da câmera, na direção do alvo do gluLookAt  */
/*                    (a cabeça, inclinada pelo pitch), para a mira   */
/*                    apontar para onde se vê. O trecho entre a       */
/*                    câmera e o personagem é ignorado (inicio):      */
/*                    o alcance conta a partir da cabeça.             */
/* ------------------------------------------------------------------ */

void raioDaMira(float *ox, float *oy, float *oz, float *dx, float *dy, float *dz,
                float *inicio, float *fim)
{
    posicaoDoOlho(ox, oy, oz);
    direcaoDoOlhar(dx, dy, dz);
    *inicio = 0.0f;

    if (modoCamera == TERCEIRA_PESSOA) {
        float alvoX = jogadorX;
        float alvoY = jogadorY + ALTURA_OLHOS + *dy * DISTANCIA_CAMERA;
        float alvoZ = jogadorZ;
        float vx = alvoX - *ox, vy = alvoY - *oy, vz = alvoZ - *oz;
        float comprimento = sqrtf(vx * vx + vy * vy + vz * vz);

        *dx = vx / comprimento;
        *dy = vy / comprimento;
        *dz = vz / comprimento;
        *inicio = comprimento;
    }
    *fim = *inicio + ALCANCE;
}

/* ------------------------------------------------------------------ */
/* Seleção do quadro: o bloco sob a mira e o inimigo sob a mira. Se o */
/* inimigo está mais perto que o bloco, é ele o alvo (ver Bloco 5).   */
/* Chamada a cada quadro, depois que a câmera e os inimigos se        */
/* moveram, e depois de cada edição do mundo.                         */
/* ------------------------------------------------------------------ */

void atualizaMira(void)
{
    float ox, oy, oz, dx, dy, dz, inicio, fim;
    float melhor;
    int i;

    raioDaMira(&ox, &oy, &oz, &dx, &dy, &dz, &inicio, &fim);
    percursoEmGrade(ox, oy, oz, dx, dy, dz, inicio, fim, &alvo);

    inimigoMirado = -1;
    melhor = alvo.acertou ? alvo.distancia : INFINITO;

    for (i = 0; i < numInimigos; i++) {
        const Inimigo *e = &inimigos[i];
        float minimo[3] = { e->x - LARGURA_INIMIGO / 2.0f, e->y,
                            e->z - LARGURA_INIMIGO / 2.0f };
        float maximo[3] = { e->x + LARGURA_INIMIGO / 2.0f, e->y + ALTURA_INIMIGO,
                            e->z + LARGURA_INIMIGO / 2.0f };
        float t;

        if (raioContraCaixa(ox, oy, oz, dx, dy, dz, minimo, maximo, &t) &&
            t >= inicio && t <= fim && t < melhor) {
            melhor = t;
            inimigoMirado = i;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Contorno do bloco selecionado: as 12 arestas de um cubo em arame,  */
/* pretas, com textura, iluminação e neblina desligadas. O cubo é     */
/* FOLGA_CONTORNO maior que o bloco em todos os lados: linhas         */
/* exatamente sobre as faces empatariam com elas no buffer de         */
/* profundidade e piscariam. Desenhado depois de toda a cena.         */
/* ------------------------------------------------------------------ */

static const int arestasDoCubo[12][2][3] = {
    { {0,0,0}, {1,0,0} }, { {0,1,0}, {1,1,0} }, { {0,0,1}, {1,0,1} }, { {0,1,1}, {1,1,1} },
    { {0,0,0}, {0,1,0} }, { {1,0,0}, {1,1,0} }, { {0,0,1}, {0,1,1} }, { {1,0,1}, {1,1,1} },
    { {0,0,0}, {0,0,1} }, { {1,0,0}, {1,0,1} }, { {0,1,0}, {0,1,1} }, { {1,1,0}, {1,1,1} },
};

void desenhaContorno(void)
{
    int i;

    if (!alvo.acertou || inimigoMirado >= 0) return;    /* nada a destacar */

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glLineWidth(2.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glPushMatrix();
    glTranslatef(alvo.x - FOLGA_CONTORNO, alvo.y - FOLGA_CONTORNO,
                 alvo.z - FOLGA_CONTORNO);
    glScalef(1.0f + 2.0f * FOLGA_CONTORNO, 1.0f + 2.0f * FOLGA_CONTORNO,
             1.0f + 2.0f * FOLGA_CONTORNO);
    glBegin(GL_LINES);
    for (i = 0; i < 12; i++) {
        glVertex3f((float)arestasDoCubo[i][0][0], (float)arestasDoCubo[i][0][1],
                   (float)arestasDoCubo[i][0][2]);
        glVertex3f((float)arestasDoCubo[i][1][0], (float)arestasDoCubo[i][1][1],
                   (float)arestasDoCubo[i][1][2]);
    }
    glEnd();
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
}


/* ================================================================== */
/* EDIÇÃO DO MUNDO                                                    */
/*                                                                    */
/* Editar o mundo é só alterar a matriz: quebrar grava VAZIO na célula */
/* atingida, e colocar grava o tipo escolhido na célula anterior. O   */
/* desenho, a colisão e a remoção de faces ocultas já consultam a     */
/* matriz a cada quadro (nada é guardado antes), então refletem a     */
/* mudança sem código extra; não há faces calculadas para refazer.    */
/* ================================================================== */


int tipoSelecionado = GRAMA;

/* teclas 1 a 8 */
static const int tiposColocaveis[8] = {
    GRAMA, TERRA, PEDRA, TRONCO, FOLHA, AREIA, NEVE, VIDRO
};

/* Botão esquerdo: a célula atingida vira ar e um item do mesmo tipo
   aparece no meio dela. Cai se não houver apoio embaixo. */
void quebraBloco(void)
{
    int tipo = blocoNoMundo(alvo.x, alvo.y, alvo.z);

    colocaBloco(VAZIO, alvo.x, alvo.y, alvo.z);
    criaItem(tipo, alvo.x + 0.5f, alvo.y + 0.5f - LADO_ITEM / 2.0f, alvo.z + 0.5f);
    atualizaMira();                         /* há outra coisa sob a mira agora */
}

/* Botão direito: o bloco escolhido vai para a célula anterior, se ela
   estiver livre (ar ou água) e nenhuma caixa ficar presa dentro dele.
   O teste é o da colisão: coloca, pergunta se a caixa do personagem
   (e as dos inimigos) ainda cabe, e desfaz se não couber. */
void colocaBlocoMirado(void)
{
    int tipo = tipoSelecionado;
    int anterior, ok, i;

    if (!dentroDoMundo(alvo.ax, alvo.ay, alvo.az)) return;
    anterior = blocoNoMundo(alvo.ax, alvo.ay, alvo.az);
    if (anterior != VAZIO && anterior != AGUA) return;
    if (CONSOME_ITENS && contagem[tipo] <= 0) return;

    colocaBloco(tipo, alvo.ax, alvo.ay, alvo.az);

    ok = posicaoLivre(jogadorX, jogadorY, jogadorZ);
    for (i = 0; i < numInimigos; i++)
        if (!caixaLivre(inimigos[i].x, inimigos[i].y, inimigos[i].z,
                        LARGURA_INIMIGO, ALTURA_INIMIGO))
            ok = 0;
    if (!ok) {                              /* prenderia alguém: desfaz */
        colocaBloco(anterior, alvo.ax, alvo.ay, alvo.az);
        return;
    }

    if (CONSOME_ITENS) {
        contagem[tipo]--;
        tituloSujo = 1;
    }
    atualizaMira();
}

/* Um clique, por evento (glutMouseFunc): o esquerdo ataca o inimigo sob
   a mira, se houver (ele só é alvo se estiver mais perto que o bloco), e
   senão quebra o bloco; o direito coloca. Usa o resultado do quadro. */
void cliqueEdicao(int botao)
{
    if (botao == GLUT_LEFT_BUTTON) {
        if (inimigoMirado >= 0)  atacaInimigo(inimigoMirado);
        else if (alvo.acertou)   quebraBloco();
    } else if (botao == GLUT_RIGHT_BUTTON && alvo.acertou) {
        colocaBlocoMirado();
    }
}


/* ================================================================== */
/* ITENS                                                              */
/*                                                                    */
/* Quebrar um bloco solta um item: um cubo pequeno e giratório, do    */
/* mesmo tipo, guardado no vetor itens[]. Ele cai com a mesma física  */
/* do personagem (sem pulo) e é recolhido quando o personagem chega   */
/* perto: sai do vetor e o contador do tipo aumenta.                  */
/* ================================================================== */


Item itens[MAX_ITENS];
int  numItens = 0;
int  contagem[NUM_TIPOS];

/* x e z são o centro; y, a base do cubo */
void criaItem(int tipo, float x, float y, float z)
{
    Item *it;

    if (numItens >= MAX_ITENS) return;      /* vetor cheio: o item se perde */
    it = &itens[numItens++];
    it->x = x;  it->y = y;  it->z = z;
    it->vy = 0.0f;
    it->idade = 0.0f;
    it->tipo = tipo;
}

/* Física e coleta, a cada atualização. A queda é a do Bloco 4: v recebe
   g x dt, a posição recebe v x dt, e a colisão vertical apoia o item no
   topo do bloco. A remoção troca o item recolhido pelo último do vetor,
   sem deixar buraco; por isso o índice só avança quando nada é removido. */
void atualizaItens(float dt)
{
    int i = 0;

    while (i < numItens) {
        Item *it = &itens[i];
        int b, tentativas = 0;
        float dx, dy, dz;

        it->idade += dt;

        /* um bloco colocado sobre o item o enterraria: sobe até sair */
        while (!caixaLivre(it->x, it->y, it->z, LADO_ITEM, LADO_ITEM) &&
               tentativas++ < ALTURA_MUNDO)
            it->y += 1.0f;

        it->vy -= GRAVIDADE * dt;
        if (it->vy < -VELOCIDADE_QUEDA_MAXIMA) it->vy = -VELOCIDADE_QUEDA_MAXIMA;
        b = moveCaixa(&it->x, &it->y, &it->z, LADO_ITEM, LADO_ITEM,
                      0.0f, it->vy * dt, 0.0f);
        if (b & (BLOQ_BAIXO | BLOQ_CIMA)) it->vy = 0.0f;

        /* distância entre o centro do item e o do personagem */
        dx = it->x - jogadorX;
        dy = (it->y + LADO_ITEM / 2.0f) - (jogadorY + ALTURA_JOGADOR / 2.0f);
        dz = it->z - jogadorZ;

        if (it->idade >= ATRASO_COLETA &&
            dx * dx + dy * dy + dz * dz < RAIO_COLETA * RAIO_COLETA) {
            contagem[it->tipo]++;
            tituloSujo = 1;
            itens[i] = itens[--numItens];   /* sai do vetor; não avança i */
        } else {
            i++;
        }
    }
}

/* Cada item é o cubo de sempre (desenhaBloco), reduzido por glScalef e
   centrado na origem, girando em torno do eixo vertical e subindo e
   descendo de leve. O teste de alfa só descarta o vazado das folhas. */
void desenhaItens(void)
{
    int i;

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    for (i = 0; i < numItens; i++) {
        const Item *it = &itens[i];
        float flutua = 0.1f + 0.05f * sinf(3.0f * it->idade);

        if (it->tipo == PLANTA || it->tipo == MACA) {   /* imagem (semana 8) */
            glPushMatrix();
            glTranslatef(it->x, it->y + flutua, it->z);
            glScalef(0.6f, 0.6f, 0.6f);
            glDisable(GL_CULL_FACE);
            desenhaItem(it->tipo == PLANTA ? CEL_FOLHA : SEM_TEXTURA);
            glEnable(GL_CULL_FACE);
            glPopMatrix();
            continue;
        }

        glPushMatrix();
        glTranslatef(it->x, it->y + LADO_ITEM / 2.0f + flutua, it->z);
        glRotatef(90.0f * it->idade, 0.0f, 1.0f, 0.0f);
        glScalef(LADO_ITEM, LADO_ITEM, LADO_ITEM);
        glTranslatef(-0.5f, -0.5f, -0.5f);  /* o cubo vai de 0 a 1: centra */
        desenhaBloco(it->tipo, 1.0f, TODAS_FACES);
        glPopMatrix();
    }

    glDisable(GL_ALPHA_TEST);
}


/* ================================================================== */
/* INIMIGOS                                                           */
/*                                                                    */
/* Caixas articuladas (tronco, cabeça, braços e pernas), com a mesma  */
/* física do personagem. Perseguem o jogador, ferem por contato,      */
/* recuam quando atingidos e, sem vida, somem deixando um item. Só    */
/* surgem à noite.                                                    */
/* ================================================================== */


Inimigo inimigos[MAX_INIMIGOS];
int     numInimigos = 0;
int     vidaJogador = VIDA_JOGADOR;
float   invulnerabilidade = 0.0f;

/* ------------------------------------------------------------------ */
/* Raio contra caixa alinhada aos eixos, pelo método das placas. Em   */
/* cada eixo, o par de planos da caixa define o trecho do raio entre  */
/* eles, de t1 a t2. O raio acerta a caixa quando os trechos dos três */
/* eixos se sobrepõem: o maior dos começos é menor que o menor dos    */
/* fins. Um eixo paralelo ao raio não tem trecho: ou o raio está      */
/* dentro das duas placas (e o eixo não restringe nada), ou fora (e   */
/* erra). entrada recebe o t em que o raio entra na caixa.            */
/* ------------------------------------------------------------------ */

int raioContraCaixa(float ox, float oy, float oz, float dx, float dy, float dz,
                    const float minimo[3], const float maximo[3], float *entrada)
{
    const float origem[3]  = { ox, oy, oz };
    const float direcao[3] = { dx, dy, dz };
    float maiorInicio = 0.0f, menorFim = INFINITO;
    int eixo;

    for (eixo = 0; eixo < 3; eixo++) {
        float t1, t2;

        if (fabsf(direcao[eixo]) < 1e-8f) {
            if (origem[eixo] < minimo[eixo] || origem[eixo] > maximo[eixo])
                return 0;
            continue;
        }
        t1 = (minimo[eixo] - origem[eixo]) / direcao[eixo];
        t2 = (maximo[eixo] - origem[eixo]) / direcao[eixo];
        if (t1 > t2) { float troca = t1; t1 = t2; t2 = troca; }

        if (t1 > maiorInicio) maiorInicio = t1;
        if (t2 < menorFim)    menorFim = t2;
        if (maiorInicio > menorFim) return 0;       /* os trechos não se tocam */
    }
    *entrada = maiorInicio;
    return 1;
}

/* ------------------------------------------------------------------ */
/* Criação e remoção. A remoção troca o inimigo pelo último do vetor. */
/* ------------------------------------------------------------------ */

int criaInimigo(float x, float y, float z)
{
    Inimigo *e;

    if (numInimigos >= MAX_INIMIGOS) return 0;
    e = &inimigos[numInimigos++];
    e->x = x;  e->y = y;  e->z = z;
    e->vy = 0.0f;  e->kx = 0.0f;  e->kz = 0.0f;
    e->yaw = 0.0f;
    e->fase = 0.0f;  e->amplitude = 0.0f;
    e->ferido = 0.0f;
    e->vida = VIDA_INIMIGO;
    return 1;
}

static void removeInimigo(int i)
{
    inimigos[i] = inimigos[--numInimigos];
}

/* ------------------------------------------------------------------ */
/* Atualização de um inimigo, com o tempo de quadro dt:               */
/*   perseguição  vetor horizontal até o jogador, normalizado, vezes  */
/*                velocidade e dt; para a PARADA_INIMIGO do jogador   */
/*   orientação   yaw = atan2(dx, -dz): o inverso de direcaoDoOlhar,  */
/*                que dá (sen yaw, -cos yaw) para o yaw da câmera     */
/*   física       gravidade e colisão eixo a eixo, como o personagem; */
/*                bloqueado na horizontal e apoiado, ele pula (sobe   */
/*                degraus)                                            */
/*   recuo        velocidade extra horizontal que decai rápido        */
/*   animação     como a do personagem: a fase avança com a distância */
/*                andada e a amplitude cai a zero quando ele para     */
/* ------------------------------------------------------------------ */

void atualizaInimigo(Inimigo *e, float dt)
{
    float dx = jogadorX - e->x, dz = jogadorZ - e->z;
    float distancia = sqrtf(dx * dx + dz * dz);
    float mx = 0.0f, mz = 0.0f;
    float antesX = e->x, antesZ = e->z, andou;
    float vx, vz;
    int b;

    if (distancia > 0.001f)
        e->yaw = atan2f(dx, -dz) * 180.0f / PI;
    if (distancia > PARADA_INIMIGO) {
        mx = dx / distancia;                /* direção unitária no chão */
        mz = dz / distancia;
    }

    vx = mx * VELOCIDADE_INIMIGO + e->kx;
    vz = mz * VELOCIDADE_INIMIGO + e->kz;
    e->kx -= e->kx * fminf(1.0f, 8.0f * dt);    /* o recuo se esgota */
    e->kz -= e->kz * fminf(1.0f, 8.0f * dt);

    e->vy -= GRAVIDADE * dt;
    if (e->vy < -VELOCIDADE_QUEDA_MAXIMA) e->vy = -VELOCIDADE_QUEDA_MAXIMA;

    b = moveCaixa(&e->x, &e->y, &e->z, LARGURA_INIMIGO, ALTURA_INIMIGO,
                  vx * dt, e->vy * dt, vz * dt);
    if (b & (BLOQ_BAIXO | BLOQ_CIMA)) e->vy = 0.0f;
    if ((b & BLOQ_BAIXO) && (b & (BLOQ_X | BLOQ_Z)) && (mx != 0.0f || mz != 0.0f))
        e->vy = VELOCIDADE_PULO;            /* um bloco na frente: pula */

    andou = sqrtf((e->x - antesX) * (e->x - antesX) +
                  (e->z - antesZ) * (e->z - antesZ));
    e->fase += andou * FASE_POR_BLOCO;
    if (andou > 0.0f) e->amplitude += VELOCIDADE_AMPLITUDE * dt;
    else              e->amplitude -= VELOCIDADE_AMPLITUDE * dt;
    if (e->amplitude > 1.0f) e->amplitude = 1.0f;
    if (e->amplitude < 0.0f) e->amplitude = 0.0f;

    if (e->ferido > 0.0f) e->ferido -= dt;
}

/* ------------------------------------------------------------------ */
/* Surgimento: sobre um bloco sólido de chão (nem água, nem folha, nem */
/* vidro, nem nuvem) onde a caixa do inimigo caiba, a pelo menos      */
/* DISTANCIA_SPAWN do jogador, para não aparecer diante dele.         */
/* Sorteia colunas do mapa e olha de cima para baixo.                 */
/* ------------------------------------------------------------------ */

/* y dos pés de um inimigo que surja na coluna (x, z), ou -1 */
static int chaoParaSurgir(int x, int z)
{
    int y;

    for (y = ALTURA_MUNDO - 1; y >= 0; y--) {
        int tipo = mundo[x][y][z];

        if (tipo == VAZIO || tipo == NUVEM) continue;
        if (tipo == AGUA || tipo == FOLHA || tipo == TRONCO || tipo == VIDRO)
            return -1;
        return caixaLivre(x + 0.5f, (float)(y + 1), z + 0.5f,
                          LARGURA_INIMIGO, ALTURA_INIMIGO) ? y + 1 : -1;
    }
    return -1;
}

static float tempoSurgir = INTERVALO_SPAWN;
static float tempoSumir  = 1.0f;

static void surgeEDesaparece(float dt)
{
    int tentativa;

    if (!ehNoite()) {
        if (DESAPARECEM_DE_DIA && numInimigos > 0) {
            tempoSumir -= dt;               /* um por segundo, sem item */
            if (tempoSumir <= 0.0f) {
                removeInimigo(0);
                tempoSumir = 1.0f;
            }
        }
        return;
    }

    tempoSurgir -= dt;
    if (tempoSurgir > 0.0f || numInimigos >= MAX_INIMIGOS) return;
    tempoSurgir = INTERVALO_SPAWN;

    for (tentativa = 0; tentativa < 20; tentativa++) {
        int x = rand() % LADO, z = rand() % LADO;
        float dx = x + 0.5f - jogadorX, dz = z + 0.5f - jogadorZ;
        int y;

        if (dx * dx + dz * dz < DISTANCIA_SPAWN * DISTANCIA_SPAWN) continue;
        y = chaoParaSurgir(x, z);
        if (y < 0) continue;
        criaInimigo(x + 0.5f, (float)y, z + 0.5f);
        return;
    }
}

/* O personagem volta ao início com a vida cheia, e os inimigos somem */
void reapareceJogador(void)
{
    jogadorX = INICIO_X;  jogadorY = INICIO_Y;  jogadorZ = INICIO_Z;
    velocidadeY = 0.0f;
    vidaJogador = VIDA_JOGADOR;
    invulnerabilidade = 2.0f;
    numInimigos = 0;
    tituloSujo = 1;
}

/* ------------------------------------------------------------------ */
/* Todos os inimigos, a cada atualização. O contato é a sobreposição  */
/* da caixa do inimigo com a do personagem (o teste da semana 5, nos  */
/* três eixos). Cada contato tira vida e liga uma invulnerabilidade   */
/* curta, para a vida não cair em todos os quadros seguidos.          */
/* ------------------------------------------------------------------ */

void atualizaInimigos(float dt)
{
    int i;

    surgeEDesaparece(dt);
    if (invulnerabilidade > 0.0f) invulnerabilidade -= dt;

    for (i = 0; i < numInimigos; i++) {
        Inimigo *e = &inimigos[i];

        atualizaInimigo(e, dt);

        if (invulnerabilidade <= 0.0f &&
            fabsf(e->x - jogadorX) < LARGURA_INIMIGO / 2.0f + LARGURA_JOGADOR / 2.0f &&
            fabsf(e->z - jogadorZ) < LARGURA_INIMIGO / 2.0f + LARGURA_JOGADOR / 2.0f &&
            e->y < jogadorY + ALTURA_JOGADOR && jogadorY < e->y + ALTURA_INIMIGO) {
            vidaJogador -= DANO_CONTATO;
            invulnerabilidade = INVULNERAVEL_TEMPO;
            tituloSujo = 1;
            if (vidaJogador <= 0) {
                reapareceJogador();
                return;                     /* o vetor foi esvaziado */
            }
        }
    }
}

/* Golpe do jogador: tira vida e empurra o inimigo para longe do jogador,
   com um saltinho. Sem vida, ele sai do vetor e deixa um item. */
void atacaInimigo(int i)
{
    Inimigo *e = &inimigos[i];
    float dx = e->x - jogadorX, dz = e->z - jogadorZ;
    float distancia = sqrtf(dx * dx + dz * dz);

    if (distancia < 0.001f) { dx = 0.0f; dz = -1.0f; distancia = 1.0f; }

    e->vida -= DANO_ATAQUE;
    e->ferido = 0.3f;
    e->kx = dx / distancia * RECUO_HORIZONTAL;
    e->kz = dz / distancia * RECUO_HORIZONTAL;
    e->vy = RECUO_VERTICAL;

    if (e->vida <= 0) {
        criaItem(ZUMBI_PELE, e->x, e->y, e->z);
        removeInimigo(i);
        atualizaMira();                     /* o índice mirado mudou */
    }
}

/* ------------------------------------------------------------------ */
/* Desenho hierárquico. O inimigo inteiro é levado aos pés e girado   */
/* por -yaw, como o personagem; cada peça é posicionada em relação a  */
/* isso, então mover o tronco move tudo. As pernas e os braços são    */
/* desenhados por membro(): pendurados abaixo da origem, girados e    */
/* só então levados à articulação, para girarem em torno do quadril   */
/* e do ombro, e não do próprio centro (a ordem das transformações    */
/* não comuta: ver membro()).                                         */
/*                                                                    */
/*   y 1.8   cabeça                                                   */
/*   y 1.4   ombro: tronco e braços (estendidos para a frente)        */
/*   y 0.7   quadril: pernas, em fases opostas                        */
/*   y 0     pés                                                      */
/* ------------------------------------------------------------------ */

void desenhaInimigo(const Inimigo *e)
{
    int pele   = e->ferido > 0.0f ? ZUMBI_DANO : ZUMBI_PELE;
    int camisa = e->ferido > 0.0f ? ZUMBI_DANO : ZUMBI_CAMISA;
    int calca  = e->ferido > 0.0f ? ZUMBI_DANO : ZUMBI_CALCA;
    float quadril = COMPRIMENTO_MEMBRO;
    float ombro   = 2.0f * COMPRIMENTO_MEMBRO;
    float angulo  = BALANCO_MAXIMO * e->amplitude * sinf(e->fase);

    glPushMatrix();
    glTranslatef(e->x, e->y, e->z);
    glRotatef(-e->yaw, 0.0f, 1.0f, 0.0f);   /* frente do inimigo = -z local */

    membro(-0.1f, quadril,  angulo, 0.18f, calca, calca, 0.1f);
    membro( 0.1f, quadril, -angulo, 0.18f, calca, calca, 0.1f);

    caixa(camisa, -0.2f, quadril, -0.1f, 0.4f, COMPRIMENTO_MEMBRO, 0.2f);

    /* braços para a frente (90 graus), balançando um pouco */
    membro(-0.26f, ombro, 90.0f + 0.3f * angulo, 0.12f, camisa, pele, 0.3f);
    membro( 0.26f, ombro, 90.0f - 0.3f * angulo, 0.12f, camisa, pele, 0.3f);

    caixa(pele, -0.2f, ombro, -0.2f, 0.4f, 0.4f, 0.4f);
    caixa(OLHO, -0.13f, ombro + 0.2f, -0.205f, 0.1f, 0.06f, 0.01f);
    caixa(OLHO,  0.03f, ombro + 0.2f, -0.205f, 0.1f, 0.06f, 0.01f);

    glPopMatrix();
}


/* ================================================================== */
/* DIA E NOITE                                                        */
/*                                                                    */
/* O relógio é o ângulo do sol da semana 8 (anguloSol, atualizado em  */
/* atualiza()). A altura do sol é sen(ângulo): positiva de dia,       */
/* negativa à noite. Os inimigos usam isto para saber quando surgir.  */
/* ================================================================== */


int ehNoite(void)
{
    return sinf(anguloSol * PI / 180.0f) < -0.15f;
}


/* ================================================================== */
/* ENTRADA                                                            */
/*                                                                    */
/* Teclado (por evento e por estado), mouse (giro do olhar e captura do */
/* cursor), roda e teclas + e - (zoom), e a atualização do movimento a */
/* cada quadro, escalada pelo tempo de quadro.                        */
/* ================================================================== */


/* ------------------------------------------------------------------ */
/* Teclado. A GLUT só avisa eventos (pressionar e soltar); o estado é */
/* construído aqui: teclas[] fica em 1 do pressionar ao soltar e é    */
/* lido a cada quadro em atualiza(). Por isso a repetição automática  */
/* do sistema está desligada (glutIgnoreKeyRepeat em main).           */
/*                                                                    */
/* As ações discretas (menu, cursor, projeção, câmera, zoom)          */
/* continuam por evento, uma vez por toque. tolower nos dois lados    */
/* evita tecla presa quando o Shift muda entre pressionar e soltar.   */
/* As setas usam o mesmo esquema em especiais[].                      */
/* ------------------------------------------------------------------ */

int teclas[256];        /* teclas de caractere: 1 pressionada, 0 solta */
int especiais[256];     /* teclas especiais (setas, F5): idem */
void teclado(unsigned char tecla, int x, int y)
{
    (void)x; (void)y;                       /* posição do mouse: não usada */

    teclas[tolower(tecla)] = 1;             /* estado: para as ações contínuas */

    if (menuAberto) {                       /* com o menu, só duas teclas valem */
        if (tecla == 27) fechaMenu();       /* 27 é o código da tecla ESC */
        else if (tolower(tecla) == 'q') exit(0);
        return;
    }

    switch (tolower(tecla)) {               /* evento: para as ações discretas */
        case 27:  abreMenu(); break;        /* ESC: menu de pausa */
        case 9:   alternaCaptura(); break;  /* 9 é o código da tecla Tab */
        case 'p':
            projecaoOrtogonal = !projecaoOrtogonal;
            aplicaProjecao();
            break;
        case 'i':
            modoDemonstracao = !modoDemonstracao;
            break;
        case 'o':
            modoLuzPontual = !modoLuzPontual;
            break;
        case 'e':
            emissaoDemo = !emissaoDemo;
            break;
        case 'f':
            modoSombreamento = GL_FLAT;
            glShadeModel(modoSombreamento);
            break;
        case 'g':
            modoSombreamento = GL_SMOOTH;
            glShadeModel(modoSombreamento);
            break;
        case 'v':
            modoCamera = (modoCamera == PRIMEIRA_PESSOA) ? TERCEIRA_PESSOA
                                                         : PRIMEIRA_PESSOA;
            break;
        case '+': case '=': mudaZoom(+1); break;    /* '=' é o + sem Shift */
        case '-': case '_': mudaZoom(-1); break;
        case 't':                           /* ciclo de dia e noite rápido */
            duracaoCiclo = (duracaoCiclo > 60.0f) ? 20.0f : 180.0f;
            break;
        case 'n':                           /* pula para a noite ou para o dia */
            anguloSol = ehNoite() ? 18.0f : 216.0f;
            break;
        case '1': case '2': case '3': case '4':     /* escolhe o tipo colocado */
        case '5': case '6': case '7': case '8':
            if (modoDemonstracao && tecla <= '3') {  /* na demonstração: luz */
                modoComponentesLuz = tecla - '0';
                atualizaComponentesLuz();
                break;
            }
            tipoSelecionado = tiposColocaveis[tecla - '1'];
            tituloSujo = 1;
            break;
        case 'x':                           /* atlas / texturas procedurais */
            usaTexturasProcedurais = !usaTexturasProcedurais;
            break;
    }
}

void tecladoSolto(unsigned char tecla, int x, int y)
{
    (void)x; (void)y;
    teclas[tolower(tecla)] = 0;
}

void especial(int tecla, int x, int y)
{
    (void)x; (void)y;

    if (tecla >= 0 && tecla < 256) especiais[tecla] = 1;

    if (tecla == GLUT_KEY_F5 && !menuAberto)               /* evento: alterna a câmera */
        modoCamera = (modoCamera == PRIMEIRA_PESSOA) ? TERCEIRA_PESSOA
                                                     : PRIMEIRA_PESSOA;
}

void especialSolto(int tecla, int x, int y)
{
    (void)x; (void)y;
    if (tecla >= 0 && tecla < 256) especiais[tecla] = 0;
}

/* ------------------------------------------------------------------ */
/* Mouse: o olhar gira pelo deslocamento em relação ao centro da      */
/* janela. Em coordenadas de tela y cresce para baixo, por isso -dy   */
/* no pitch. Com o cursor capturado, ele volta ao centro a cada       */
/* movimento (glutWarpPointer); o evento desse reposicionamento chega */
/* com deslocamento zero e é ignorado. Registrada com e sem botão     */
/* pressionado.                                                       */
/*                                                                    */
/* No macOS o cursor não é reposicionado: depois de cada              */
/* glutWarpPointer o sistema ignora o mouse por cerca de 0,25 s, e a  */
/* câmera andaria aos trancos. Lá, capturaCursor() desacopla o cursor */
/* do mouse (CGAssociateMouseAndMouseCursorPosition): ele fica parado */
/* e oculto, e o deslocamento de cada movimento é lido direto do      */
/* sistema (CGGetLastMouseDelta).                                     */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/* O pitch fica entre -LIMITE_PITCH e LIMITE_PITCH (89 graus). Em 90, */
/* a direção do olhar fica paralela ao "cima" do gluLookAt, o produto */
/* vetorial que monta os eixos da câmera se anula e a imagem inverte. */
/* ------------------------------------------------------------------ */

int cursorCapturado = 0;    /* 1 enquanto o mouse controla o olhar */
int recentrar = 0;          /* 1: o próximo evento só leva o cursor ao centro */

void movimentoMouse(int x, int y)
{
    int cx = larguraJanela / 2, cy = alturaJanela / 2;
    int dx = x - cx, dy = y - cy;

    if (!cursorCapturado) return;

    if (recentrar) {                        /* cursor fora do centro: só */
        recentrar = 0;                      /* reposiciona, sem girar    */
#ifndef __APPLE__
        glutWarpPointer(cx, cy);
#endif
        return;
    }

#ifdef __APPLE__
    {
        int32_t mx, my;                     /* deslocamento deste movimento */
        CGGetLastMouseDelta(&mx, &my);
        dx = mx;
        dy = my;
    }
#endif

    if (dx == 0 && dy == 0) return;         /* eco do glutWarpPointer */

    yaw   += dx * SENSIBILIDADE;
    pitch -= dy * SENSIBILIDADE;            /* y de tela cresce para baixo */

    if (pitch >  LIMITE_PITCH) pitch =  LIMITE_PITCH;
    if (pitch < -LIMITE_PITCH) pitch = -LIMITE_PITCH;

#ifndef __APPLE__
    glutWarpPointer(cx, cy);                /* devolve o cursor ao centro */
#endif
}

/* Captura e liberação do cursor. O jogo começa com o cursor capturado;
   Tab alterna, e um clique na janela captura de novo. Sem uma forma de
   liberar, o usuário não conseguiria sair da janela.
   capturaCursor() não move o cursor na hora: só o oculta e pede que o
   próximo movimento o leve ao centro. Assim funciona também no início,
   quando a janela talvez ainda nem esteja na tela, e o cursor já está
   oculto quando é reposicionado (em algumas sessões Wayland o
   reposicionamento só funciona com o cursor oculto). */
void capturaCursor(void)
{
    cursorCapturado = 1;
    recentrar = 1;
    glutSetCursor(GLUT_CURSOR_NONE);
#ifdef __APPLE__
    CGAssociateMouseAndMouseCursorPosition(false);  /* cursor parado */
#endif
}

void liberaCursor(void)
{
    cursorCapturado = 0;
    glutSetCursor(GLUT_CURSOR_INHERIT);
#ifdef __APPLE__
    CGAssociateMouseAndMouseCursorPosition(true);   /* cursor volta a andar */
#endif
}

void alternaCaptura(void)
{
    if (cursorCapturado) liberaCursor();
    else                 capturaCursor();
}

/* Clique, por evento: é uma ação discreta, uma vez por toque (semana 5).
   Com o cursor liberado, o clique só captura de novo; com ele capturado,
   quebra, ataca ou coloca (ver cliqueEdicao). */
void cliqueMouse(int botao, int estado, int x, int y)
{
    (void)x; (void)y;
    if (estado != GLUT_DOWN || menuAberto) return;

    if (!cursorCapturado) capturaCursor();
    else                  cliqueEdicao(botao);
}

/* O cursor entrou na janela (por exemplo, depois de trocar de janela
   com Alt+Tab): ele chega pela borda, então o próximo movimento só o
   leva ao centro, sem girar o olhar */
void entradaJanela(int estado)
{
    if (estado == GLUT_ENTERED) recentrar = 1;
}

/* ------------------------------------------------------------------ */
/* Zoom: a roda e as teclas + e - mudam o campo de visão (trocam a    */
/* lente), sem mover a câmera. Passos de 5 graus, entre FOV_MIN e     */
/* FOV_MAX. glutMouseWheelFunc é extensão da freeglut, por isso       */
/* GL/freeglut.h.                                                     */
/* ------------------------------------------------------------------ */

/* +1 aproxima, -1 afasta; usada pela roda e pelas teclas + e - */
void mudaZoom(int direcao)
{
    campoDeVisao -= direcao * 5.0f;         /* fov menor = mais perto */
    if (campoDeVisao < FOV_MIN) campoDeVisao = FOV_MIN;
    if (campoDeVisao > FOV_MAX) campoDeVisao = FOV_MAX;

    aplicaProjecao();
}

void rodaMouse(int roda, int direcao, int x, int y)
{
    (void)roda; (void)x; (void)y;
    if (!menuAberto)
        mudaZoom(direcao);                  /* roda para cima aproxima */
}

/* ------------------------------------------------------------------ */
/* Atualização, a cada quadro (glutIdleFunc). dt é o tempo desde o    */
/* quadro anterior, limitado a DT_MAXIMO; posição += velocidade x dt  */
/* deixa a velocidade igual em qualquer taxa de quadros.              */
/*                                                                    */
/* A frente e a direita saem só do yaw, então o avanço fica no plano  */
/* do chão. A direção das teclas é normalizada (a diagonal não fica   */
/* mais rápida), R multiplica o passo e moveJogador() aplica a        */
/* colisão. No fim, a animação de caminhada avança com a distância    */
/* realmente andada.                                                  */
/* ------------------------------------------------------------------ */

int ultimoTempo = 0;    /* tempo da atualização anterior, em ms */

float faseCaminhada = 0.0f;      /* radianos; avança com a distância andada */
float amplitudeCaminhada = 0.0f; /* 0 parado, 1 andando; suaviza o balanço */

void atualiza(void)
{
    int agora = glutGet(GLUT_ELAPSED_TIME);
    float dt = (agora - ultimoTempo) / 1000.0f;
    ultimoTempo = agora;
    if (dt > DT_MAXIMO) dt = DT_MAXIMO;

    if (menuAberto) {                       /* pausa: o relógio continua sendo */
        glutPostRedisplay();                /* lido, para o primeiro dt depois */
        return;                             /* de fechar não ser enorme */
    }

    if (modoDemonstracao) {
        anguloDemonstracao += 35.0f * dt;
        if (anguloDemonstracao >= 360.0f) anguloDemonstracao -= 360.0f;
        glutPostRedisplay();
        return;
    }

    anguloSol += (360.0f / duracaoCiclo) * dt;
    if (anguloSol >= 360.0f) anguloSol -= 360.0f;

    float passo = VELOCIDADE * dt;
    float giro  = VELOCIDADE_GIRO * dt;
    if (teclas['r']) passo *= FATOR_CORRIDA;    /* correr */

    if (especiais[GLUT_KEY_LEFT])  yaw -= giro;
    if (especiais[GLUT_KEY_RIGHT]) yaw += giro;

    float rad = yaw * PI / 180.0f;
    float frenteX  =  sinf(rad), frenteZ  = -cosf(rad);
    float direitaX =  cosf(rad), direitaZ =  sinf(rad);

    /* direção desejada no chão: cada tecla vale 1 (pressionada) ou 0 */
    float frente = (float)(teclas['w'] - teclas['s']);
    float lado   = (float)(teclas['d'] - teclas['a']);
    float mx = frenteX * frente + direitaX * lado;
    float mz = frenteZ * frente + direitaZ * lado;

    float comprimento = sqrtf(mx * mx + mz * mz);
    if (comprimento > 0.0f) {               /* normaliza: na diagonal o */
        mx /= comprimento;                  /* comprimento seria 1.41   */
        mz /= comprimento;
    }

    /* Física (semana 9): a altura não é mais controlada por teclas. O
       espaço, lido por estado, só dá a velocidade inicial do pulo, e só
       com o personagem apoiado. A cada atualização, a velocidade recebe
       a gravidade vezes dt e a posição recebe a velocidade vezes dt (uma
       integração numérica). A velocidade de queda é limitada. */
    if (teclas[' '] && jogadorNoChao)
        velocidadeY = VELOCIDADE_PULO;
    velocidadeY -= GRAVIDADE * dt;
    if (velocidadeY < -VELOCIDADE_QUEDA_MAXIMA)
        velocidadeY = -VELOCIDADE_QUEDA_MAXIMA;

    float antesX = jogadorX, antesZ = jogadorZ;
    moveJogador(mx * passo, velocidadeY * dt, mz * passo);

    /* animação: quanto o personagem realmente andou no chão (a colisão
       pode ter recusado parte do movimento) */
    float andou = sqrtf((jogadorX - antesX) * (jogadorX - antesX) +
                        (jogadorZ - antesZ) * (jogadorZ - antesZ));
    faseCaminhada += andou * FASE_POR_BLOCO;
    if (andou > 0.0f) amplitudeCaminhada += VELOCIDADE_AMPLITUDE * dt;
    else              amplitudeCaminhada -= VELOCIDADE_AMPLITUDE * dt;
    if (amplitudeCaminhada > 1.0f) amplitudeCaminhada = 1.0f;
    if (amplitudeCaminhada < 0.0f) amplitudeCaminhada = 0.0f;

    /* o resto do mundo, depois do movimento do personagem; a mira vem por
       último, com a câmera, os inimigos e os blocos já atualizados */
    atualizaInimigos(dt);
    atualizaItens(dt);
    atualizaMira();

    glutPostRedisplay();
}


/* ================================================================== */
/* MENU                                                               */
/*                                                                    */
/* Menu de pausa, como o do Minecraft: ESC abre e fecha, o jogo para de */
/* se mover, o cursor fica livre e a tela mostra as instruções de     */
/* movimentação por cima da cena escurecida. Com o menu aberto, Q sai do */
/* jogo.                                                              */
/* ================================================================== */


int menuAberto = 0;

/* Abrir pausa o jogo (atualiza() deixa de mover o personagem) e libera
   o cursor, para o usuário poder usar o mouse fora do jogo. Fechar
   captura o cursor de novo. */
void abreMenu(void)
{
    menuAberto = 1;
    liberaCursor();
}

void fechaMenu(void)
{
    menuAberto = 0;
    capturaCursor();
}

void alternaMenu(void)
{
    if (menuAberto) fechaMenu();
    else            abreMenu();
}

/* ------------------------------------------------------------------ */
/* Texto                                                              */
/* As fontes da GLUT (glutBitmapCharacter) usam a codificação         */
/* Latin-1: um byte por letra, e as letras acentuadas do português    */
/* têm códigos de 192 a 255. Este arquivo, como o resto do código,    */
/* está em UTF-8, em que essas mesmas letras ocupam DOIS bytes (por   */
/* exemplo, "ç" é 0xC3 0xA7). Enviados direto para a GLUT, os dois    */
/* bytes virariam dois símbolos estranhos.                            */
/* proximoCaractere() lê um caractere UTF-8 e devolve o código        */
/* Latin-1 dele. Nos dois bytes 110xxxxx 10yyyyyy, os bits xxxxx e    */
/* yyyyyy juntos formam o código: (b1 & 0x1F) << 6 | (b2 & 0x3F).     */
/* Caracteres fora do Latin-1 (como as setas) viram '?', por isso o   */
/* menu escreve "seta para cima" em vez do símbolo.                   */
/* ------------------------------------------------------------------ */

#define FONTE       GLUT_BITMAP_HELVETICA_18
#define FONTE_TITULO GLUT_BITMAP_TIMES_ROMAN_24

static int proximoCaractere(const char **texto)
{
    unsigned char b1 = (unsigned char)**texto;
    (*texto)++;

    if (b1 < 0x80) return b1;                       /* ASCII: 1 byte */

    if ((b1 & 0xE0) == 0xC0 && **texto) {           /* 2 bytes */
        unsigned char b2 = (unsigned char)**texto;
        int codigo = ((b1 & 0x1F) << 6) | (b2 & 0x3F);
        (*texto)++;
        return codigo <= 0xFF ? codigo : '?';
    }

    while ((**texto & 0xC0) == 0x80) (*texto)++;    /* pula o resto */
    return '?';
}

/* largura do texto na tela, em pixels */
static int larguraTexto(void *fonte, const char *texto)
{
    int largura = 0;
    while (*texto)
        largura += glutBitmapWidth(fonte, proximoCaractere(&texto));
    return largura;
}

/* escreve com o início da linha de base em (x, y), em pixels */
static void escreve(void *fonte, float x, float y, const char *texto)
{
    glRasterPos2f(x, y);
    while (*texto)
        glutBitmapCharacter(fonte, proximoCaractere(&texto));
}

/* um retângulo preenchido, em pixels */
static void retangulo(float x0, float y0, float x1, float y1)
{
    glBegin(GL_QUADS);
    glVertex2f(x0, y0); glVertex2f(x1, y0);
    glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}

/* ------------------------------------------------------------------ */
/* Menu: desenhado por cima da cena numa segunda passada, em 2D.      */
/* Projeção ortográfica em pixels (gluOrtho2D), modelo-visualização   */
/* identidade e teste de profundidade desligado, para o painel não    */
/* ficar atrás dos blocos. A cena é escurecida com transparência      */
/* (alfa 0.5). As matrizes 3D são guardadas e devolvidas no fim. A    */
/* cor do texto vale a partir do glRasterPos, por isso glColor vem    */
/* antes.                                                             */
/* ------------------------------------------------------------------ */

static const char *instrucoes[][2] = {
    { "W / S",                    "andar para frente / para trás" },
    { "A / D",                    "andar para a esquerda / para a direita" },
    { "R (segurando)",            "correr" },
    { "Espaço",                   "pular" },
    { "Setas esquerda / direita", "girar o olhar" },
    { "Mouse",                    "olhar ao redor" },
    { "Botão esquerdo",           "quebrar bloco / atacar" },
    { "Botão direito",            "colocar bloco" },
    { "1 a 8",                    "escolher o bloco a colocar" },
    { "N",                        "alternar dia / noite" },
    { "I",                        "demonstração de iluminação" },
    { "1 / 2 / 3",                "componentes da luz" },
    { "F / G",                    "sombreamento plano / Gouraud" },
    { "O",                        "luz pontual / direcional" },
    { "E",                        "alternar emissao luminosa do cubo" },
    { "Agua e fogo",               "efeitos animados por shaders GLSL" },
};

#define NUM_INSTRUCOES (int)(sizeof(instrucoes) / sizeof(instrucoes[0]))
#define ALTURA_LINHA 30
#define MARGEM       40
#define ESPACO_COLUNAS 40

void desenhaMenu(void)
{
    const char *titulo = "Controles";
    const char *rodape = "ESC volta ao jogo     Q sai do jogo";
    int larguraChaves = 0, larguraAcoes = 0, i;

    /* 1. configuração 2D, guardando a 3D */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, larguraJanela, 0, alturaJanela);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);                 /* painel e texto não recebem luz */
    glDisable(GL_TEXTURE_2D);               /* painel e texto em cor sólida */
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* 2. escurece a cena */
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    retangulo(0, 0, larguraJanela, alturaJanela);

    /* 3. tamanho e posição do painel, a partir do texto */
    for (i = 0; i < NUM_INSTRUCOES; i++) {
        int lc = larguraTexto(FONTE, instrucoes[i][0]);
        int la = larguraTexto(FONTE, instrucoes[i][1]);
        if (lc > larguraChaves) larguraChaves = lc;
        if (la > larguraAcoes)  larguraAcoes  = la;
    }

    float larguraPainel = larguraChaves + ESPACO_COLUNAS + larguraAcoes + 2 * MARGEM;
    float alturaPainel  = (NUM_INSTRUCOES + 4) * ALTURA_LINHA + 2 * MARGEM;
    float esquerda = (larguraJanela - larguraPainel) / 2.0f;
    float topo     = (alturaJanela + alturaPainel) / 2.0f;

    glColor4f(0.08f, 0.08f, 0.12f, 0.85f);
    retangulo(esquerda, topo - alturaPainel, esquerda + larguraPainel, topo);

    glDisable(GL_BLEND);

    /* 4. texto: a cor vem antes do glRasterPos (feito em escreve) */
    float y = topo - MARGEM - 20;
    glColor3f(1.0f, 1.0f, 1.0f);
    escreve(FONTE_TITULO,
            (larguraJanela - larguraTexto(FONTE_TITULO, titulo)) / 2.0f, y, titulo);

    y -= 2 * ALTURA_LINHA;
    for (i = 0; i < NUM_INSTRUCOES; i++) {
        glColor3f(1.0f, 0.85f, 0.35f);                  /* tecla: amarelo */
        escreve(FONTE, esquerda + MARGEM, y, instrucoes[i][0]);
        glColor3f(0.92f, 0.92f, 0.92f);                 /* ação: branco */
        escreve(FONTE, esquerda + MARGEM + larguraChaves + ESPACO_COLUNAS, y,
                instrucoes[i][1]);
        y -= ALTURA_LINHA;
    }

    y -= ALTURA_LINHA;
    glColor3f(0.65f, 0.65f, 0.70f);
    escreve(FONTE, (larguraJanela - larguraTexto(FONTE, rodape)) / 2.0f, y, rodape);

    /* 5. devolve a configuração 3D */
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_FOG);
    glPopMatrix();                                      /* modelo-visualização */
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


/* ================================================================== */
/* MIRA E TÍTULO                                                      */
/*                                                                    */
/* Provisórios, até a interface da semana 10: uma cruz no centro da   */
/* tela e, no título da janela, FPS, vida, bloco escolhido, itens     */
/* recolhidos e dia ou noite.                                         */
/* ================================================================== */


/* Mira: o mesmo esquema 2D do menu (projeção em pixels, sem
   profundidade), uma cruz branca com borda preta para aparecer em
   qualquer fundo */
void desenhaMira(void)
{
    float cx = larguraJanela / 2.0f, cy = alturaJanela / 2.0f;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, larguraJanela, 0, alturaJanela);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);

    glColor3f(0.0f, 0.0f, 0.0f);
    retangulo(cx - 11, cy - 3, cx + 11, cy + 3);
    retangulo(cx - 3, cy - 11, cx + 3, cy + 11);
    glColor3f(1.0f, 1.0f, 1.0f);
    retangulo(cx - 9, cy - 1, cx + 9, cy + 1);
    retangulo(cx - 1, cy - 9, cx + 1, cy + 9);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

const char *nomeDoTipo(int tipo)
{
    switch (tipo) {
        case GRAMA:  return "Grama";
        case TERRA:  return "Terra";
        case PEDRA:  return "Pedra";
        case TRONCO: return "Tronco";
        case FOLHA:  return "Folha";
        case AREIA:  return "Areia";
        case NEVE:   return "Neve";
        case VIDRO:  return "Vidro";
        case NUVEM:  return "Nuvem";
        case ZUMBI_PELE: return "Carne";
        case PLANTA: return "Muda";
        case MACA:   return "Maca";
        default:     return "?";
    }
}

int tituloSujo = 1;

/* O título só é refeito a cada segundo (junto com o FPS) ou quando algo
   que ele mostra muda (tituloSujo): ASCII puro, porque a janela do
   sistema não usa a codificação do código-fonte */
void atualizaTitulo(int fps)
{
    char titulo[256];
    int n, tipo, algum = 0;

    n = snprintf(titulo, sizeof titulo,
                 "Minecraft | %d FPS | Vida %d | Bloco: %s | %s | Itens:",
                 fps, vidaJogador, nomeDoTipo(tipoSelecionado),
                 ehNoite() ? "noite" : "dia");

    for (tipo = 0; tipo < NUM_TIPOS; tipo++)
        if (contagem[tipo] > 0 && n < (int)sizeof titulo - 24) {
            n += snprintf(titulo + n, sizeof titulo - n, " %s %d",
                          nomeDoTipo(tipo), contagem[tipo]);
            algum = 1;
        }
    if (!algum)
        snprintf(titulo + n, sizeof titulo - n, " nenhum");

    glutSetWindowTitle(titulo);
}


/* ================================================================== */
/* QUADRO E PROGRAMA PRINCIPAL                                        */
/* ================================================================== */


/* ------------------------------------------------------------------ */
/* Quadro: a GLUT chama esta função sempre que a janela precisa ser   */
/* redesenhada. A cada chamada:                                       */
/*   1. apaga a imagem anterior (cor e profundidade)                  */
/*   2. posiciona a câmera (primeira ou terceira pessoa)              */
/*   3. desenha a cena e, em terceira pessoa, o personagem            */
/*   4. mostra o resultado na tela                                    */
/*   5. atualiza a contagem de FPS                                    */
/* ------------------------------------------------------------------ */

int inicioContagem = 0;   /* início da contagem de FPS (semana 1), em ms */
int quadros = 0;          /* quadros desenhados desde inicioContagem */
int fps = 0;              /* quadros no último segundo, para o título */

void desenha(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (modoDemonstracao) {
        /* A demonstração usa uma câmera própria, sem alterar a câmera do jogo. */
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluPerspective(50.0, (double)larguraJanela / (double)alturaJanela,
                       0.1, 100.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0.0, 2.8, 8.0, 0.0, 1.2, 0.0, 0.0, 1.0, 0.0);
        {
            const GLfloat posicaoDemo[] = { -3.0f, 6.0f, 5.0f,
                                             modoLuzPontual ? 1.0f : 0.0f };
            const GLfloat difusaDemo[] = { 0.90f, 0.90f, 0.90f, 1.0f };
            glLightfv(GL_LIGHT0, GL_POSITION, posicaoDemo);
            if (modoComponentesLuz >= 2)
                glLightfv(GL_LIGHT0, GL_DIFFUSE, difusaDemo);
        }
        glDisable(GL_FOG);                  /* a neblina é só do mundo */
        desenhaDemonstracao();
        glEnable(GL_FOG);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    } else {
        posicionaCamera();
        atualizaCeuELuz();                  /* luz e cores, depois do gluLookAt */
        desenhaDomo();                      /* primeiro, sem profundidade */

        /* opacos antes dos semitransparentes: a mistura precisa do que
           está atrás já no buffer, e isso inclui o personagem */
        desenhaOpacos();
        desenhaItens();
        for (int i = 0; i < numInimigos; i++)
            desenhaInimigo(&inimigos[i]);
        if (modoCamera == TERCEIRA_PESSOA)
            desenhaJogador();
        desenhaTransparentes();
        desenhaAgua();
        desenhaFogo();
        desenhaContorno();                  /* depois da cena, por cima */
        desenhaMira();
    }

    if (menuAberto)
        desenhaMenu();

    glutSwapBuffers();

    /* conta os quadros e, a cada 1000 ms, guarda o total para o título */
    quadros++;
    int agora = glutGet(GLUT_ELAPSED_TIME);
    if (agora - inicioContagem >= 1000) {
        fps = quadros;
        quadros = 0;
        inicioContagem = agora;
        tituloSujo = 1;
    }
    if (tituloSujo) {
        atualizaTitulo(fps);
        tituloSujo = 0;
    }
}

/* ------------------------------------------------------------------ */
/* Programa principal                                                 */
/* Cria a janela, registra as funções que a GLUT vai chamar em cada   */
/* situação (callbacks), liga os testes do OpenGL, constrói o mundo e */
/* entra no laço principal, de onde não sai mais.                     */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    /* janela com buffer duplo (GLUT_DOUBLE), cores RGB e buffer de
       profundidade (GLUT_DEPTH) */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(larguraJanela, alturaJanela);
    glutCreateWindow("Minecraft");

#ifndef __APPLE__
    /* a GLEW só pode ser iniciada depois que a janela existe */
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        printf("Erro GLEW: %s\n", glewGetErrorString(err));
        return 1;
    }
#endif

    printf("OpenGL %s | %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));

    carregaAtlas();                         /* lê o atlas e liga a textura */
    texSol = carregaAstro("sol.ppm");
    texLua = carregaAstro("lua.ppm");
    inicializaTexturas();                   /* texturas procedurais em memória */
    inicializaIluminacao();                 /* pipeline fixo do OpenGL 1.0 */
    inicializaShaders();                    /* água e fogo animados em GLSL */

    /* as teclas são lidas por estado: a repetição automática do sistema
       operacional não acrescenta nada e é desligada */
    glutIgnoreKeyRepeat(1);

    /* callbacks: a GLUT chama cada função quando o evento acontece */
    glutReshapeFunc(redimensiona);          /* janela abriu ou mudou de tamanho */
    glutDisplayFunc(desenha);               /* hora de desenhar */
    glutKeyboardFunc(teclado);              /* tecla comum pressionada */
    glutKeyboardUpFunc(tecladoSolto);       /* tecla comum solta */
    glutSpecialFunc(especial);              /* tecla especial pressionada */
    glutSpecialUpFunc(especialSolto);       /* tecla especial solta */
    glutPassiveMotionFunc(movimentoMouse);  /* mouse movido sem botão */
    glutMotionFunc(movimentoMouse);         /* mouse movido com botão */
#ifndef __APPLE__
    glutMouseWheelFunc(rodaMouse);          /* roda do mouse (freeglut) */
#endif
    glutMouseFunc(cliqueMouse);             /* clique: captura o cursor */
    glutEntryFunc(entradaJanela);           /* cursor entrou na janela */

    glutIdleFunc(atualiza);                 /* nada acontecendo: atualiza */

    glEnable(GL_DEPTH_TEST);                /* o mais próximo cobre o mais distante,
                                               não importa a ordem de desenho */
    glEnable(GL_CULL_FACE);                 /* não desenha faces de costas */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);

    GLfloat corNeblina[4] = { 0.53f, 0.81f, 0.92f, 1.0f };  /* azul do horizonte de dia */

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 8.0f);
    glFogf(GL_FOG_END, 26.0f);
    glFogfv(GL_FOG_COLOR, corNeblina);
    
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);   /* cor de fundo: azul do céu */

    srand((unsigned)time(NULL));            /* onde os inimigos surgem */
    construirMundo();                       /* preenche a matriz, uma vez só */
    capturaCursor();                        /* o mouse já começa controlando o olhar */

    ultimoTempo = glutGet(GLUT_ELAPSED_TIME);
    inicioContagem = ultimoTempo;

    glutMainLoop();                         /* laço principal: só sai com Q no menu */
    return 0;
}