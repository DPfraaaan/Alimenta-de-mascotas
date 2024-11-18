## Codigo para el esp32

## bibliografía

### Servo 
https://www.youtube.com/watch?v=jJQG936B6uM 

### Hx-711 ( celda de carga )  
https://github.com/akshayvernekar/HX711_IDF/tree/master

## Diagrama de flujo

```mermaid
flowchart TD
    A[Start] --> B[Initialize I2C & LCD]
    B --> C[Initialize GPIO Pins]
    C --> D[Initialize Servo & Ultrasonic]
    D --> E[Configure Hardware Timer]
    
    %% Main program flow
    E --> F{Main Loop}
    F --> G[Check Button Press]
    
    %% Button handling
    G --> H{Button Pressed?}
    H -- Yes --> I[Increment Timer Hours]
    I --> J{Hours >= 24?}
    J -- Yes --> K[Reset Hours to 0]
    J -- No --> F
    K --> F
    
    %% Timer callback flow
    F --> L{Timer Interrupt\n Every Minute}
    L --> M[Increment Minutes]
    M --> N{Minutes >= Set Hours?}
    
    %% Distance checking and feeding logic
    N -- Yes --> O[Read Ultrasonic Distance]
    O --> P{Distance > 100cm?}
    P -- Yes --> Q[Toggle Red LED\nShow 'Falta comida']
    P -- No --> R{50cm < Distance <= 100cm?}
    R -- Yes --> S[Turn On Green LED\nShow 'Comida ok']
    R -- No --> T[Turn Off Green LED]
    
    %% Weight checking and servo control
    Q --> U[Check Food Weight]
    S --> U
    T --> U
    U --> V{Weight > 1.5kg?}
    V -- Yes --> W[Show 'Hay comida']
    V -- No --> X[Move Servo to 90°]
    X --> Y[Wait 500ms]
    Y --> Z[Move Servo to 0°]
    
    %% Loop back
    W --> F
    Z --> F
    
    %% LCD Update Task
    F --> AA[LCD Update Task]
    AA --> BB[Display Status & Time]
    BB --> F

    style A fill:#90EE90
    style F fill:#FFB6C1
    style L fill:#FFD700
    style O fill:#87CEEB
    style U fill:#DDA0DD
```
### Segunda interación del código

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


