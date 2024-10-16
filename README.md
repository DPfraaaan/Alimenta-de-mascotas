# Documentación
Aquí se encuentra información sobre los distintos componentes del proyecto, la función que cumplen, algunas características importantes de cada uno y el enlace a las hojas de datos (datasheets) de cada componente.
## Descripción:
### ESP-32
<img src="https://github.com/user-attachments/assets/b91a1bf9-f857-49a9-9e0c-b8bb0b9d5d26" width="400" />

 El ESP-32 es un microcontrolador potente y versátil que cuenta con conectividad Wi-Fi y Bluetooth. Es ideal para proyectos de Internet de las Cosas (IoT) y ofrece múltiples pines de entrada/salida, así como capacidades de procesamiento para ejecutar diversas aplicaciones.
 

### Sensor Ultrasonido (HC-SR04)
<img src="https://github.com/user-attachments/assets/a186bee1-1097-4063-b07b-be7ec473d014" alt="Descripción de la imagen" width="400" />

El HC-SR04 es un sensor de distancia que utiliza ondas ultrasónicas para medir la distancia a un objeto. Emite un pulso de sonido a 40 kHz y mide el tiempo que tarda en regresar el eco. Es útil en aplicaciones como la medición de distancias y la detección de obstáculos.

### LEDs (Verde, Amarillo, Rojo)
<img src="https://github.com/user-attachments/assets/2261d343-64ba-42f8-9b59-5bce6e2c4d1e" alt="Descripción de la imagen" width="400" />

Los LEDs (diodos emisores de luz) son componentes electrónicos que emiten luz cuando se les aplica corriente. En este caso, los LEDs de colores (verde, amarillo y rojo) se pueden usar para indicar estados diferentes en un sistema, como el funcionamiento normal (verde), advertencias (amarillo) o errores (rojo).

### Buzzer
<img src="https://github.com/user-attachments/assets/44f777fd-1b99-4a28-bb05-e99e489e98f0" alt="Descripción de la imagen" width="400" />

El buzzer es un dispositivo que produce sonido al recibir una señal eléctrica. Puede ser un buzzer pasivo (requiere una señal de audio) o activo (produce un tono fijo). Se utiliza en alarmas, notificaciones y sistemas de alerta.

### Servo (SG-90)
<img src="https://github.com/user-attachments/assets/57ebe009-4789-436c-9ba3-eda464722c45" alt="Descripción de la imagen" width="400" />

El SG-90 es un servomotor de bajo costo que permite un control preciso de la posición angular. Tiene un rango de movimiento de aproximadamente 180 grados y se utiliza comúnmente en aplicaciones de robótica y automatización, como el control de brazos robóticos o mecanismos de dirección.

### Celda de Carga (1 kg) con HX711
<img src="https://github.com/user-attachments/assets/2b7285a3-4523-42de-a3dd-8ad9fc764cd6" alt="Descripción de la imagen" width="350" />
<img src="https://github.com/user-attachments/assets/bf9e8caf-4b2b-46b9-a744-ba50ad936f40" alt="Descripción de la imagen" width="350" />

La celda de carga es un sensor que mide el peso aplicando una fuerza. En este caso, tiene una capacidad de 1 kg. El HX711 es un amplificador de señal diseñado para leer la salida de la celda de carga, permitiendo medir con precisión el peso en aplicaciones de pesaje y balanzas.

### Pantalla LCD 16x2 I2C
<img src="https://github.com/user-attachments/assets/690706dc-7f37-4af6-bc7c-9b95d7367554" alt="Descripción de la imagen" width="400" />

Esta pantalla LCD tiene 16 columnas y 2 filas, y utiliza la interfaz I2C para comunicarse con otros dispositivos, lo que simplifica el cableado y la programación. Es ideal para mostrar información, como datos del sensor o mensajes al usuario, en proyectos electrónicos.
