# minecraft-cg

Um cenário no estilo Minecraft feito em C com OpenGL no pipeline fixo, como material da disciplina de Computação Gráfica.

O código inteiro está em um único arquivo, [`mine.c`](mine.c), comentado para ser lido do começo ao fim. Ele é organizado em seções, nesta ordem:

| Seção | Conteúdo |
|---|---|
| Constantes | Tipos de bloco, tamanho do mundo, medidas do personagem e da câmera |
| Declarações antecipadas | Funções e variáveis usadas antes de serem definidas; servem de índice |
| TEXTURAS E TRANSPARÊNCIA | Tudo o que a aula de texturas trouxe, num bloco só: atlas, célula de cada face, faces ocultas, casinha de vidro e três passadas |
| MUNDO | Mapa, matriz do mundo, construção e desenho dos blocos |
| COLISÃO | Posição do personagem e colisão com o mundo |
| CÂMERA | Projeção, direção do olhar, primeira e terceira pessoa, desenho do personagem |
| ENTRADA | Teclado, mouse, roda e atualização do movimento |
| MENU | Menu de pausa com as instruções (ESC) |
| QUADRO E PROGRAMA PRINCIPAL | Desenho do quadro, FPS e `main` |

As funções centrais têm um comentário antes delas explicando o que fazem e por quê.

## O que tem no cenário

- Um terreno de 16 × 16 colunas de blocos.
- Uma montanha irregular com neve no topo.
- Um gramado com um laguinho.
- Uma praia e o mar.
- Uma árvore no formato do Minecraft, com as folhas vazadas.
- Uma casinha de vidro ao lado de onde o personagem começa.
- Duas nuvens.
- Um personagem no estilo do Minecraft, com braços e pernas que balançam ao andar.
- Um menu de pausa (ESC) com as instruções de movimentação.

Tudo é feito com **um único cubo de lado 1**, desenhado várias vezes em posições diferentes. Os blocos do terreno são texturizados com um atlas pixelado; a água e o vidro são semitransparentes. Não há iluminação: cada face recebe um brilho fixo (topo claro, laterais médias, base escura), multiplicado pela textura, o que dá a impressão de volume. As nuvens e o personagem continuam com cor sólida.

## Conteúdos por aula

| Semana | Conteúdo | Onde aparece no código |
|---|---|---|
| 1 | Janela, laço principal, tempo de quadro, FPS no título, ESC | `main`, `desenha`, `atualiza` |
| 2 | Primitivas (`GL_QUADS`) e cor por face | `desenhaBloco`, `corDaFace` |
| 3 | Cubo 3D, buffer de profundidade, descarte de faces, instâncias com `glPushMatrix`/`glPopMatrix` | `desenhaBloco`, `blocoEm` |
| 4 | Perspectiva, correção de proporção, câmera com `gluLookAt`, navegação pelo teclado | `aplicaProjecao`, `redimensiona`, `posicionaCamera` |
| 5 | Teclado por estado e tempo de quadro; mouse com yaw e pitch e captura do cursor; matriz do mundo e colisão por caixa alinhada aos eixos; primeira e terceira pessoa; zoom pela roda | `teclado`, `tecladoSolto`, `atualiza`, `mundo`, `posicaoLivre`, `moveJogador`, `direcaoDoOlhar`, `movimentoMouse`, `posicionaCamera`, `desenhaJogador`, `rodaMouse` |
| Texturas e transparência | Atlas lido de um PPM e enviado com `glTexImage2D`; células por face e `glTexCoord2f`; filtro `GL_NEAREST` (visual pixelado) e `GL_MODULATE`; alfa por cor-chave; recorte com `GL_ALPHA_TEST` nas folhas; mistura com `GL_BLEND` e `glDepthMask` no vidro e na água, em três passadas, com os semitransparentes ordenados; faces ocultas por vizinho opaco | `lerPPM`, `converteRGBA`, `carregaAtlas`, `celulaDaFace`, `coordenadasDaCelula`, `desenhaBloco`, `faceVisivel`, `facesVisiveis`, `desenhaOpacos`, `listaTransparentes`, `desenhaTransparentes`, `casinhaEm` |

## Dependências

- Compilador C (`gcc`)
- OpenGL, GLU, GLUT (freeglut) e GLEW

