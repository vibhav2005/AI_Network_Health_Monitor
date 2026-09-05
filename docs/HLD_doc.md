Simulated Devices
       │ TCP telemetry
       ▼
TCP Server / Connection Handler
       │ validated events
       ▼
Thread-safe telemetry queue
       ├── Database worker → PostgreSQL
       ├── Health-rule engine → alerts / AI job queue
       └── In-memory latest device state
                              │
                         REST API
                              │
                         Qt/QML dashboard


## Client Simulator

The client simulator represents multiple network devices running locally. Each simulated device connects to the backend through TCP, authenticates itself, and periodically sends generated telemetry data such as latency, packet loss, CPU usage, memory usage, and connection status.

## Backend Server

The backend server accepts TCP device connections, authenticates devices, validates telemetry, and maintains each device’s latest health state. It stores device and telemetry data in PostgreSQL, applies health rules to identify anomalies, triggers AI diagnostics when required, and provides REST APIs for the dashboard.

## PostgreSQL Database

PostgreSQL stores device registration and authentication information, incoming telemetry records with timestamps, health alerts, and AI diagnostic results. The backend reads this data to provide historical monitoring information through the REST API.

## AI Diagnostics

The AI diagnostics component receives abnormal telemetry and relevant device history from the backend. It calls an external AI API and returns a probable cause, a human-readable explanation, and suggested corrective actions, which the backend stores and exposes to the UI.

## Qt/QML UI

The Qt/QML application is the desktop dashboard. It communicates only with the backend through REST APIs to show device health, telemetry history, alerts, and AI diagnostic results.