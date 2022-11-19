#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>


/**
 * Self expl
 */
void handle(pid_t left_subpin, pid_t right_subpin);
/**
 * Spawns a series of pins giving each 3 args:
 *  Name 
 *  pid of pin down left        - or -1 if it doesn't have one
 *  pid of pin down right       - or -1 if it doesn't have one
 *
 *      If only one of the pins is -1 and the pin
 *      should have a child that child will be used
 *
 * It will access the args in inverse order, so 
 * for 3 pins, the following call could be used: 
 *      spawnPin(3, {name_of_3, lp3, rp3, name_of_2, lp2, rp2, name_of_1, lp1, rp1})
 *
 * Returns the pid of the topmost child
 *
 */
int spawnChilds(int n, int *args, char *argv0);


int main(int argc, char *argv[])
{
    // Declaring vars first because old c 
    int argv0size, *args; 
    pid_t pin_pids[10];
    

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

    // Now this is A
    // I have to change my name first :) 
    argv0size = strlen(argv[0]);
    strncpy(argv[0], "A", argv0size);

    pin_pids[0] = getpid();  

    // Spawn H and I first
    args = malloc(sizeof(int) * 3);

    args[0] = 'H'; args[1] = -1; args[2] = -1;
    pin_pids[7] = spawnChilds(1, args, argv[0]); 

    args[0] = 'I'; args[1] = -1; args[2] = -1;
    pin_pids[8] = spawnChilds(1, args, argv[0]); 

    // Now spawn E with I and H as sub-pins 
    args[0] = 'E'; args[1] = pin_pids[7]; args[2] = pin_pids[8];
    pin_pids[4] = spawnChilds(1, args, argv[0]); 
    free(args);

    // Now the fan part:
    //     Spawn B, D, G and C, F, J
    args = malloc(3 * 3 * sizeof(int));             // 3 pins with 3 ints/pid_t each

    args[0] = 'G'; args[1] = -1; args[2] = -1;      // G has no sub pins
    args[3] = 'D'; args[4] = -1; args[5] = pin_pids[7]; // D has H as is rsub_pin 
    args[6] = 'B'; args[7] = -1; args[8] = pin_pids[4]; // B has E as is rsub_pin 
    pin_pids[1] = spawnChilds(3, args, argv[0]);

    args[0] = 'J'; args[1] = -1; args[2] = -1;      // J has no sub pins
    args[3] = 'F'; args[4] = pin_pids[8]; args[5] = -1; // F has I as is lsub_pin 
    args[6] = 'C'; args[7] = pin_pids[4]; args[8] = -1; // C has E as is lsub_pin 
    pin_pids[2] = spawnChilds(3, args, argv[0]);

    free(args);

    // At this point, pins 3, 5, 6 and 9 are unknown 
    handle(pin_pids[1], pin_pids[2]);
}


void handle(pid_t left_subpin, pid_t right_subpin)
{
    if (left_subpin != -1)
        printf("%d has %d and %d as sub-pins\n", getpid(), left_subpin, right_subpin);
    else
        printf("%d doesnt have sub-pins :(\n", getpid());

    while (1)
        pause();
    exit(0);
}

int spawnChilds(int n, int *args, char *argv0)
{
    // ai = (--n) * 3  =>> empiezo a contar por 1 (--n)
    // y cada bolo necesita 3 argumentos ( * 3 )
    int f, ai = (--n) * 3; 
    pid_t lsub_pin = args[ai + 1], rsub_pin = args[ai + 2];

    switch ((f = fork()))
    {
        case -1:
            perror("child spawn failed\n");
            exit(-1);

        case 0:
            // Its a child, spawn if needed
            if (n)
            {
                // Check if this should be a saved (yes, yes it should) 
                if (lsub_pin == -1)
                {
                    lsub_pin = spawnChilds(n, args, argv0);
                }
                else 
                {
                    // Si no es uno tiene que ser el otro
                    rsub_pin = spawnChilds(n, args, argv0);
                }
            }
            
            // Rename
            *argv0 = args[ai];

            // Handle
            handle(lsub_pin, rsub_pin);

            // It shouldn't reach this
            perror("this is bad!\n");
            return 0;

        default:
            // Its the father, return the pid
            return f;
    }
}



