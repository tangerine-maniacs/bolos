#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <math.h>


/*
 * Función principal de todos los bolos una vez creados
 */
void mente(pid_t suBoloI, pid_t suBoloD);

int engendrar(int n, int *args, char *bolos);

/*
 * Esta función se auto documenta bien
 */
char* toString(int v);


int main(int argc, char *argv[])
{
    int argv0size, *args; 
    pid_t pid_H, pid_I, pid_E, pid_B, pid_C;
    
    /* 
     * Comprobar el bolo dependiendo del número de argumentos que se le pasen
     */
    switch (argc) 
    {
        case 1:
            // TODO: Comprobar que el nombre del porgrama es "bolos" para
            // verificar que es P (nos pueden llamar al programa con más de 
            // un argumento?).
            /* 
             * No se ha llamado con ningún argumento, es P.
             */
            switch (fork())
            {
                case -1:
                    perror("Fallo en el primer fork (1)\n");
                    exit(1);

                case 0:
                    execl(argv[0], "A", argv[0], NULL);

                default:
                    printf("P muere\n");
                    exit(0);
            }

        case 2:
            /* Tenemos 2 argumentos:
             *   - el nombre del bolo
             *   - el nombre del programa
             *  Esto sólo ocurre cuando el bolo es A.
             */
            break;

        case 4:
            /* Tenemos 4 argumentos:
             *   - nombre del bolo
             *   - argv[0] original
             *   - pid del bolo de la izquierda
             *   - pid del bolo de la derecha
             * Este es el caso para cualquier bolo que no sea A.
             */
            mente(atoi(argv[2]), atoi(argv[3]));

            /* No se debería llegar a este punto, handle hace exit. */
            perror("Ejecutado el tope de después de handle para el caso"
                "general (2)\n");
            exit(2);
    }
    
    /* 
     * Si hemos llegado aquí, el bolo es A (el único que sale del switch de
     * arriba).
     */
    /* === Engendramos a H e I, que son hijos de A, y se los tenemos que ===
     * === pasar a D, E y F.                                             ===
     */
    /* Reservamos espacio para los argumentos que le vamos a pasar a engendrar.
     * Estos bolos no tienen hijos, así que reservamos espacio para una tupla
     * de 3 argumentos.
     * El nombre del bolo, el pid del suBoloI y el pid de suBoloD.
     */
    args = malloc(sizeof(int) * 3);
    //TODO: check if malloc failed.

    /* 
     * NOTA: El primer elemento de args es un caracter, pero lo convertimos a
     * entero, y lo pasamos como entero, para que nos resulte más sencillo
     * de manejar (sólo tenemos que pasar una array).
     */
    args[0] = 'H'; args[1] = -1; args[2] = -1;
    pid_H = engendrar(1, args, argv[1]); 

    args[0] = 'I'; args[1] = -1; args[2] = -1;
    pid_I = engendrar(1, args, argv[1]); 

    /* === Engendramos a E con H e I como hijos                          ===
     */
    args[0] = 'E'; args[1] = pid_H; args[2] = pid_I;
    pid_E = engendrar(1, args, argv[1]); 

    free(args); /* Liberamos la memoria del malloc anterior. */


    /* === Engendramos las ristras BDG y CFJ.                            ===
     * === Para BDG, le pasamos a engendrar los datos de B, y sus hijos, ===
     * === D y G.                                                        ===
     */
    args = malloc(3 * 3 * sizeof(int));

    args[0] = 'G'; args[1] = -1; args[2] = -1;      // G no tiene suBolos
    args[3] = 'D'; args[4] = -1; args[5] = pid_H;   // D tiene H como suBoloD 
    args[6] = 'B'; args[7] = -1; args[8] = pid_E;   // B tiene E como suBoloD 
    pid_B = engendrar(3, args, argv[1]);

    /* === Para CFJ, le pasamos a engendrar los datos de C, y sus hijos, ===
     * === F y J.                                                        ===
     */
    args[0] = 'J'; args[1] = -1; args[2] = -1;      // J no tiene suBolos
    args[3] = 'F'; args[4] = pid_I; args[5] = -1;   // F tiene I como suBoloI
    args[6] = 'C'; args[7] = pid_E; args[8] = -1;   // C tiene E como suBoloI
    pid_C = engendrar(3, args, argv[1]);

    free(args); /* Liberamos la memoria del malloc anterior. */

    // At this point, pins 3, 5, 6 and 9 are unknown 
    mente(pid_B, pid_C);
}

