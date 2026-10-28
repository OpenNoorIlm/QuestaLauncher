<div align="center">

<img src="questa_launcher.svg" width="96" height="96" alt="QuestaLauncher logo"/>

# QuestaLauncher

**A sleek Qt6 GUI launcher for Questa Intel FPGA Starter Edition (FSE) on Linux**

[![Release](https://img.shields.io/github/v/release/OpenNoorIlm/QuestaLauncher?style=flat-square)](https://github.com/OpenNoorIlm/QuestaLauncher/releases)
[![License](https://img.shields.io/github/license/OpenNoorIlm/QuestaLauncher?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20x86__64-blue?style=flat-square)]()
[![Qt](https://img.shields.io/badge/Qt-6.x-green?style=flat-square)]()

</div>

---

QuestaLauncher replaces the need to hand-craft `.do` scripts and export environment variables every time you want to simulate a SystemVerilog project in Questa FSE.  
Pick a folder, (optionally) tune compile order or exclude files, hit **Launch** — done.

---

## Features

| Feature | Detail |
|---|---|
| 🚀 **One-click launch** | Generates `run.do` and spawns Questa GUI automatically |
| ⇅ **Compile-order control** | Drag-and-drop dialog to set exact `vlog` order |
| ✕ **Per-file exclusion** | Exclude individual `.sv` files; restore them any time |
| ⚙ **Persistent configuration** | vsim path, license file, env vars saved via `QSettings` |
| 🔑 **License guard** | Blocks launch and warns visually if license is not set |
| ⬇ **Install helpers** | Direct links to Questa FSE download & Intel license portal |
| 🎨 **Dark cyber theme** | Custom-painted grid overlay, glow effects, monospace font |

---

## Screenshots

> *Coming soon — contributions welcome.*

---

## Quick Start

### 1 — Download

Grab the latest `.deb` or `.AppImage` from the [Releases](https://github.com/OpenNoorIlm/QuestaLauncher/releases) page.

**Debian / Ubuntu (.deb)**
```bash
sudo dpkg -i questaLauncher_*.deb
questa-launcher          # or find it in your app launcher
```

**AppImage**
```bash
chmod +x QuestaLauncher-*.AppImage
./QuestaLauncher-*.AppImage
```

### 2 — Configure (required the first time)

Click **⚙ CONFIG** and fill in:

| Field | Example |
|---|---|
| vsim binary | `/home/you/altera/25.1std/questa_fse/linux_x86_64/vsim` |
| License file | `/home/you/.licenses/questa_license.dat` |
| Default folder | `/home/you/projects/` |

The license status badge turns **green** when the file is found on disk.

### 3 — Simulate

1. Browse to (or paste) your SystemVerilog project folder.
2. *(Optional)* Click **✕ DISINCLUDE** to hide files from compilation.
3. *(Optional)* Click **⇅ COMPILE ORDER** to set `vlog` order.
4. Click **▶ LAUNCH QUESTA**.

---

## Building from Source

See [Build.md](Build.md) for full instructions.

---

## Dependencies

See [Dependencies.md](Dependencies.md).

---

## Project Layout

```
QuestaLauncher/
├── main.cpp              Entry point
├── mainwindow.h/.cpp     Main window
├── prioritydialog.h/.cpp Compile-order drag-and-drop dialog
├── excludedialog.h/.cpp  Per-file exclusion dialog
├── configdialog.h/.cpp   Settings dialog (QSettings-backed)
├── resources.qrc         Embedded SVG icon
├── questa_launcher.svg   App icon
└── CMakeLists.txt        Build system
```

---

## License

[MIT](LICENSE) © OpenNoorIlm
