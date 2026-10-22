/*
 * Projeto Minecraft
 *
 * Reúne o que foi visto nas semanas 1 a 5:
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
 *
 * Cenário: terreno 16 x 16 com uma montanha, uma praia e o mar,
 * uma árvore e duas nuvens. Tudo é feito com um único cubo de lado 1,
 * desenhado várias vezes em posições diferentes.
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
 *   Espaço ou ^  subir
 *   Shift ou v   descer
 *   Mouse        direção do olhar (já começa capturado)
 *   Roda ou + -  aproximar / afastar (campo de visão)
 *   Tab          liberar / capturar o cursor; clique também captura
 *   F5 ou V      alternar terceira (início) / primeira pessoa
 *   P            alternar perspectiva / ortográfica
 *   ESC          abrir / fechar o menu (pausa, instruções)
 *   Q            sair (com o menu aberto)
 *
 * Organização do arquivo, de cima para baixo:
 *   constantes, declarações antecipadas, mundo, colisão, câmera,
 *   entrada, menu e, no fim, desenho do quadro e main()
 *
 * Compilar e rodar (Linux):
 *   gcc mine.c -o mine -lGLEW -lGL -lGLU -lglut -lm && ./mine
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>            /* carrega as funções do OpenGL */
#include <GL/freeglut.h>        /* janela, teclado, mouse e laço principal;
                                   a versão freeglut traz a roda do mouse */
#include <GL/glu.h>             /* gluPerspective e gluLookAt */

#include <stdio.h>              /* printf, sprintf */
#include <stdlib.h>             /* exit */
#include <math.h>               /* sinf, cosf, floorf, sqrtf, ceilf */
#include <ctype.h>              /* tolower */

#define PI 3.14159265f

/* ------------------------------------------------------------------ */
/* Tipos de bloco                                                     */
/* Cada tipo é só um número com nome. O número não muda a forma do    */
/* bloco (todos são o mesmo cubo); ele só escolhe a cor, lá em        */
/* corDaFace(). Para criar um tipo novo: um #define aqui e um case    */
/* com a cor em corDaFace().                                          */
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


/* ================================================================== */
/* Declarações antecipadas                                            */
/* Funções e variáveis usadas antes do ponto do arquivo em que        */
/* são definidas. Servem também de índice do que cada parte faz.      */
/* ================================================================== */

/*
 * O cenário: o mapa de alturas, a matriz 3D com o tipo de cada bloco,
 * a construção do mundo a partir do mapa e o desenho dos blocos.
 */
/* tipo de cada bloco, indexado como mundo[x][y][z] (ver MUNDO) */
extern unsigned char mundo[LADO][ALTURA_MUNDO][LADO];

/* desenho de um bloco */
void corDaFace(int tipo, int ehTopo, float brilho, float tom);
void desenhaBloco(int tipo, float tom);     /* cubo de lado 1 na origem */
void blocoEm(int tipo, int x, int y, int z);

/* acesso à matriz */
int  alturaDoTerreno(int x, int z);
int  dentroDoMundo(int x, int y, int z);
void colocaBloco(int tipo, int x, int y, int z);
int  blocoNoMundo(int x, int y, int z);

/* construção (uma vez, no início) e desenho (a cada quadro) */
void colunaEm(int x, int z);
void arvoreEm(int x, int z);
void nuvemEm(int x, int z, int largura, int fundo);
void construirMundo(void);
void desenhaCena(void);

/*
 * A posição do personagem e a colisão dele com o mundo.
 */
/* posição do personagem; jogadorY é a altura dos PÉS (ver COLISÃO) */
extern float jogadorX, jogadorY, jogadorZ;

int  blocoSolido(int x, int y, int z);
int  posicaoLivre(float x, float y, float z);
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


/* ================================================================== */
/* MUNDO                                                              */
/*                                                                    */
/* O cenário: cores e desenho dos blocos, o mapa de alturas, a matriz */
/* 3D do mundo, a construção do terreno, da árvore e das nuvens, e o  */
/* desenho da cena inteira a partir da matriz.                        */
/* ================================================================== */


