# Loxie Lang

Uma linguagem desenvolvida ao longo da minha leitura do (excelente!) [Crafting Interpreters](https://craftinginterpreters.com/), com algumas modificações próprias feitas ao longo do caminho.

## Objetivos

1. Aprender como compiladores e interpretadores funcionam;
2. Criar uma linguagenzinha própria que é realmente utilizável;
3. Criar um... Portugol 2...?

## Como compilar

Instruções sobre como compilar a linguagem podem ser encontradas no Makefile. Você vai precisar de usar alguma ferramenta que possa compilar com Makefiles, como o GCC. Eu uso a distribuição [MinGW](https://www.mingw-w64.org/), mas você pode usar qualquer uma que contenha alguma ferramenta make.

Resumidamente, para compilar a linguagem, clone o repositório/extraia o zip, vá pra pasta principal e rode (não copie o '>'):

```
> make -f Makefile [RELEASE="Y"]
```

(a parte entre [] é opcional, e só compila com algumas otimizações)

Tudo deve compilar sem erros. Depois, vá pra pasta `out/` e rode:

```
> loxiec
```

Isso inicia a sessão REPL! Digite algumas expressões:

```
> imprima 1 + 1;
2

> imprima (2 + 3) * 4;
20

> imprima "Ola, mundo!";
Ola, mundo!
```

Você também pode passar o nome de um arquivo:

```
> loxiec caminho/pro/arquivo.lox
```

Tada! Você conseguiu!

**TODO: Fazer um tutorial completo!!**

