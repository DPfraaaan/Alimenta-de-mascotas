# Alimentador-de-mascotas

El proyecto consiste en un alimentador automático de mascotas que consta de un tanque de comida y un plato que incluye una balanza para medir el peso de la comida que hay en él. Este dispositivo rellenará automáticamente el plato cuando esté vacío.

### Funcionamiento: 
Con un botón se permitirá cambiar la frecuencia con la que se rellena el plato, con intervalos de 4 horas.
El alimentador contará con una pantalla que mostrará información como la cantidad de comida en gramos en el plato y el tiempo restante para el próximo rellenado, en caso de que el animal haya consumido toda la comida del plato actual. Por último, incluirá un sensor ultrasónico que medirá el nivel del tanque. Si está lleno, se encenderá un LED verde y si está bajo, un LED rojo. Cuando el nivel esté muy bajo, se activará un buzzer como alarma.

### Componentes:
* Sensor ultrasonido (Hc-sr04)
* Leds (Verde ;Rojo)
* Buzzer 
* Servo (sg-90)
* ESP-32
* Celda de carga (1kg) Hx711
* Pantalla LCD 16x2 I2C
* 1 Botón genéricO

### Comunicación entre módulos y MCU
* i2c ( pantalla LCD, Servo, sensor de ultrasonido ) 
* 2 pines (Clock y Data), de forma serial muy similar a I2C (celda de carga)
* PWM (servomotor ( como tal no es un sistema ) )

## Diagrama de conexión

![image](https://github.com/user-attachments/assets/da3c9469-515d-4984-9959-3e53d2369183)

## Actualizaión:
Un botón fue eliminado debido a que no era relevante
* Se opto por solo un botón que permite cambiar los intervalos de relleno del plato de comida de a 4hs cada intervalo.

## Objetivos
* Lograr el funcionamiento propuesto en un inicio haciendo un dispositivo funcional y práctico

## Propuestas a Futuro
* Mejorar el diseño del dispositivo, (agregar más piezas impresas en 3D).
* Simplificar y mejorar las piezas impresas en 3D
* Agregar mas funcionalidades para hacerlo menos limitado.

## Problemas encontrados durante el proceso:
* Es importante tener en cuenta que el ESP trabaja con un nivel lógico de voltajes de 3,3 V, entonces la algunos de sensores u actuadores podrían no funcionar con los niveles lógicos del ESP por lo que lo ideal es implementar un logic level shifter de 5v <-- (nivel lógico de la mayoría de sensores implementados) a 3,3 V. (al final todos nuestros actuadores, sensores y LCD permitían su correcto funcionamiento con 3,3 V por lo que no se implementó).
* En nuestro caso la configuración de la resistencia de carga fue muy complicada y no pudimos conseguir el funcionamiento de esta; por esto es que decidimos hacer una versión del alimentador sin esta.

## Ideas para una futrura implementación
* Lograr el funcionamiento de la báscula para cumplir con la idea inicial. (o buscasr alternativa a esta).
* Comunicación Wifi o bluethoot o un servidor para aumentar las funcionalidades

## Cómo funciona nuestra última versión sin báscula

Básicamente cuenta con un LDR que cuando detecta mucha luz (que esta de día), llena el recipiente de comida del animal, esto lo hace una vez en el día. Cuando es de noche o hay poca luz hablita la lectura del tanque, cuando está en niveles de comida adecuados prende un led verde, intermitente; cuando esta en niveles críticos prede un led rojo y un buzer intermitente. El botón se mantuvo este permite selecionar entre perro Grande (G), Mediano (M), Chico (C). Como no tenemos una báscula para medir los niveles de comida esto lo que hace es cambiar el tiempo de apertura del servo. Más tiempo, más comida (ej perro grande), menos tiempo menos comida (perro chico). La informacón de tipo de perro estqaa mostrada en el LCD.

### ¡¡Información de hadware y software (LDR) se encuentran en el github!! 








