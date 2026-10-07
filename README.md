# c-s-expr-html

Un renderizador de layouts HTML y servidor HTTP dinámico en C puro, basado en una abstracción de árbol jerárquico inspirada en expresiones S (S-expressions).

## Descripción

El proyecto explora la generación y serialización de estructuras HTML dinámicas sin frameworks ni dependencias externas. Nació como un experimento de abstracción del DOM mediante S-expressions en JavaScript, reescrito posteriormente en C como un DSL de manipulación de nodos en memoria.

## Características

* **Sin dependencias externas:** Implementado exclusivamente con la biblioteca estándar de C y la API POSIX de sockets (`sys/socket.h`, `arpa/inet.h`, `unistd.h`).


* **Estructura AST manual:** Manejo de la estructura del documento a través del tipo `Node`, que almacena etiquetas, estilos inline, texto interno y referencias a hijos.


* **Renderizado recursivo:** Serialización del árbol de nodos a un buffer de memoria de tamaño finito (`render_node`).


* **Servidor HTTP integrado:** Socket TCP escuchando peticiones en el puerto `8080` para entregar el layout directamente.



## Estructura principal

```c
typedef struct Node {
    char *tag;
    char *style;
    char *text;
    struct Node **hijos;
    int num_hijos;
} Node;
```

## Ejemplo de DSL

```c
Node *layout = udiv("display: flex; flex-direction: column; height: 100vh;", NULL, (Node*[]){
    udiv("background-color: #0000FF; height: 3vh;", NULL, NULL, 0),
    udiv("background-color: #00FF00; height: 7vh;", NULL, NULL, 0)
}, 2);
```

## Compilación y Ejecución

```bash
gcc -O2 main.c -o server
./server

```

Abrir `http://localhost:8080` en un navegador web.

## Licencia

AGPL3