/*
 * Esta función se encarga de manejar la lógica de los bolos una vez se han
 * creado.
 */
void mente(pid_t suBoloI, pid_t suBoloD)
{
    if (suBoloI != -1)
        printf("%d has %d and %d as sub-pins\n", getpid(), suBoloI, suBoloD);
    else
        printf("%d doesnt have sub-pins :(\n", getpid());

    while (1)
        pause();
    exit(0);
}

/*
 * Engendra una ristra de bolos (o un solo bolo) a partir de un array de
 * tuplas de 3 elementos (nombre, pidI, pidD).
 *
 * Devuelve el pid del primer bolo de la ristra.
 *
 * n:    número de bolos que vamos a engendrar
 *
 * args: array de tuplas de 3 elementos, cada tupla son los argumentos que
 *       vamos a pasar a la llamada de execl de un bolo.
 *       En cada tupla:
 *        - tupla[0] es el nombre del bolo
 *        - tupla[1] es el pid del suBoloI (o -1 si no tiene)
 *        - tupla[2] es el pid del suBoloD (o -1 si no tiene)
 *
 * argv0_inicial: nombre del programa inicial (argv[0] para el padre.). Lo 
 *        necesitamos para poder pasarle el nombre del programa a las llamadas
 *        de execl.
 *
 * La lista de tuplas se pasa en orden inverso (la tupla del padre es la última
 * de la lista, la de su hijo es la penúltima,...).
 */
int engendrar(int n, int *args, char *argv0_inicial)
{
    // ai = (--n) * 3  =>> empiezo a contar por 1 (--n)
    // y cada bolo necesita 3 argumentos ( * 3 )
    int f;
    int ai = (--n) * 3; /* ai = argument index 
                         * Índice del primer elemento de la última tupla (
                         * la del bolo que nos toca).
                         */

    pid_t suBoloI = args[ai + 1];
    pid_t suBoloD = args[ai + 2];
    char *name;

    switch ((f = fork()))
    {
        case -1:
            perror("Fallo al hacer fork para engendrar un hijo (3)\n");
            exit(3);

        case 0:
            /* Código del hijo, engendramos al siguiente bolo de la ristra
             * si es necesario
             */
            if (n > 0)
            {
                /* Si el subbolo de la izquierda es -1 (no existe), engendramos
                 * ese bolo.
                 * Si no, engendramos el subbolo de la derecha.
                 */
                if (suBoloI == -1)
                    suBoloI = engendrar(n, args, argv0_inicial);
                else 
                    suBoloD = engendrar(n, args, argv0_inicial);
            }
            
            /* Cuando un hijo es creado y termina de engendrar, ejecuta execl
             * para la función principal con el nombre cambiado y los siguientes
             * argumentos:
             *      [nombre, argv0_inicial, suBoloI, suBoloD]
             */
            // TODO: Esto en un ctostr()?
            //  ~ ctostr no le gusta a mi compilador :(
            name = malloc(2 * sizeof(char));
            name[0] = args[ai]; name[1] = 0;

            execl(argv0_inicial, name, argv0_inicial, toString(suBoloI), toString(suBoloD), NULL);

            /* Si llegamos aquí, algo ha fallado */
            perror("Se ha llegado al tope de ejecutar. (4)\n");
            exit(4);

        default:
            /* Somos el bolo padre (la primera tupla que se ha de engendrar),
             * devolvemos el pid del hijo que hemos creado.*/
            return f;
    }
}

char *toString(int v)
{
    int len = (int)(ceil(log10(abs(v))) + 2);
    char *str = malloc(len * sizeof(char));

    sprintf(str, "%d", v); 
    return str;
}


