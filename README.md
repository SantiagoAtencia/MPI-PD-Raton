# MPI-PD-Raton

de claudio @clamaveruma

Versión "navidad"

El programa está desarrollado en codespaces. Así no hay que configurar nada localmente. Si se quiere, se puede pasar a linux, tal cual.
Tiene un Makefile. Así que con 
 make run

ya lo ejecuta y copila si es necesario.
Además he puesto un "make install" para instalar el entorno si fuera necesario. En el codespaces ya está todo instalado, lógicamente.

La versión actual:
  - Genera unos sub-laberintos de forma aleatoria, puede que sean válidos o no. Queda pendiente mejorar
  - Al juntarse no se comprueba nada. Queda pendiente mejorar

En la parte de la carrera hay 3 procesos:
 - rank 0. Es el controlador del juego
 - rank 1. RAtón
 - rank 2. Gato

*El controlador del juego:*
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
    - fin de bucle 

*rutina de ratón* ( gato: idem)
 - recive el laberinto
 - bucle:
     - Espera un tiempo fijo (sleep)
     - Si hay un mensaje del controlador de juego: salir del bucle
     - Actualiza su posición
     - Envia posición a controlador del juego
     - fin de bucle

