# Native chat protocol

## Sub-features
Real WS join, nickname, broadcast, malformed-input resilience and disconnect. The helper explicitly drives these using actual WebSocket clients.

## How to get to it (user POV)
Build/start server then open React UI, or run CLI for startup errors.

## Driving it with native WebSocket and Puppeteer
Use SKILL.md helper command with CHAT_SERVER_BIN. Freshloopbackinstance, two fabricatedpeers, no usercontacts.

## Gotchas
Singlethreadboundedproof only. Audit/security/concurrency/oldclientgaps remain, not wholeprojectpass.
