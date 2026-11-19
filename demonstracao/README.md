# Demonstração dos defeitos da interface

O comportamento normal do projeto não muda quando nenhuma macro é definida.
Os dois defeitos da aula são modos de compilação temporários do próprio
`mine.c`, para evitar manter uma cópia divergente do projeto.

Execute os comandos a partir da pasta `minecraft-cg`, onde está `atlas.ppm`.

## 1. Guardar somente a matriz de projeção

### Linux/macOS

```sh
gcc mine.c -DDEMO_UI_MATRIX_ONLY=1 -o demonstracao/mine-matriz -lGLEW -lGL -lGLU -lglut -lm
./demonstracao/mine-matriz
```

No jogo, alterne para primeira pessoa e gire a câmera. A mira deverá sair do
centro porque a matriz `GL_MODELVIEW` ainda contém a transformação da câmera.

### Windows MinGW

```powershell
gcc mine.c -DDEMO_UI_MATRIX_ONLY=1 -o demonstracao/mine-matriz.exe -lglew32 -lfreeglut -lopengl32 -lglu32
.\demonstracao\mine-matriz.exe
```

## 2. Mouse sem conversão vertical

### Linux/macOS

```sh
gcc mine.c -DDEMO_UI_RAW_MOUSE_Y=1 -o demonstracao/mine-mouse -lGLEW -lGL -lGLU -lglut -lm
./demonstracao/mine-mouse
```

### Windows MinGW

```powershell
gcc mine.c -DDEMO_UI_RAW_MOUSE_Y=1 -o demonstracao\mine-mouse.exe -lglew32 -lfreeglut -lopengl32 -lglu32
.\demonstracao\mine-mouse.exe
```

No menu, mova o mouse sobre o botão `JOGAR` e observe que o destaque e o
clique são avaliados na posição verticalmente espelhada. Clique na posição
espelhada para mostrar que o botão é reconhecido no lugar errado.

## 3. Barra em posição fixa

### Linux/macOS

```sh
gcc mine.c -DDEMO_UI_FIXED_BAR=1 -o demonstracao/mine-barra -lGLEW -lGL -lGLU -lglut -lm
./demonstracao/mine-barra
```

Redimensione a janela para uma largura maior e depois menor. A barra ficará
presa à coordenada horizontal inicial e perderá o centro.

### Windows MinGW

```powershell
gcc mine.c -DDEMO_UI_FIXED_BAR=1 -o demonstracao/mine-barra.exe -lglew32 -lfreeglut -lopengl32 -lglu32
.\demonstracao\mine-barra.exe
```

## 4. Pausa sem reiniciar o tempo de quadro

### Linux/macOS

```sh
gcc mine.c -DDEMO_UI_PAUSE_CLOCK=1 -o demonstracao/mine-pausa -lGLEW -lGL -lGLU -lglut -lm
./demonstracao/mine-pausa
```

### Windows MinGW

```powershell
gcc mine.c -DDEMO_UI_PAUSE_CLOCK=1 -o demonstracao\mine-pausa.exe -lglew32 -lfreeglut -lopengl32 -lglu32
.\demonstracao\mine-pausa.exe
```

Durante o jogo, mantenha `W` pressionado, pressione `ESC`, aguarde alguns
segundos e pressione `ESC` novamente sem soltar `W`. O personagem avançará de
uma só vez, porque o tempo da pausa foi usado como um único `dt`. O modo
demonstrativo reinicia o relógio ao sair do menu principal, para que o efeito
ocorra somente no retorno da pausa. Ele também remove temporariamente o limite
de `dt` para deixar o erro visível; isso não ocorre na versão correta.

## 5. Versão correta

Compile normalmente, sem nenhuma das macros:

```sh
gcc mine.c -o demonstracao/mine-correto -lGLEW -lGL -lGLU -lglut -lm
./demonstracao/mine-correto
```

No Windows MinGW:

```powershell
gcc mine.c -o demonstracao/mine-correto.exe -lglew32 -lfreeglut -lopengl32 -lglu32
.\demonstracao\mine-correto.exe
```

Não é necessário alterar ou reverter o código entre as demonstrações: cada
executável é compilado com uma flag independente, e a compilação sem flags
preserva a implementação correta.