/* ------------------------------------------------------------------ */
/* Cores dos blocos                                                   */
/* Escolhe a cor de UMA face do cubo, a partir de:                    */
/*   tipo    qual bloco é (GRAMA, PEDRA...), define a cor base        */
/*   ehTopo  1 se for a face de cima; a grama usa isso para ser verde */
/*           só no topo e marrom nas laterais                         */
/*   brilho  quanto clarear ou escurecer a face. Ainda não há         */
/*           iluminação (semana 6), então cada face recebe um brilho  */
/*           fixo: topo claro, laterais médias, base escura. Isso dá  */
/*           a impressão de volume.                                   */
/*   tom     pequena variação entre blocos vizinhos (xadrez), para    */
/*           dar para enxergar onde um bloco termina e o outro começa */
/* A cor final é a cor base multiplicada por brilho e tom.            */
/* ------------------------------------------------------------------ */

void corDaFace(int tipo, int ehTopo, float brilho, float tom)
{
    float r, g, b;                          /* vermelho, verde, azul, de 0 a 1 */

    switch (tipo) {
        case GRAMA:
            if (ehTopo) { r = 0.37f; g = 0.62f; b = 0.21f; }   /* verde no topo */
            else        { r = 0.53f; g = 0.38f; b = 0.26f; }   /* terra nas laterais */
            break;
        case TERRA:   r = 0.53f; g = 0.38f; b = 0.26f; break;
        case PEDRA:   r = 0.49f; g = 0.49f; b = 0.49f; break;
        case TRONCO:  r = 0.40f; g = 0.31f; b = 0.19f; break;
        case FOLHA:   r = 0.23f; g = 0.48f; b = 0.13f; break;
        case AREIA:   r = 0.86f; g = 0.81f; b = 0.58f; break;
        case AGUA:    r = 0.20f; g = 0.40f; b = 0.85f; break;
        case NUVEM:   r = 1.00f; g = 1.00f; b = 1.00f; break;
        case NEVE:    r = 0.95f; g = 0.97f; b = 1.00f; break;   /* branco levemente azulado */
        case PELE:    r = 0.80f; g = 0.60f; b = 0.47f; break;   /* personagem: */
        case CABELO:  r = 0.30f; g = 0.20f; b = 0.12f; break;
        case CAMISA:  r = 0.00f; g = 0.62f; b = 0.66f; break;
        case CALCA:   r = 0.22f; g = 0.22f; b = 0.55f; break;
        case SAPATO:  r = 0.35f; g = 0.35f; b = 0.35f; break;
        case OLHO_BRANCO: r = 0.95f; g = 0.95f; b = 0.95f; break;
        case OLHO:    r = 0.30f; g = 0.25f; b = 0.65f; break;
        default:     r = 1.00f; g = 0.00f; b = 1.00f; break;   /* rosa = tipo desconhecido */
    }

    glColor3f(r * brilho * tom, g * brilho * tom, b * brilho * tom);
}

/* ------------------------------------------------------------------ */
/* Um bloco: cubo de lado 1, do canto (0,0,0) ao canto (1,1,1).       */
/* São 6 faces, cada uma um GL_QUADS de 4 vértices.                   */
/* Os vértices de cada face estão em sentido anti-horário quando      */
/* vistos de fora do cubo. É assim que o OpenGL sabe qual lado da     */
/* face é a frente, e o GL_CULL_FACE pode pular as faces viradas de   */
/* costas para a câmera (semanas 2 e 3).                              */
/* Este cubo sempre fica na origem; quem o leva para outro lugar é    */
/* blocoEm(), logo abaixo, ou desenhaJogador().                       */
/* ------------------------------------------------------------------ */

