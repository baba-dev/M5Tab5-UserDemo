# Add toggle button prompt

1. **Goal**: Introduce a new LVGL toggle button on the home dashboard for a Home
   Assistant light entity.
2. **Files to touch**: `custom/ui/home_dashboard.cpp`, `custom/integration/mqtt_light_bridge.cpp`,
   `docs/entities.json` (add the new entity).
3. **Constraints**:
   - Follow existing LVGL component patterns (`custom::ui` namespace).
   - All commands must route through `tab5/${device_id}/command` using the contract in
     [`docs/mqtt_contract.md`](../mqtt_contract.md).
   - Keep assets under 256 KiB; prefer reusing shared icons.
4. **Done when**:
   - The toggle animates between on/off states with MQTT state feedback.
   - Entity metadata is documented in `docs/entities.json`.
   - Unit tests or mocks compile without additional warnings.
5. **Test / run command**: `idf.py build`
