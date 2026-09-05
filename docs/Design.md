AI-Powered Network Health Monitor
│
├── Client Simulator
│   └── Creates multiple simulated devices and generates telemetry.
│
├── Backend Server
│   ├── TCP connection handling
│   ├── Device authentication
│   ├── Telemetry validation and health rules
│   ├── PostgreSQL access
│   └── REST API for the UI
│
├── AI Diagnostics
│   └── Receives abnormal-health context from the backend and returns likely causes.
│
├── Database
│   └── Stores device details, credentials, telemetry history, and diagnostic results.
│
└── Qt/QML UI
    └── Uses the REST API to show device status, charts, alerts, and AI diagnostics.


## Data flow in our system:

TCP devices → backend → PostgreSQL; REST backend → Qt/QML; anomalies → AI diagnostics.