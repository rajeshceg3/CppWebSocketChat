---
name: verify-websocket-chat
description: Build and drive CppWebSocketChat's real native server and production React UI with isolated loopback clients after chat changes.
---
# Native WebSocket chat verification

## Launch
Use CMake3.22+, g++11/C++17 or newer, Boost1.75+ with system/thread/json libraries and dependencies, Node22.12+, npm and Chrome. The proof runtime used privately built Boost1.83 (system/thread/json plus chrono/atomic/date_time), not system installation. Configure `cmake -S . -B /tmp/chat-build -DBOOST_ROOT=/path/to/boost -DBoost_NO_SYSTEM_PATHS=ON -DBoost_NO_BOOST_CMAKE=ON` when using a private Boost tree; otherwise normal Boost discovery applies. `cmake --build /tmp/chat-build -j2` builds server and tests. GoogleTest1.12.1 is fetched by the existing CMake recipe; network/cache is a build prerequisite, not application traffic. Run `ctest --test-dir /tmp/chat-build --output-on-failure`.
`npm ci --ignore-scripts --prefix react-chat-ui` and `npm ci --ignore-scripts --prefix .cursor/skills/verify-websocket-chat` install locked UI/helper dependencies. Set CHROME_PATH if not /usr/bin/google-chrome. Normal server command is `/tmp/chat-build/websocket-chat-server PORT [THREADS]`; normal UI is `npm start` in react-chat-ui. Verification owns separate instances.

## Doctor
`CHAT_SERVER_BIN=/tmp/chat-build/websocket-chat-server node .cursor/skills/verify-websocket-chat/helpers/verify.mjs /tmp/chat-proof` begins with executable presence, invalid-port exits and an owned busy-port rejection, then waits for its own real server startup and successful WebSocket handshake/join before driving. It runs a fresh server with default one thread. After any failure rerun from scratch; never attach to an unrelated8080 server. REACT_APP_WS_URL is set during the helper's real production UI build to its own ephemeral loopback endpoint.

## Drive
The same helper command drives two native WebSocket clients: join notifications, nickname change, malformedJSON/wrongshape/wrongtexttype ignored, subsequent valid broadcast and disconnect. It then builds real React production output, serves it on a private ephemeral port, opens two independent browser contexts at390/1365, sets nicknames through visible inputs, sends literal HTML-looking text, sees it in the other browser and reloads the sender to prove saved nickname is resent. No mocks/test-only routes/internal state setters. All chat peers are disposable helper clients, never actual contacts. CTest complements this proof but uses capturing-session mocks and is not the real networking proof.

## Evidence
Output holds result.json, server.log, ui-build.log and both-width received screenshots. Inspect actual pixels, including final animation completion, text contrast and mobile clipping. Logs contain fabricated messages only. Assert literal HTML stays text and inspect the actual sender nickname after reload, not merely the local heading. No current hosted endpoint is inferred.

## Cleanup
Helper closes only its own contexts/browser/server and temporary production build. It preserves proof outside temporary state. Confirm result.json/screens remain after exit. Server uses an owned free port; busy-port fixture is its own listener. Never kill by name or delete tracked historical logs/tests/client programs. Build output stays outside source by default; COPY_COMPILE_COMMANDS_TO_SOURCE is explicit opt-in.

## Coverage and maintenance
One server thread is proved. Existing multiple-thread mode has unsynchronized shared-session state and needs separate concurrency repair/proof; do not expose it as certified safe. No TLS/authentication/origin restrictions/rate limiting/message size/backpressure/automatic reconnect/full accessibility/device/hosted proof. UI dependency audit findings and legacy CRA test drift remain open. Old client sources and tests are preserved, not certified. Use /maintain-verification-skill after changes, keeping product regressions out of map-only maintenance.
