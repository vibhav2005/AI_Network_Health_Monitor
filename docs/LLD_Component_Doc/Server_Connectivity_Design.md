network_health_monitor_client
          │
          └── ws2_32
              ↓
           TCP Client


network_health_monitor_server
          │
          ├── ws2_32
          │     ↓
          │   TCP Server
          │
          └── libpq
                ↓
           PostgreSQL