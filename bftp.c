#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
/*
██████╗       ███████╗████████╗██████╗
██╔══██╗      ██╔════╝╚══██╔══╝██╔══██╗
██████╔╝█████╗█████╗     ██║   ██████╔╝
██╔══██╗╚════╝██╔══╝     ██║   ██╔═══╝
██████╔╝      ██║        ██║   ██║
╚═════╝       ╚═╝        ╚═╝   ╚═╝
open <dirección-ip>: establece una conexión remota
close: cierra la conexión actual
quit: termina el programa
cd <directorio>: cambia de directorio remoto
get <archivo>: recupera un archivo remoto
lcd <directorio>: cambia de directorio local
ls: lista los archivos del directorio remoto
put <archivo>: envía un archivo a la máquina remota
pwd: muestra el directorio activo remoto
*/

void clean_terminal() {
    printf("\e[1;1H\e[2J");
}

void progress_bar(double percentage) {
    char bar[] = "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||";
    int pb_width = 70;
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * pb_width);
    int rpad = pb_width - lpad;
    printf("\r\033[31;1m [\033[34;1m%.*s%*s\033[0m\033[31;1m]\033[1;33m %3d%%\033[0m", lpad, bar, rpad, "", val);
    fflush(stdout);
}

// imprimimos una de las opciones del menu, con todo y color
void print_menu_option(char command[], char parameters[], char description[]) {
    printf(
        "\033[1;24;36m"
        " > %s\t",
        command);
    printf(
        "\033[34;1m"
        "%s\t"
        "\033[7;34;37m",
        parameters);
    printf("%s\033[0m \n", description);
}

// imprimimos una linea para gui
void print_linea() {
    printf("\033[1;33m------------------------------------------------------------------------\n\033[0m");
}

void print_menu() {
    printf(
        "                \033[1;33m██████\033[31;1m╗       \033[1;33m███████\033[31;1m╗\033[1;33m████████\033[31;1m╗\033[1;33m██████\033[31;1m╗ \n"
        "                \033[1;33m██\033[31;1m╔══\033[1;33m██\033[31;1m╗      \033[1;33m██\033[31;1m╔════╝╚══\033[1;33m██\033[31;1m╔══╝\033[1;33m██\033[31;1m╔══\033[1;33m██\033[31;1m╗\n"
        "                \033[1;33m██████\033[31;1m╔╝\033[1;33m█████\033[31;1m╗\033[1;33m█████\033[31;1m╗     \033[1;33m██\033[31;1m║   \033[1;33m██████\033[31;1m╔╝\n"
        "                \033[1;33m██\033[31;1m╔══\033[1;33m██\033[31;1m╗╚════╝\033[1;33m██\033[31;1m╔══╝     \033[1;33m██\033[31;1m║   \033[1;33m██\033[31;1m╔═══╝ \n"
        "                \033[1;33m██████\033[31;1m╔╝      \033[1;33m██\033[31;1m║        \033[1;33m██\033[31;1m║   \033[1;33m██\033[31;1m║     \n"
        "                \033[31;1m╚═════╝       ╚═╝        ╚═╝   ╚═╝     \n \033[0m");

    print_linea();

    print_menu_option("open ", "<dirección-ip>", "Establece una conexión remota");
    print_menu_option("close", "        ", "Cierra la conexión actual");
    print_menu_option("quit ", "        ", "Termina el programa");
    print_menu_option("cd   ", "<directorio> ", "Cambia de directorio remoto");
    print_menu_option("get  ", "<archivo> ", "Recupera un archivo remoto");
    print_menu_option("lcd  ", "<directorio>  ", "Cambia de directorio local");
    print_menu_option("ls   ", "        ", "Lista los archivos del directorio remoto");
    print_menu_option("put  ", "<archivo>", "Envía un archivo a la máquina remota");
    print_menu_option("pwd  ", "        ", "Muestra el directorio activo remoto");
    print_linea();

    printf("\n\033[34;1m > \033[0m");
}

void main(int argc, char *argv[]) {
    char input[60];

    char command[60];    // comando prncipal (open, close, cd, get, etc)
    char parameter[60];  // guardamosos parametros del commando

    while (1) {
        clean_terminal();
        print_menu();
        fgets(input, 60, stdin);
        sscanf(input, "%s %s", command, parameter);

        //condiciones del menu
        if (strcmp(command, "open") == 0) {
            printf("commando open!");
        }
        else if (strcmp(command, "close") == 0) {
            printf("commando close!");
        }
        else if (strcmp(command, "quit") == 0) {
            printf("commando quit!");
        }
        else if (strcmp(command, "cd") == 0) {
            printf("commando cd!");
        }
        else if (strcmp(command, "get") == 0) {
            printf("commando get!");
        }
        else if (strcmp(command, "lcd") == 0) {
            printf("commando lcd!");
        }
        else if (strcmp(command, "ls") == 0) {
            printf("commando ls!");
        }
        else if (strcmp(command, "put") == 0) {
            printf("commando put!");
        }
        else if (strcmp(command, "pwd") == 0) {
            printf("commando pwd!");
        }
        else{
            printf("\033[31m[!] Comando Desconocido!\033[0m");
            for (size_t i = 0; i < 11; i++)
            {
                progress_bar(i/10.0);
                usleep(35000);
     
            }
            
        }
        getchar();
    }
}