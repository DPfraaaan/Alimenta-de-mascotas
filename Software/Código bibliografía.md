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
