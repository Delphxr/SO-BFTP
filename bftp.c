#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "gui_drawer.h"

int actual_conections = 0;  // variable que cuenta el numero de clientes actuales, como guia en gui

// cuando alguien se conecta se crea un hilo con este thread (server)
void *connection_handler(void *socket_desc) {
    // Get the socket descriptor
    int sock = *(int *)socket_desc;

    int read_size, c;
    char *message, client_message[2000];
    FILE *file;
    // Send some messages to the client

    // Receive a message from client
    while ((read_size = recv(sock, client_message, 2000, 0)) > 0) {
        // Send the message back to client
        printf("Se ha recibido: %s \n", client_message);

        // vamos a leer y devolver un mensaje con el archivo!!!!!
        file = fopen(client_message, "r");
        if (file == NULL) {
            printf("Error opening file!\n");
            strcpy(client_message, "\nHubo un error");
            send(sock, client_message, strlen(client_message), 0);
            memset(client_message, 0, sizeof(client_message));
            continue;
        }
        int index = 0;
        while ((c = fgetc(file)) != EOF) {
            client_message[index] = c;
            if (index == 1999) {
                send(sock, client_message, strlen(client_message), 0);
                memset(client_message, 0, sizeof(client_message));
                index = 0;
            }
            index++;
        }
        send(sock, client_message, strlen(client_message), 0);
        fclose(file);

        memset(client_message, 0, sizeof(client_message));
    }
    if (read_size == 0) {
        puts("Client disconnected");
        actual_conections--;
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }
    // Free the socket pointer
    free(socket_desc);
    return 0;
}

// se encarga de escuchar como server, a ver si un cliente se une
void *listener_thread() {
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf(" No se pudo crear el socket\n");
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);
    // Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        // print the error message
        perror(" Bind failed. Error");
        exit(-1);
        ;
    }

    // Listen
    listen(socket_desc, 3);
    // Accept and incoming connection
    // puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    // revisamos si hay conexiones
    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client,
                                 (socklen_t *)&c))) {
        puts("Connection accepted");
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
        if (pthread_create(&sniffer_thread, NULL, connection_handler,
                           (void *)new_sock) < 0) {
            perror("could not create thread");
            exit(-1);
            ;
        }
        // Now join the thread , so that we dont terminate before the thread
        // pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
        actual_conections++;
        
    }
    if (client_sock < 0) {
        perror("accept failed");
        exit(-1);
    }
}

// nos conectamos como clientes a un server
int open_conection(char adress[]) {
    int sock;
    struct sockaddr_in server;
    char message[1000], server_reply[2000];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);
    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }
    puts("Connected\n");
    // keep communicating with server
    while (1) {
        printf("\nEnter message : ");
        scanf("%s", message);

        // Send some data
        if (send(sock, message, strlen(message), 0) < 0) {
            puts("Send failed");
            return 1;
        }
        // Receive a reply from the server

        memset(server_reply, 0, sizeof(server_reply));  // limapiamos el buffer

        if (recv(sock, server_reply, 2000, 0) < 0) {
            puts("recv failed");
            break;
        }
        puts("Server reply :");
        puts(server_reply);
    }
    close(sock);

    return 0;
}


void main(int argc, char *argv[]) {
    char input[60];      // input de comandos
    char command[60];    // comando prncipal (open, close, cd, get, etc)
    char parameter[60];  // parametros del commando (ip, archivo, etc)

    // creamos el thread listener
    pthread_t listener;
    int return_code = pthread_create(&listener, NULL, listener_thread, NULL);
    if (return_code) {
        printf("ERROR; return code from pthread_create() is %d\n", return_code);
        exit(-1);
    }

    // menu loop
    while (1) {
        clean_terminal();
        print_menu(actual_conections);

        // menu input
        fgets(input, 60, stdin);
        sscanf(input, "%s %s", command, parameter);
        // condiciones del menu
        if (strcmp(command, "open") == 0) {
            open_conection(parameter);
        } else if (strcmp(command, "close") == 0) {
            printf("commando close!");
        } else if (strcmp(command, "quit") == 0) {
            return;
            exit(0);
        } else if (strcmp(command, "cd") == 0) {
            printf("commando cd!");
        } else if (strcmp(command, "get") == 0) {
            printf("commando get!");
        } else if (strcmp(command, "lcd") == 0) {
            printf("commando lcd!");
        } else if (strcmp(command, "ls") == 0) {
            printf("commando ls!");
        } else if (strcmp(command, "put") == 0) {
            printf("commando put!");
        } else if (strcmp(command, "pwd") == 0) {
            printf("commando pwd!");
        } else {
            print_red("[!] Comando Desconocido!\n");
            for (size_t i = 0; i < 11; i++) {
                progress_bar(i / 10.0);
                usleep(35000);
            }
        }
        getchar();
    }
}