void desenhaBloco(int tipo, float tom)
{
    glBegin(GL_QUADS);

    /* frente (z = 1) */
    corDaFace(tipo, 0, 0.80f, tom);
    glVertex3f(0, 0, 1); glVertex3f(1, 0, 1); glVertex3f(1, 1, 1); glVertex3f(0, 1, 1);

    /* trás (z = 0) */
    corDaFace(tipo, 0, 0.80f, tom);
    glVertex3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(0, 1, 0); glVertex3f(1, 1, 0);

    /* direita (x = 1) */
    corDaFace(tipo, 0, 0.65f, tom);
    glVertex3f(1, 0, 1); glVertex3f(1, 0, 0); glVertex3f(1, 1, 0); glVertex3f(1, 1, 1);

    /* esquerda (x = 0) */
    corDaFace(tipo, 0, 0.65f, tom);
    glVertex3f(0, 0, 0); glVertex3f(0, 0, 1); glVertex3f(0, 1, 1); glVertex3f(0, 1, 0);

    /* topo (y = 1): a única face com ehTopo = 1 */
    corDaFace(tipo, 1, 1.00f, tom);
    glVertex3f(0, 1, 1); glVertex3f(1, 1, 1); glVertex3f(1, 1, 0); glVertex3f(0, 1, 0);

    /* base (y = 0) */
    corDaFace(tipo, 0, 0.50f, tom);
    glVertex3f(0, 0, 0); glVertex3f(1, 0, 0); glVertex3f(1, 0, 1); glVertex3f(0, 0, 1);

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
    /* soma par: tom normal; soma ímpar: um pouco mais escuro (xadrez) */
    float tom = ((x + y + z) % 2 == 0) ? 1.0f : 0.92f;

    glPushMatrix();
    glTranslatef((float)x, (float)y, (float)z);
    desenhaBloco(tipo, tom);
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
    {  3, 3, 4, 4, 4, 4, 3, 3, 2, 2, 1, 0, 0, 1, 2, 2 },   /* z =  7  laguinho */
    {  2, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 0, 0, 1, 2, 2 },   /* z =  8 */
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
/* terreno, a árvore e as nuvens. A árvore vem depois do terreno e    */
/* escreve por cima.                                                  */
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

    nuvemEm(1, 2, 5, 3);                    /* sobre a montanha */
    nuvemEm(10, 0, 4, 2);                   /* sobre a árvore, mais ao fundo */
}

/* ------------------------------------------------------------------ */
/* A cena inteira: percorre a matriz do mundo e desenha cada posição  */
/* que tem bloco. Os blocos já foram colocados por construirMundo();  */
/* aqui só se lê a matriz. A cena sai igual à da semana 4, porque a   */
/* matriz foi preenchida pelas mesmas funções que antes desenhavam.   */
/* ------------------------------------------------------------------ */

