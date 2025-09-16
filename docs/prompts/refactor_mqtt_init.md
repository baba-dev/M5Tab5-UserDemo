# Refactor MQTT init prompt

1. **Goal**: Simplify MQTT initialization by extracting connection setup into a
   reusable helper.
2. **Files to touch**: `custom/integration/mqtt_client.cpp`, `custom/integration/mqtt_client.h`,
   `docs/mqtt_contract.md` (note any handshake changes).
3. **Constraints**:
   - Maintain compatibility with Home Assistant discovery topics.
   - Ensure reconnect backoff remains < 60 seconds.
   - Keep new helpers free of global state; prefer dependency injection for testability.
4. **Done when**:
   - Connection setup lives in a dedicated function with unit-test coverage.
   - Existing UI modules compile without changes.
   - Logging clearly states broker URI and credential source.
5. **Test / run command**: `idf.py build`
