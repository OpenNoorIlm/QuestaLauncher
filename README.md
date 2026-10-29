<div align="center">

<img src="questa_launcher.svg" width="96" height="96" alt="MultiHD Launcher logo"/>

# MultiHD Launcher

**A sleek Qt6 GUI launcher for Questa FSE · Quartus Prime · Gowin EDA on Linux**

> **⚠ Name change:** This project was previously called **QuestaLauncher**.  
> It was renamed to **MultiHDLauncher** in v1.2.0 to reflect its expanded support  
> for Quartus Prime and Gowin EDA in addition to Questa Intel FPGA FSE.  
> Release archives (v1.0, v1.1 …) keep their original titles as an archival record.

[![Release](https://img.shields.io/github/v/release/OpenNoorIlm/QuestaLauncher?style=flat-square)](https://github.com/OpenNoorIlm/QuestaLauncher/releases)
[![License](https://img.shields.io/github/license/OpenNoorIlm/QuestaLauncher?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20x86__64-blue?style=flat-square)]()
[![Qt](https://img.shields.io/badge/Qt-6.x-green?style=flat-square)]()

</div>

---

MultiHD Launcher replaces the need to hand-craft project files and export environment
variables every time you want to simulate or synthesise a SystemVerilog project.  
Pick a folder, choose your target tool(s), hit **Launch** — done.

---

## Features

| Feature | Detail |
|---|---|
| 🚀 **One-click launch** | Generates all project files and opens the chosen EDA tool |
| ⬡ **Questa / Modelsim** | Generates `run.do`, manages library deps, spawns vsim GUI |
| ◈ **Quartus Prime** | Generates `.qpf`, `.qsf` and an `.sdc` timing template |
| ◆ **Gowin EDA** | Interactive `.cst` pin-assignment editor, `.f` file list, `.gprj` |
| ⇅ **Compile-order control** | Drag-and-drop dialog to set exact `vlog` order |
| ✕ **Per-file exclusion** | Exclude individual `.sv` files; restore them any time |
| ⚙ **Persistent configuration** | vsim/quartus/gowin paths, license file, env vars via `QSettings` |
| 🔑 **License guard** | Blocks Questa launch and warns visually if license is not set |
| ⬇ **Install helpers** | Direct links to Questa FSE download & Intel license portal |
| 🎨 **Dark cyber theme** | Custom-painted grid overlay, glow effects, monospace font |

---

## Gowin EDA — generated files

When you tick **◆ Gowin EDA** and hit Launch, a constraint editor dialog opens:

| File | Purpose |
|---|---|
| `<project>.gprj` | Gowin IDE project XML (device, file list, top module) |
| `<project>.cst` | Physical constraint file — you assign pin LOC values here |
| `<project>.f` | File list for `vlog`/`iverilog -f` simulation flows |
| `impl/pnr/` | Output directory pre-created for Gowin place-and-route |

The CST dialog auto-detects ports from your top-module `.sv` file and generates a
filled template. Change the placeholder `XX` values to the actual pin numbers from
your board's schematic.

---

## Quartus Prime — generated files

| File | Purpose |
|---|---|
| `<project>.qpf` | Quartus project file |
| `<project>.qsf` | Quartus settings file (device, files, top entity) |
| `<project>.sdc` | Timing constraints template (only written if missing) |
| `output_files/` | Output directory pre-created |

---

## Questa / Modelsim — generated files

| File | Purpose |
|---|---|
| `run.do` | Questa TCL script (`vlib`, `vlog`, `vopt`, `vsim`, `run -all`) |

---

## Screenshots

> *Coming soon — contributions welcome.*

---

## Quick Start

### 1 — Download

Grab the latest `.deb` or `.AppImage` from the [Releases](https://github.com/OpenNoorIlm/QuestaLauncher/releases) page.

**Debian / Ubuntu (.deb)**
```bash
sudo dpkg -i multihd-launcher_*.deb
MultiHDLauncher          # or find it in your app launcher
```

**AppImage**
```bash
chmod +x MultiHDLauncher-*.AppImage
./MultiHDLauncher-*.AppImage
```

### 2 — Configure (required the first time)

Click **⚙ CONFIG** and fill in:

| Field | Example |
|---|---|
| vsim binary | `/home/you/altera/25.1std/questa_fse/linux_x86_64/vsim` |
| License file | `/home/you/.licenses/questa_license.dat` |
| Default folder | `/home/you/projects/` |
| Quartus binary | `/usr/bin/quartus`  *(optional)* |
| Gowin IDE binary | `/opt/gowin/IDE/bin/gw_ide`  *(optional)* |

The license status badge turns **green** when the file is found on disk.  
**Questa will refuse to start without a valid license file** — the badge and launch  
button both warn you when it is missing.

### 3 — Simulate / Synthesise

1. Browse to (or paste) your SystemVerilog project folder.
2. *(Optional)* Click **✕ DISINCLUDE** to hide files from compilation.
3. *(Optional)* Click **⇅ COMPILE ORDER** to set `vlog` order.
4. Tick the target(s) in **COMPILE FOR**: Questa, Quartus Prime, Gowin EDA.
5. Click **▶ LAUNCH**.

---

## Building from Source

See [Build.md](Build.md) for full instructions.

---

## Dependencies

See [Dependencies.md](Dependencies.md).

---

## Project Layout

```
QuestaLauncher/          ← repo root (name kept for URL continuity)
├── main.cpp              Entry point
├── mainwindow.h/.cpp     Main window (MultiHD Launcher)
├── prioritydialog.h/.cpp Compile-order drag-and-drop dialog
├── excludedialog.h/.cpp  Per-file exclusion dialog
├── configdialog.h/.cpp   Settings dialog (QSettings-backed)
├── gowincstdialog.h/.cpp Gowin .cst pin-assignment editor
├── librarymanager.h/.cpp Shared library / dependency manager
├── librarydialog.h/.cpp  Library management dialog
├── resources.qrc         Embedded SVG icon
├── questa_launcher.svg   App icon
├── MultiHDLauncher.desktop  XDG desktop entry
└── CMakeLists.txt        Build system
```

---

## Changelog

### v1.2.0 — MultiHDLauncher rename
- **Renamed** from QuestaLauncher to MultiHDLauncher
- **Gowin EDA:** interactive `.cst` pin-assignment dialog (auto-detects ports),  
  `.f` simulation file list, updated `.gprj` with selectable device
- **Quartus Prime:** `.sdc` timing-constraints template auto-generated on first run
- **All tools:** `generateRunDo` / `generateQuartusFiles` / `generateGowinFiles`  
  now factored into separate helpers for clarity
- QSettings organisation updated to `MultiHDLauncher`

### v1.1.0
- Configuration dialog (vsim path, license file, SALT_LICENSE_SERVER, MTI_VCO_MODE)
- Live license status badge; launch blocked when license missing
- Library manager for shared compiled deps
- Quartus Prime and Gowin EDA project-file generation

### v1.0.0 — QuestaLauncher
- Initial release: Questa FSE launcher, run.do generation, compile-order dialog

---

## License

[MIT](LICENSE) © OpenNoorIlm