void desenhaCena(void)
{
    int x, y, z;

    for (x = 0; x < LADO; x++)
        for (y = 0; y < ALTURA_MUNDO; y++)
            for (z = 0; z < LADO; z++)
                if (mundo[x][y][z] != VAZIO)
                    blocoEm(mundo[x][y][z], x, y, z);
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

float jogadorX = 8.5f;  /* começa no gramado, no meio em x,        */
float jogadorY = 3.0f;  /* com os pés sobre a grama (coluna de     */
float jogadorZ = 10.5f; /* altura 2, chão em y = 3); com yaw 0,    */
                        /* olha para a montanha                    */

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
    return tipo != VAZIO && tipo != AGUA;
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

int posicaoLivre(float x, float y, float z)
{
    float meia = LARGURA_JOGADOR / 2.0f;
    int x0 = (int)floorf(x - meia), x1 = (int)floorf(x + meia - FOLGA);
    int y0 = (int)floorf(y),        y1 = (int)floorf(y + ALTURA_JOGADOR - FOLGA);
    int z0 = (int)floorf(z - meia), z1 = (int)floorf(z + meia - FOLGA);
    int cx, cy, cz;

    for (cx = x0; cx <= x1; cx++)
        for (cy = y0; cy <= y1; cy++)
            for (cz = z0; cz <= z1; cz++)
                if (blocoSolido(cx, cy, cz))
                    return 0;               /* a caixa toca um bloco sólido */
    return 1;
}

/* ------------------------------------------------------------------ */
/* Resposta à colisão: o movimento é testado antes de ser aplicado,   */
/* um eixo por vez (x, z, y). Se um eixo bloqueia, os outros          */
/* continuam, e o personagem desliza rente à parede em vez de travar  */
/* na diagonal.                                                       */
/*                                                                    */
/* Deslocamentos maiores que PASSO_MAXIMO são divididos em pedaços,   */
/* para nenhum teste saltar por cima de uma parede (tunelamento).     */
/* ------------------------------------------------------------------ */

/* Aplica um pedaço do deslocamento, já pequeno o bastante para não
   atravessar paredes */
static void movePedaco(float dx, float dy, float dz)
{
    if (posicaoLivre(jogadorX + dx, jogadorY, jogadorZ)) jogadorX += dx;
    if (posicaoLivre(jogadorX, jogadorY, jogadorZ + dz)) jogadorZ += dz;
    if (posicaoLivre(jogadorX, jogadorY + dy, jogadorZ)) jogadorY += dy;
}

void moveJogador(float dx, float dy, float dz)
{
    float comprimento = sqrtf(dx * dx + dy * dy + dz * dz);
    int pedacos = (int)ceilf(comprimento / PASSO_MAXIMO);
    int i;

    if (pedacos < 1) pedacos = 1;           /* parado: um pedaço nulo */

    for (i = 0; i < pedacos; i++)
        movePedaco(dx / pedacos, dy / pedacos, dz / pedacos);
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
    desenhaBloco(tipo, 1.0f);
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
    float dx, dy, dz;
    float cabecaX = jogadorX;
    float cabecaY = jogadorY + ALTURA_OLHOS;
    float cabecaZ = jogadorZ;

    direcaoDoOlhar(&dx, &dy, &dz);

    if (modoCamera == PRIMEIRA_PESSOA) {
        gluLookAt(cabecaX, cabecaY, cabecaZ,
                  cabecaX + dx, cabecaY + dy, cabecaZ + dz,
                  0.0, 1.0, 0.0);
    } else {
        float ry = yaw * PI / 180.0f;
        float frenteX = sinf(ry), frenteZ = -cosf(ry);  /* só o plano do chão */

        gluLookAt(cabecaX - frenteX * DISTANCIA_CAMERA,  /* atrás */
                  cabecaY + ELEVACAO_CAMERA,             /* acima, altura fixa */
                  cabecaZ - frenteZ * DISTANCIA_CAMERA,
                  cabecaX,                               /* olha o personagem, */
                  cabecaY + dy * DISTANCIA_CAMERA,       /* inclinado pelo pitch */
                  cabecaZ,
                  0.0, 1.0, 0.0);
    }
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
        case 'v':
            modoCamera = (modoCamera == PRIMEIRA_PESSOA) ? TERCEIRA_PESSOA
                                                         : PRIMEIRA_PESSOA;
            break;
        case '+': case '=': mudaZoom(+1); break;    /* '=' é o + sem Shift */
        case '-': case '_': mudaZoom(-1); break;
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
        glutWarpPointer(cx, cy);
        return;
    }

    if (dx == 0 && dy == 0) return;         /* eco do glutWarpPointer */

    yaw   += dx * SENSIBILIDADE;
    pitch -= dy * SENSIBILIDADE;            /* y de tela cresce para baixo */

    if (pitch >  LIMITE_PITCH) pitch =  LIMITE_PITCH;
    if (pitch < -LIMITE_PITCH) pitch = -LIMITE_PITCH;

    glutWarpPointer(cx, cy);                /* devolve o cursor ao centro */
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
}

void liberaCursor(void)
{
    cursorCapturado = 0;
    glutSetCursor(GLUT_CURSOR_INHERIT);
}

void alternaCaptura(void)
{
    if (cursorCapturado) liberaCursor();
    else                 capturaCursor();
}

