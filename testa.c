#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * Handle the execution of a process.
 *
 * The function won't return, it will end the process with exit instead.
 */
void handle();

/**
 * Spawns childs in a chain form.
 *
 * @param n: The length of the child-chain,
 */
int spawnChilds(int n);


int main()
{
    // This is P, spawn A and move to it
    switch (fork())
    {
        case -1:
            perror("the first one :(\n");
            exit(-1);

        case 0:
            break;

        default:
            printf("P dies\n");
            exit(0);
    }

    // Toma español de repente:
    //   Necesitamos alguna manera de que los procesos "conozcan" a sus
    //   "familiares".
    //   Mi idea es spawnear primero a H e I para guardar sus PIDs en variables.
    //   Luego se forkea E con esas variables ya puestas y se guarda su PID.
    //   Y por último se forkean las 2 líneas grandes.
    //   Así sabemos los dos "bolos" que hay que tirar para todos los procesos,
    //   ¡y sin transmitir información ninguna!
    //
    //   luego lo implemento

    // Now this is A
    printf("B: %d\n", spawnChilds(3));  // B, D, G
    printf("H: %d\n", spawnChilds(1));  // H
    printf("E: %d\n", spawnChilds(1));  // E
    printf("I: %d\n", spawnChilds(1));  // I
    printf("C: %d\n", spawnChilds(3));  // C, F, J

    handle();
}

void handle()
{
    while (1)
        pause();
    exit(0);
}

int spawnChilds(int n)
{
    int f;
    n--;

    switch ((f = fork()))
    {
        case -1:
            perror("child spawn failed\n");
            exit(-1);

        case 0:
            // Its a child, spawn if needed
            if (n)
                spawnChilds(n);

            // Handle
            handle();

            // It shouldn't reach this
            perror("this is bad!\n");
            return 0;

        default:
            // Its the father, return the pid
            return f;
    }
}



