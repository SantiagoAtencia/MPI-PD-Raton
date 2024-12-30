# MPI-PD-Raton

## Versión "navidad"

Programa desarrollado en codespaces, no es necesario configurar nada localmente. Si se quiere, se puede pasar a linux, tiene un Makefile y con make run se compila y ejecuta.

Con make install se instala el entorno si fuera necesario. En el codespaces ya está todo instalado, lógicamente.

Lo podemos compilar y ejecutar manualmente pudiendo poner los parámetros de ejecución que deseemos.

Compilación manual (Ejecutar en raíz):

```
mpicc -I include -o maze_race src/*.c
```

Ejecución manual:

```
mpirun -np \<num_processes> ./maze_race \<width> \<height> \<play time>
```

La versión actual:

  
  - Genera unos sub-laberintos de forma aleatoria.
  - Al juntarse se comprueban y corrigen



En la parte de la carrera hay 3 procesos:
 - rank 0. Es el controlador del juego
 - rank 1. Ratón
 - rank 2. Gato

*El controlador del juego:*
```  
  - Le pasa el laberinto a los animales.
  - Mantiene una copia local de las posiciones de los animales
  - Bucle:
      - Espera mensaje de algún animal (receive bloqueante)
      - Si es del ratón, actualiza su posición y lo pinta
      - Si es del gato, idem.
      - Comprueba alguna de las 3 condiciones de fin de juego: gato come, ratón llega a meta , o ha pasado un tiempo
      - Si "fin de juego":
          - Notifica a los animales para que paren
          - Se imprime razón de finalización.
          - Salir  
      - En otro caso: imprimir tiempo restante
      - Fin de bucle 
```
*Rutina de ratón/gato*
``` 
- Recibe el laberinto
- Bucle:
    - Espera un tiempo fijo (sleep)
    - Si hay un mensaje del controlador de juego: salir del bucle
    - Actualiza su posición
    - Envia posición a controlador del juego
    - Fin de bucle
```