/* Clique com o cursor liberado: captura de novo */
void cliqueMouse(int botao, int estado, int x, int y)
{
    (void)botao; (void)x; (void)y;
    if (estado == GLUT_DOWN && !cursorCapturado && !menuAberto)
        capturaCursor();
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

    /* Shift: a GLUT original não avisa quando só o Shift é pressionado,
       mas a freeglut (a partir da 3.0) o entrega como tecla especial,
       GLUT_KEY_SHIFT_L ou _R, e ele entra em especiais[] como as setas */
    int sobe  = especiais[GLUT_KEY_UP]   || teclas[' '];
    int desce = especiais[GLUT_KEY_DOWN] || especiais[GLUT_KEY_SHIFT_L]
                                         || especiais[GLUT_KEY_SHIFT_R];
    float subir = (float)(sobe - desce);

    float antesX = jogadorX, antesZ = jogadorZ;
    moveJogador(mx * passo, subir * passo, mz * passo);

    /* animação: quanto o personagem realmente andou no chão (a colisão
       pode ter recusado parte do movimento) */
    float andou = sqrtf((jogadorX - antesX) * (jogadorX - antesX) +
                        (jogadorZ - antesZ) * (jogadorZ - antesZ));
    faseCaminhada += andou * FASE_POR_BLOCO;
    if (andou > 0.0f) amplitudeCaminhada += VELOCIDADE_AMPLITUDE * dt;
    else              amplitudeCaminhada -= VELOCIDADE_AMPLITUDE * dt;
    if (amplitudeCaminhada > 1.0f) amplitudeCaminhada = 1.0f;
    if (amplitudeCaminhada < 0.0f) amplitudeCaminhada = 0.0f;

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
    { "Espaço ou seta para cima", "subir" },
    { "Shift ou seta para baixo", "descer" },
    { "Setas esquerda / direita", "girar o olhar" },
    { "Mouse",                    "olhar ao redor" },
};

#define NUM_INSTRUCOES (int)(sizeof(instrucoes) / sizeof(instrucoes[0]))
#define ALTURA_LINHA 30
#define MARGEM       40
#define ESPACO_COLUNAS 40

void desenhaMenu(void)
{
    const char *titulo = "Movimentação";
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
    glPopMatrix();                                      /* modelo-visualização */
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
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

void desenha(void)
{
    /* apaga a cor (fica o azul do glClearColor) e o buffer de
       profundidade, que guarda a distância de cada pixel */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    posicionaCamera();

    desenhaCena();
    if (modoCamera == TERCEIRA_PESSOA)
        desenhaJogador();
    if (menuAberto)
        desenhaMenu();                      /* por cima de tudo */

    /* desenhamos num buffer escondido; aqui ele troca de lugar com o que
       está na tela, para não aparecer a imagem pela metade */
    glutSwapBuffers();

    /* conta os quadros e, a cada 1000 ms, mostra o total no título */
    quadros++;
    int agora = glutGet(GLUT_ELAPSED_TIME);
    if (agora - inicioContagem >= 1000) {
        char titulo[64];
        sprintf(titulo, "Minecraft | %d FPS", quadros);
        glutSetWindowTitle(titulo);
        quadros = 0;
        inicioContagem = agora;
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

    /* a GLEW só pode ser iniciada depois que a janela existe */
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        printf("Erro GLEW: %s\n", glewGetErrorString(err));
        return 1;
    }

    printf("OpenGL %s | %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));

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
    glutMouseWheelFunc(rodaMouse);          /* roda do mouse (freeglut) */
    glutMouseFunc(cliqueMouse);             /* clique: captura o cursor */
    glutEntryFunc(entradaJanela);           /* cursor entrou na janela */

    glutIdleFunc(atualiza);                 /* nada acontecendo: atualiza */

    glEnable(GL_DEPTH_TEST);                /* o mais próximo cobre o mais distante,
                                               não importa a ordem de desenho */
    glEnable(GL_CULL_FACE);                 /* não desenha faces de costas */
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);   /* cor de fundo: azul do céu */

    construirMundo();                       /* preenche a matriz, uma vez só */
    capturaCursor();                        /* o mouse já começa controlando o olhar */

    ultimoTempo = glutGet(GLUT_ELAPSED_TIME);
    inicioContagem = ultimoTempo;

    glutMainLoop();                         /* laço principal: só sai com Q no menu */
    return 0;
}