É preciso usar a **freeglut**, não outra GLUT: a roda do mouse usa `glutMouseWheelFunc`, que é extensão da freeglut, e descer com Shift usa `GLUT_KEY_SHIFT_L`, das versões mais novas dela (testado com a 3.4).

**Ubuntu / Debian**
```bash
sudo apt install build-essential freeglut3-dev libglew-dev
```

**Arch / Manjaro**
```bash
sudo pacman -S gcc freeglut glew glu
```

**Windows**: use o MinGW com as bibliotecas freeglut e GLEW instaladas.

**macOS**: não precisa instalar nada; o código usa a GLUT e o OpenGL que já vêm no sistema (blocos `#ifdef __APPLE__`). Essa GLUT não tem a roda do mouse nem avisa o Shift sozinho, então no Mac o zoom é só por `+`/`-` e descer é só pela seta ↓. O olhar pelo mouse também é tratado à parte: a GLUT da Apple trava o mouse por 0,25 s a cada reposicionamento do cursor, então no Mac o cursor é desacoplado do mouse e o deslocamento é lido direto do sistema (por isso o `-framework ApplicationServices`).

## Como compilar e rodar

**Linux**
```bash
gcc mine.c -o mine -lGLEW -lGL -lGLU -lglut -lm
./mine
```

**Windows (MinGW)**
```bash
gcc mine.c -o mine.exe -lglew32 -lfreeglut -lopengl32 -lglu32
mine.exe
```

**macOS**
```bash
clang mine.c -o mine -framework OpenGL -framework GLUT -framework ApplicationServices -lm
./mine
```

Rode o jogo **de dentro da pasta do projeto**: ele lê o `atlas.ppm` da pasta atual e, se não achar o arquivo, avisa no terminal e fecha.

## Atlas de texturas

Todas as faces vêm de uma única imagem, `atlas.ppm`: 64 × 64 pixels divididos em células de 16 × 16, quatro por linha. O arquivo é gerado pelo script `gerar_atlas.py`, que desenha cada célula com um pouco de ruído pixelado (só biblioteca padrão do Python):

```bash
python3 gerar_atlas.py
```

| Célula | Aparência | | Célula | Aparência |
|---|---|---|---|---|
| 0 | grama, topo | | 6 | folha (vazados em magenta) |
| 1 | grama, lateral | | 7 | vidro |
| 2 | terra | | 8 | água |
| 3 | pedra | | 9 | areia |
| 4 | anéis da madeira | | 10 | neve |
| 5 | casca | | 11 a 15 | livres |

O formato PPM (`P6`) não tem canal alfa. Na leitura, o jogo transforma o magenta (255, 0, 255) em transparente e dá opacidade parcial às células de vidro (0,4) e de água (0,6).

## Controles

| Entrada | Ação | Leitura |
|---|---|---|
| W / S | Andar para frente / para trás | estado |
| A / D | Andar para a esquerda / para a direita | estado |
| R (segurando) | Correr (dobra a velocidade) | estado |
| ← / → | Girar o olhar | estado |
| Espaço ou ↑ / Shift ou ↓ | Subir / descer | estado |
| Mouse | Direção do olhar (já começa capturado) | evento de movimento |
| Roda ou + / - | Aproximar / afastar (campo de visão) | evento |
| Tab | Liberar / capturar o cursor (clicar na janela também captura) | evento |
| F5 ou V | Alternar primeira / terceira pessoa | evento |
| P | Alternar perspectiva / ortográfica | evento |
| ESC | Abrir / fechar o menu de pausa, com os controles | evento |
| Q (com o menu aberto) | Sair | evento |

A coluna "Leitura" diz como cada entrada é tratada: ações contínuas, que duram enquanto a tecla está pressionada, são lidas **por estado** a cada quadro; ações discretas, que acontecem uma vez por toque, são tratadas **por evento**.

O personagem começa no gramado, olhando para a montanha, com a câmera em terceira pessoa. Não há gravidade: para subir um degrau, use Espaço ou ↑. O mouse já começa controlando o olhar; Tab libera o cursor, e Tab ou um clique na janela o capturam de novo.

## Como o cenário é montado

Os eixos do mundo são:

