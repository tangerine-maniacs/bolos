#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Árbol de procesos:
 *
 *              P
 *              |
 *        ------A------
 *        |  |  |  |  |
 *        B  H  E  I  C
 *        |           |
 *        D           F
 *        |           |
 *        G           J
 *
 */

void logica() {
  while (1) 
    pause();
  exit(0);
}


int numhijos(int profundidad, int posicion) {
  if (profundidad == 0) // A tiene 5 hijos
    return 5;
  else if (profundidad < 3) { // B, C, D, E, F, H, I
    if (posicion == 0) // B, D, F tienen 1 hijo
      return 1;
    else if (posicion == 4) // C tiene 1 hijo
      return 1;
    else // H, E, I tienen 0 hijos
      return 0;
  } else { // G, J
    return 0; 
  }
}

// int profundidad:
// Almacena la profundidad del proceso, así sabe cuántos hijos tiene que tener.
// El proceso P es el nivel -1, A es el nivel 0, B,H,E,I,C son el nivel 1, etc.
// Además del nivel, como son sólo los hijos de los bordes los que tienen que
// tener hijos, arbol también toma como argumento la posición entre los hermanos
// (B es el cero, H el primero, E el segundo, I el tercero y C el cuarto...).

// Crea un árbol. Función recursiva.
void arbol(int profundidad, int posicion) {
  int hijos = numhijos(profundidad, posicion);
  int hijoshechos = 0;

  fprintf(stdout, "soy %d del nivel %d y tengo que tener %d hijos\n", posicion, profundidad, hijos);

  while (hijoshechos < hijos) {
    // Creamos un hijo
    pid_t pid;
    switch (pid = fork()) {
      case -1:
        fprintf(stderr, "Error al crear el proceso %d del nivel %d\n", posicion, profundidad);
        perror("error de fork");
        exit(1);

      case 0:
        // Este es el hijo, así que llamamos a la función recursiva con el nivel
        // siguiente y la posición del hijo.
        
        arbol(profundidad + 1, hijoshechos);
        fprintf(stderr, "Un hijo ha llegado a una parte de la función a la que"
            "no debería de poder haber llegado, investigar\n");
        exit(13);

      default:
        // Este es el padre, no hacemos nada, volvemos a iterar y hacemos
        // el siguiente hijo.
        hijoshechos++;
    }
  }

  // AVISO: Podría pasar que un hijo llegase a este punto? Puede, no sé, creo que 
  // no debería de pasar, porque después de cada hijo tengo un exit. Pero por si
  // acaso, lo documento.

  // Este proceso ya no tiene que tener más hijos, así que entramos en la 
  // lógica del programa. Como esta función devuelve por sí misma, no tenemos
  // que preocuparnos de la cadena de funciones previas (para que se ejecute
  // el return de main).
  logica();

  fprintf(stderr, "Un padre ha llegado a una parte de la función a la que"
      "no debería de poder haber llegado, investigar\n");
  exit(14);
}

int main(int argc, char *args[])
{
  pid_t pid;
  switch (pid = fork()) {
    case -1:
      perror("error de fork (A)");
      exit(1);

    case 0:
      // Este es el hijo, así que llamamos a la función recursiva con el nivel
      // siguiente y la posición del hijo.
      
      arbol(0, 0);
      fprintf(stderr, "A llegado a una parte de la función a la que"
          "no debería de poder haber llegado, investigar\n");
      exit(13);

    // No ponemos case default porque el padre no hace nada, en cuanto Crea
    // a su primer hijo, se muere.
  }

  return 0;
}

