#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "gui_drawer.h"
#include "tree.h"

int actual_conections = 0;  // variable que cuenta el numero de clientes actuales, como guia en gui

// cuando alguien se conecta se crea un hilo con este thread (server)
void *connection_handler(void *socket_desc) {
    // Get the socket descriptor
    int sock = *(int *)socket_desc;

    int read_size, c;
    char *message, client_message[2000];
    FILE *file;

    char command[60];    // comando prncipal (open, close, cd, get, etc)
    char parameter[60];  // parametros del commando (ip, archivo, etc)

    // Receive a message from client
    while ((read_size = recv(sock, client_message, 2000, 0)) > 0) {
        // dividimos el input entre commando-parametro
        sscanf(client_message, "%s %s", command, parameter);

        if (strcmp(command, "cd") == 0) {
            char pwd[100];

            chdir(parameter);
            strcpy(client_message, getcwd(pwd, 100));
            send(sock, client_message, strlen(client_message), 0);

        } else if (strcmp(command, "get") == 0) {
            // vamos a leer y devolver un mensaje con el archivo!!!!!

            file = fopen(parameter, "r");
            if (file == NULL) {
                printf("Error opening file!\n");
                strcpy(client_message, "Hubo un error al intentar abrir el archivo");
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
        } else if (strcmp(command, "lcd") == 0) {
            printf("commando lcd!");
        } else if (strcmp(command, "ls") == 0) {
            strcpy(client_message, print_directorio());
            send(sock, client_message, strlen(client_message), 0);
        } else if (strcmp(command, "put") == 0) {
            printf("commando put!");
        } else if (strcmp(command, "pwd") == 0) {
            char pwd[100];

            strcpy(client_message, getcwd(pwd, 100));
            send(sock, client_message, strlen(client_message), 0);
        }

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

/* se encarga de escuchar como server, a ver si un cliente se une
y crea un thread nuevo para que sea su server privado*/
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
        actual_conections++;
    }
    if (client_sock < 0) {
        perror("accept failed");
        exit(-1);
    }
}

void main(int argc, char *argv[]) {
    char input[60];      // input de comandos
    char command[60];    // comando prncipal (open, close, cd, get, etc)
    char parameter[60];  // parametros del commando (ip, archivo, etc)

    // creamos el thread listener (server)
    pthread_t listener;
    int return_code = pthread_create(&listener, NULL, listener_thread, NULL);
    if (return_code) {
        printf("ERROR; return code from pthread_create() is %d\n", return_code);
        exit(-1);
    }

    // client
    int sock;
    struct sockaddr_in server;
    char server_reply[2000];

    // menu loop
    while (1) {
        clean_terminal();
        print_menu(actual_conections);

        // menu input
        fgets(input, 60, stdin);
        sscanf(input, "%s %s", command, parameter);
        // condiciones del menu

        if (strcmp(command, "open") == 0) {
            // Create socket
            if (sock != -1) {
                sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock == -1) {
                    printf("Could not create socket");
                }

                server.sin_addr.s_addr = inet_addr(parameter);
                server.sin_family = AF_INET;
                server.sin_port = htons(8888);
                // Connect to remote server
                if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
                    print_red(" [!] Connect failed. Error");
                }
            }
        } else if (strcmp(command, "close") == 0) {
            if (sock != -1) {
                close(sock);
            }
        } else if (strcmp(command, "quit") == 0) {
            return;
            exit(0);
        } else if (strcmp(command, "cd") == 0) {
            if (sock != -1) {
                // Send some data
                if (send(sock, input, strlen(input), 0) < 0) {
                    print_red("[!] Send failed");
                    getchar();
                    continue;
                }

                memset(server_reply, 0, sizeof(server_reply));  // limapiamos el buffer

                if (recv(sock, server_reply, 2000, 0) < 0) {
                    puts("recv failed");
                    break;
                }
                clean_terminal();
                print_line();
                puts(server_reply);
                print_line();
                print_yellow("Presione enter para continuar...");
            }
        } else if (strcmp(command, "get") == 0) {
            if (sock != -1) {
                // Send some data
                if (send(sock, input, strlen(input), 0) < 0) {
                    print_red("[!] Send failed");
                    getchar();
                    continue;
                }

                memset(server_reply, 0, sizeof(server_reply));  // limapiamos el buffer

                if (recv(sock, server_reply, 2000, 0) < 0) {
                    puts("recv failed");
                    break;
                }
                clean_terminal();
                print_line();
                puts(server_reply);
                print_line();
                print_yellow("Presione enter para continuar...");
            }
        } else if (strcmp(command, "lcd") == 0) {
            chdir(parameter);
            char pwd[100];

            clean_terminal();
            print_line();
            puts(getcwd(pwd, 100));
            print_line();
            print_yellow("Presione enter para continuar...");
        } else if (strcmp(command, "ls") == 0) {
            if (sock != -1) {
                // Send some data
                if (send(sock, input, strlen(input), 0) < 0) {
                    print_red("[!] Send failed");
                    getchar();
                    continue;
                }

                memset(server_reply, 0, sizeof(server_reply));  // limapiamos el buffer

                if (recv(sock, server_reply, 2000, 0) < 0) {
                    puts("recv failed");
                    break;
                }
                clean_terminal();
                print_line();
                puts(server_reply);
                print_line();
                print_yellow("Presione enter para continuar...");
            }
        } else if (strcmp(command, "put") == 0) {
            printf("commando put!");
        } else if (strcmp(command, "pwd") == 0) {
            if (sock != -1) {
                // Send some data
                if (send(sock, input, strlen(input), 0) < 0) {
                    print_red("[!] Send failed");
                    getchar();
                    continue;
                }

                memset(server_reply, 0, sizeof(server_reply));  // limapiamos el buffer

                if (recv(sock, server_reply, 2000, 0) < 0) {
                    puts("recv failed");
                    break;
                }
                clean_terminal();
                print_line();
                puts(server_reply);
                print_line();
                print_yellow("Presione enter para continuar...");
            }
        } else {
            print_red("[!] Comando Desconocido!\n");
            /*for (size_t i = 0; i < 11; i++) {
                progress_bar(i / 10.0);
                usleep(35000);
            }*/
        }
        getchar();
        memset(command, 0, sizeof(command));
        memset(parameter, 0, sizeof(parameter));
    }
}