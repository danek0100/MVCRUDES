# MVCRUDES
Migartion, Versioning, CRUD, Events, Streams for C++

Project Steps:
Base:
- Reading entities
- Reading enums
- Configaration

M:
- Tables Creation
- Table initialization

V:
- Tables structures update
- Entites in table update base on version

CRUD:
- Flexible CRUD operation without SQL based on Entitie

Events:
- Events providing on changes.
- By delivery method:
-- One attempt (Push)
-- With storage (Pull)
-- One attempt with storage (Push + Pull)

Streams:
- Definition protocol's to interaction with crud provider.
-- Think about: WebSocket, HTTP (base) + Reactive (batch streaming).

Optional:
- PowerFull filtering
- Smart Cache
- Compression of Jsons.
- Codegen
- Codegen + gRPC support (if possible)
- Comfort integration with CMake

CI/CD:
- Tests
- Building lib

Doc:
- Readme
- Documentation
- Examples: MVCRUDES + Client
