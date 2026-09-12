DatabaseManager
│
├── Connection management
│   ├── connect()
│   ├── disconnect()
│   ├── isConnected()
│   └── getConnectionStatus()
│
├── Server information
│   ├── getServerVersion()
│   ├── getServerAddress()
│   └── getServerParameters()
│
└── Query execution
    ├── executeQuery()
    └── checkResult()
             ↓
       PostgreSQL / libpq


DatabaseToolkit
│
├── insert()
├── update()
├── delete()
├── select()
├── findDevice()
├── addTelemetry()
└── createAlert()
             ↓
       DatabaseManager