## Codigo para el esp32

## bibliografía

### Servo 
https://www.youtube.com/watch?v=jJQG936B6uM 

### Hx-711 ( celda de carga )  
https://github.com/akshayvernekar/HX711_IDF/tree/master

## Diagrama de flujo

```mermaid
flowchart TD
    %% Main initialization
    A[Start] --> B[Initialize I2C & LCD]
    B --> C[General Config\n- GPIO Setup\n- Servo PWM Setup]
    C --> D[Configure Hardware Timers\n- Ultrasonic Timer\n- Servo Timers]
    D --> E[Initialize Ultrasonic Sensor]
    
    %% Task Creation
    E --> F[Create FreeRTOS Tasks]
    
    %% Core 0 Tasks
    F --> G[Core 0]
    G --> H[Button Task]
    
    %% Core 1 Tasks
    F --> I[Core 1]
    I --> J[LCD Update Task]
    
    %% Monitor Task (Any Core)
    F --> K[Any Core]
    K --> L[Monitor Task]
    
    %% Button Task Flow
    H --> M{Button\nPressed?}
    M -- Yes --> N[Increment Timer\nby 2 Hours]
    N --> O{Hours >= 24?}
    O -- Yes --> P[Reset Hours to 0]
    O -- No --> M
    P --> M
    
    %% Timer Callback Flow
    Q[Hardware Timer\nCallback] --> R[Increment Minutes]
    R --> S{Minutes >=\nSet Hours?}
    S -- Yes --> T[Check Distance]
    
    %% Distance and Weight Check
    T --> U{Distance\n> 100cm?}
    U -- Yes --> V[Red LED Blink\nShow 'Falta comida']
    U -- No --> W{50cm < Distance\n<= 100cm?}
    W -- Yes --> X[Green LED On\nShow 'Comida ok']
    W -- No --> Y[LEDs Off]
    
    %% Weight Check and Servo Control
    V & X & Y --> Z[Check Weight]
    Z --> AA{Weight\n> 1.5kg?}
    AA -- Yes --> AB[Show 'Hay comida']
    AA -- No --> AC[Move Servo 90°]
    AC --> AD[Wait 500ms]
    AD --> AE[Move Servo 0°]
    
    %% LCD Update Task Flow
    J --> AF{Update\nNeeded?}
    AF -- Yes --> AG[Clear LCD]
    AG --> AH[Show Status]
    AH --> AI[Show Remaining Time]
    AI --> AJ[Reset Update Flag]
    AJ --> AF
    AF -- No --> J
    
    %% Monitor Task Flow
    L --> AK[Check Stack Usage]
    AK --> AL[Print Stack Info]
    AL --> L
    
    %% Styling
    classDef core0 fill:#FFB6C1,stroke:#333,stroke-width:2px
    classDef core1 fill:#90EE90,stroke:#333,stroke-width:2px
    classDef anyCore fill:#87CEEB,stroke:#333,stroke-width:2px
    classDef hardware fill:#FFD700,stroke:#333,stroke-width:2px
    classDef init fill:#DDA0DD,stroke:#333,stroke-width:2px
    
    class A,B,C,D,E,F init
    class G,H,M,N,O,P core0
    class I,J,AF,AG,AH,AI,AJ core1
    class K,L,AK,AL anyCore
    class Q,R,S,T,U,V,W,X,Y,Z,AA,AB,AC,AD,AE hardware
```

## CÓDIGO FUNCIONAL
```mermaid
flowchart TD
    %% Style definitions
    classDef start fill:#4CAF50,stroke:#45a049,color:white
    classDef init fill:#2196F3,stroke:#1976D2,color:white
    classDef process fill:#90CAF9,stroke:#64B5F6,color:black
    classDef decision fill:#FFA726,stroke:#FB8C00,color:black
    classDef alert fill:#EF5350,stroke:#E53935,color:white
    classDef success fill:#66BB6A,stroke:#4CAF50,color:white
    classDef servo fill:#AB47BC,stroke:#8E24AA,color:white
    classDef return fill:#78909C,stroke:#607D8B,color:white

    A[Start] --> B[Initialize Hardware]
    B --> C[Display 'Alimentador' on LCD]
    C --> D{Main Loop}
    
    D --> E[Read Button State]
    E --> F{Button Pressed?}
    F -->|Yes| G[Increment Pet Size]
    G --> H[Update LCD with Size]
    H --> I{5 seconds passed?}
    I -->|Yes| J[Display 'seleccionado']
    I -->|No| D
    F -->|No| K[Check LDR Value]
    
    K --> L{Is it Night?<br/>LDR > threshold}
    L -->|Yes| M[Check Food Level<br/>with Ultrasonic]
    M --> N{Distance > Max_cm?}
    N -->|Yes| O[Blink Red LED<br/>Sound Buzzer]
    N -->|No| P{Min_cm < Distance < Max_cm?}
    P -->|Yes| Q[Blink Green LED]
    
    L -->|No| R{LDR < 600 AND<br/>servobreaker true?}
    R -->|Yes| S[Open Servo 90°]
    S --> T{Check Pet Size}
    T -->|Large| U[Wait 2s]
    T -->|Medium| V[Wait 1s]
    T -->|Small| W[Wait 0.5s]
    U --> X[Close Servo 0°]
    V --> X
    W --> X
    X --> Y[Set servobreaker false]
    
    O --> D
    Q --> D
    Y --> D
    P -->|No| D
    R -->|No| D

    %% Apply styles
    class A start
    class B,C init
    class E,G,H,J,K,M process
    class F,I,L,N,P,R,T decision
    class O alert
    class Q success
    class S,U,V,W,X,Y servo
    class D return
```



