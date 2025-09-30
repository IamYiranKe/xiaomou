# Repository Guidelines

## Project Structure & Module Organization
- `main/`: Core ESP-IDF C++ (audio, display, protocols, shared headers).
- `main/boards/`: Board pin maps, Kconfig overrides, and hardware notes + READMEs.
- `scripts/`: Asset and release tooling (`build_default_assets.py`, `gen_lang.py`, acoustic utils).
- `docs/`: Protocol references (MQTT, MCP, WebSocket) and integration playbooks.
- `partitions/` + `sdkconfig.defaults.*`: Partition tables and baseline configs per MCU family.

## Build, Test, and Development Commands
- `source $IDF_PATH/export.sh`: Load the ESP-IDF 5.4+ toolchain.
- `idf.py set-target <chip>`: Select the MCU (`esp32s3`, `esp32c6`, `esp32p4`, etc.).
- `idf.py -DBOARD_NAME=<board> menuconfig`: Tune board options; sync to defaults files.
- `idf.py build`: Compile firmware; rerun after code or asset changes.
- `idf.py flash monitor`: Flash hardware and stream logs for validation.
- `scripts/build_default_assets.py --board <board>`: Rebuild bundled audio/emotion/UI assets.

## Coding Style & Naming Conventions
- Apply Google C++ style: 4 spaces, inline braces, consistent `ESP_LOG*` usage.
- Keep file names lowercase with underscores; use PascalCase for classes/member functions, snake_case elsewhere.
- Run `clang-format --style=Google` on edited C++ and keep diffs free of stray whitespace.
- Name new Kconfig symbols in uppercase with underscores, mirroring ESP-IDF.

## Testing Guidelines
- No automated test suite exists; rely on `idf.py build` plus hardware validation on the target board.
- Exercise wake-word, audio, connectivity, and MCP flows with `idf.py monitor`, noting quirks in the board README.

## Commit & Pull Request Guidelines
- Follow Conventional Commit prefixes (`feat:`, `fix:`, `ci:`) and include board or subsystem context; use `Bump to <version>` for releases.
- Reference issues, list tested hardware (`esp32-s3-box3`, `atom-echo`, etc.), and call out config changes in PRs.
- Re-run `idf.py build` for each affected board and attach key logs or screenshots when behavior or UI shifts.

## Board & Asset Tips
- Start new boards by copying a similar `main/boards/*/` profile, updating `idf_component.yml`, and refreshing partition defaults.
- Keep heavy binaries in the assets partition; use `scripts/ogg_converter/` or `scripts/mp3_to_ogg.sh`, and never commit credentials from captured logs.

## Custom Hardware Reference
- `debug/sch/Netlist_Schematic1_2025-09-30.enet`: Authoritative board netlist/BOM—open it for every hardware issue and during reviews.
- Highlights: ESP32-S3-WROOM-1-N16R8 module (`ESPRESSIF`, LCSC `C2913202`), ES8311 codec (`C962342`), SPI display lines (`SPI_DC`, `SPI_CS`, `SPI_SCK`, `SPI_MOSI`, `SPI_RESET`), I²C control (`SCL`, `SDA`), and I²S audio paths (`DIN`, `DOUT`, `LRCK`, `SCLK`).
- Checklist: Verify `LED_BL`, USB `USB_IN_D±`, and 3V3 rails against the netlist before escalating hardware bugs.