- **x**: da esquerda (0) para a direita (15)
- **y**: altura, a partir de 0, crescendo para cima
- **z**: do fundo do mapa (0) para a frente (15), onde fica o mar

### O mapa

O terreno vem da matriz `mapa`, que mostra o mundo **visto de cima**. Cada linha é um valor de `z`, cada número na linha é um valor de `x`, e o número em si é a **altura** daquela coluna de blocos:

| Valor | Significado |
|---|---|
| 0 | Mar (areia no fundo, água por cima) |
| 1 | Praia |
| 2 ou mais | Terra firme com grama no topo |
| 6 ou mais | Topo coberto de neve (`NIVEL_NEVE`) |

O acesso é `mapa[z][x]`, com a linha antes da coluna. Assim a tabela aparece no código do mesmo jeito que o mundo visto de cima.

### Do mapa aos blocos

1. `construirMundo`, chamada uma única vez no início, percorre os 16 × 16 quadrados do mapa e chama `colunaEm(x, z)` para cada um.
2. `colunaEm` lê a altura `h` no mapa e empilha blocos de `y = 0` até o topo. Para cada andar, uma sequência de `if` escolhe o tipo comparando `y` com `h`:
   - acima do chão e até o nível da água: **água**
   - coluna baixa: **areia**
   - topo de coluna alta: **neve**
   - topo: **grama**
   - dois andares abaixo do topo: **terra**
   - o resto: **pedra**
3. Cada bloco é gravado com `colocaBloco(tipo, x, y, z)` na matriz `mundo`.
4. `arvoreEm`, `casinhaEm` e `nuvemEm` usam o mesmo `colocaBloco` para montar a árvore, a casinha de vidro e as nuvens.
5. A cada quadro, a matriz é percorrida e `blocoEm(tipo, x, y, z)` é chamada para cada posição ocupada: ela desloca a origem com `glTranslatef` e desenha ali o cubo de sempre, só com as faces que nenhum vizinho opaco esconde (`facesVisiveis`).

### Desenho em três passadas

A mistura do vidro e da água usa a cor que já está na tela, então a ordem de desenho importa:

1. **Opacos** (terra, pedra, grama, tronco, areia, neve, nuvens), em `desenhaOpacos`.
2. **Recorte**: as folhas, com o teste de alfa ligado; os vazados são descartados. Logo depois vem o personagem.
3. **Semitransparentes**: vidro e água, em `desenhaTransparentes`, ordenados do mais distante ao mais próximo da câmera, com a mistura ligada e a escrita de profundidade desligada.

### A matriz do mundo

`mundo[x][y][z]` guarda o tipo de cada bloco (ou `VAZIO`) numa grade de 16 × 16 × 16. **Atenção à ordem**: o `mapa` é `[z][x]`, para parecer o mundo visto de cima, mas o `mundo` segue a ordem dos eixos, `[x][y][z]`. Todo acesso passa por `colocaBloco` e `blocoNoMundo`, que também testam os limites.

A matriz é o que permite a colisão: a célula que contém um ponto `(px, py, pz)` é `(floor(px), floor(py), floor(pz))`, então saber se o personagem bate num bloco é consultar as poucas células que a caixa dele ocupa (`posicaoLivre`). Fora do mundo conta como sólido, e a água não é sólida.

## Experimente

- **Mudar o terreno**: edite os números da matriz `mapa`. Um `0` no meio do gramado vira um lago; um `9` vira uma torre.
- **Mais ou menos neve**: altere `NIVEL_NEVE`.
- **Outra árvore**: acrescente `arvoreEm(x, z);` em `construirMundo`, num quadrado de grama.
- **Outra nuvem**: acrescente `nuvemEm(x, z, largura, fundo);` em `construirMundo`.
- **Um tipo de bloco novo**: crie um `#define` (por exemplo, `#define MADEIRA 20`; de 10 a 16 são as cores do personagem e 17 é o vidro), desenhe a célula em `gerar_atlas.py` (há 5 livres) e acrescente o `case` em `celulaDaFace`. Para cor sólida, sem textura, o `case` vai em `corDaFace`.
- **Quebrar um bloco**: `colocaBloco(VAZIO, x, y, z)` depois de `construirMundo`; o desenho e a colisão passam a ver o buraco.
