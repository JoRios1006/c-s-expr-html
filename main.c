#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 65536

typedef struct Node {
    char *tag;
    char *style;
    char *text;
    struct Node **hijos;
    int num_hijos;
} Node;

// Constructor genérico de nodos
Node* create_node(char *tag, char *style, char *text, Node **hijos, int num_hijos) {
    Node *n = (Node*)malloc(sizeof(Node));
    n->tag = tag;
    n->style = style;
    n->text = text;
    n->hijos = hijos;
    n->num_hijos = num_hijos;
    return n;
}

Node* udiv(char *style, char *text, Node **hijos, int num_hijos) {
    return create_node("div", style, text, hijos, num_hijos);
}

int render_node(Node *n, char *buf, size_t buf_size, int offset) {
    offset += snprintf(buf + offset, buf_size - offset, "<%s", n->tag);
    if (n->style && n->style[0] != '\0') {
        offset += snprintf(buf + offset, buf_size - offset, " style=\"%s\"", n->style);
    }
    offset += snprintf(buf + offset, buf_size - offset, ">");
    if (n->text) {
        offset += snprintf(buf + offset, buf_size - offset, "%s", n->text);
    }
    for (int i = 0; i < n->num_hijos; i++) {
        offset = render_node(n->hijos[i], buf, buf_size, offset);
    }
    offset += snprintf(buf + offset, buf_size - offset, "</%s>", n->tag);
    return offset;
}

void free_tree(Node *n) {
    if (!n) return;
    for (int i = 0; i < n->num_hijos; i++) {
        free_tree(n->hijos[i]);
    }
    free(n);
}
int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Crear socket TCP (IPv4, SOCK_STREAM)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error en socket()");
        exit(EXIT_FAILURE);
    }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error en bind()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Error en listen()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor HTTP corriendo en http://localhost:%d\n", PORT);
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd < 0) {
            perror("Error en accept()");
            continue;
        }
        char request_buffer[1024] = {0};
        read(client_fd, request_buffer, sizeof(request_buffer) - 1);
        Node *layout =
            udiv("display: flex; flex-direction: column; margin: 0; height: 100vh; width: 100vw;", NULL, (Node*[]){
                // Fila 1: Barra superior azul
                udiv("background-color: #0000FF; height: 3vh;", NULL, NULL, 0),
                // Fila 2: Barra secundaria verde
                udiv("background-color: #00FF00; height: 7vh;", NULL, NULL, 0),
                // Fila 3: Contenedor principal dividido en 5 columnas
                udiv("display: flex; flex-direction: row; flex: 1;", NULL, (Node*[]){
                        // Columna 1 (izquierda exterior): Roja
                        udiv("background-color: #FF0000; flex: 2;", NULL, NULL, 0),
                        // Columna 2 (izquierda interior): Azul
                        udiv("background-color: #0000FF; flex: 1;", NULL, NULL, 0),
                        // Columna 3 (Centro): Contenedor con elementos apilados verticalmente
                        udiv("display: flex; flex-direction: column; flex: 4;", NULL, (Node*[]){
                                // Centro - Bloque superior rojo
                                udiv("background-color: #FF0000; flex: 1.5;", NULL, NULL, 0),
                                // Centro - Barra separadora verde
                                udiv("background-color: #00FF00; height: 8vh;", NULL, NULL, 0),
                                // Centro - Bloque central rojo
                                udiv("background-color: #FF0000; flex: 2;", NULL, NULL, 0),
                                // Centro - Base dividida en 3 sub-columnas
                                udiv("display: flex; flex-direction: row; flex: 1.5;", NULL, (Node*[]){
                                        udiv("background-color: #00FF00; flex: 1;", NULL, NULL, 0), // Base verde izquierda
                                        udiv("background-color: #0000FF; flex: 1;", NULL, NULL, 0), // Base azul central
                                        udiv("background-color: #00FF00; flex: 1;", NULL, NULL, 0)  // Base verde derecha
                                    }, 3)
                            }, 4),
                        // Columna 4 (derecha interior): Azul
                        udiv("background-color: #0000FF; flex: 1;", NULL, NULL, 0),
                        // Columna 5 (derecha exterior): Roja
                        udiv("background-color: #FF0000; flex: 2;", NULL, NULL, 0)
                    }, 5)
            }, 3);

        char html_body[BUFFER_SIZE] = {0};
        int body_len = render_node(layout, html_body, sizeof(html_body), 0);

        char response[BUFFER_SIZE];
        int response_len = snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<!DOCTYPE html><html><body style=\"margin:0;\">%s</body></html>",
            body_len + 57, // 57 es la longitud aproximada del wrapper html/body
            html_body
        );
        write(client_fd, response, response_len);
        free_tree(layout);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
