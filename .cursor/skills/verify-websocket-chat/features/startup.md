# Startup and failure handling

## Sub-features
Missing/invalid/out-of-range/trailing-junk ports and an owned busy port fail nonzero instead of misleading startup/error spin. Multi-thread/security/automaticreconnect/oldclient not covered.

## How to get to it (user POV)
Build/start server then open React UI, or run CLI for startup errors.

## Driving it with native WebSocket and Puppeteer
Use SKILL.md helper command with CHAT_SERVER_BIN. Freshloopbackinstance, two fabricatedpeers, no usercontacts.

## Gotchas
Singlethreadboundedproof only. Audit/security/concurrency/oldclientgaps remain, not wholeprojectpass.
