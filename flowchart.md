flowchart TD
    A([START]) --> B[เปิดใช้งานโมดูล]
    B --> C[เชื่อมต่อ Wi-Fi<br/>ด้วย Credentials ที่กำหนด]
    C --> D{เชื่อมต่อ Wi-Fi<br/>สำเร็จหรือไม่?}
    D -- ไม่ --> C
    D -- ใช่ --> E[เชื่อมต่อกับ Server]
    E --> F{เชื่อมต่อ Server<br/>สำเร็จหรือไม่?}
    F -- ไม่ --> E
    F -- ใช่ --> G{Magnetic Sensor 1 ON?}
    F -- ใช่ --> H{Magnetic Sensor 2 ON?}

    G -- ใช่ --> H
    G -- ไม่ --> I[Motor 1 ON<br/>เปิดกล่อง 1]
    I --> J[ตรวจสอบ LDR Sensor 1]
    J --> K[เริ่มจับเวลา Timer]
    K --> L{ตรวจพบการเคลื่อนไหว<br/>จาก Sensor หรือไม่?}
    L -- ใช่ --> M[อัปเดตข้อมูลไปยัง Server]
    L -- ไม่ --> N{ครบเวลา 30 วินาทีหรือไม่?}
    N -- ไม่ --> L
    N -- ใช่ --> M
    M --> O[ปิดประตู 1]

    H -- ใช่ --> P[อัปเดตข้อมูลไปยัง Server]
    H -- ไม่ --> Q[Motor 2 ON<br/>เปิดกล่อง 2]
    Q --> R[ตรวจสอบ LDR Sensor 2]
    R --> S[เริ่มจับเวลา Timer]
    S --> T{ตรวจพบการเคลื่อนไหว<br/>จาก Sensor หรือไม่?}
    T -- ใช่ --> U[อัปเดตข้อมูลไปยัง Server]
    T -- ไม่ --> V{ครบเวลา 30 วินาทีหรือไม่?}
    V -- ไม่ --> T
    V -- ใช่ --> U
    U --> W[ปิดประตู 2]

    O --> X[วนกลับไปตรวจสอบสถานะ]
    P --> X
    W --> X
    X --> G

    classDef startEnd fill:#f8bbd0,stroke:#c2185b,stroke-width:2px,color:#222;
    classDef process fill:#d6ecff,stroke:#1976d2,stroke-width:1.5px,color:#222;
    classDef decision fill:#fff0a8,stroke:#b8860b,stroke-width:1.5px,color:#222;
    class A startEnd;
    class B,C,E,I,J,K,M,O,Q,R,S,P,U,W,X process;
    class D,F,G,H,L,N,T,V decision;

