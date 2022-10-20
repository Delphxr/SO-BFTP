#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
// the thread function
void *connection_handler(void *);



int main(int argc, char *argv[])
{
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);
    // Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        // print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    // Listen
    listen(socket_desc, 3);
    // Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    // Accept and incoming connection
    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client,
                                 (socklen_t *)&c)))
    {
        puts("Connection accepted");
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
        if (pthread_create(&sniffer_thread, NULL, connection_handler,
                           (void *)new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
        // Now join the thread , so that we dont terminate before the thread
        // pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    // Get the socket descriptor
    int sock = *(int *)socket_desc;

    int read_size, c;
    char *message, client_message[2000];
    FILE *file;
    // Send some messages to the client

    // Receive a message from client
    while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
    {
        // Send the message back to client
        printf("Se ha recibido: %s \n", client_message);

        //vamos a leer y devolver un mensaje con el archivo!!!!!
        file = fopen(client_message, "r");
        if (file == NULL)
        {
            printf("Error opening file!\n");
            strcpy(client_message, "\nHubo un error");
            send(sock, client_message, strlen(client_message), 0);
            memset(client_message, 0, sizeof(client_message));
            continue;
        }
        int index = 0;
        while ((c = fgetc(file)) != EOF)
        {
            client_message[index] = c;
            if (index == 1999)
            {
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
    if (read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("recv failed");
    }
    // Free the socket pointer
    free(socket_desc);
    return 0;
}
