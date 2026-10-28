# Dependencies

QuestaLauncher is a pure Qt6 application with no third-party library dependencies beyond what ships with Qt and a standard Linux toolchain.

---

## Runtime Dependencies

### Questa Intel FPGA Starter Edition (FSE)

| Item | Notes |
|---|---|
| **vsim** binary | The Questa simulation executable (`linux_x86_64/vsim`) |
| **License file** | A valid `.dat` or `.lic` file issued by Intel/Siemens EDA |

Questa FSE can be downloaded from the [Intel software portal](https://www.intel.com/content/www/us/en/software-kit/825277/questa-intel-fpgas-standard-edition-software-version-23-1-1.html).  
A free license can be requested at the [Intel PSG license portal](https://licensing.intel.com/psg/s/sales-signup-evaluationlicenses).

### Qt 6 runtime libraries

If you install via `.deb`, these are pulled in automatically by the package manager.  
If you run the `.AppImage`, all Qt libraries are bundled — no system Qt required.

| Qt module | Used for |
|---|---|
| `Qt6::Core` | `QSettings`, `QProcess`, `QDir`, `QFile`, `QStringList` |
| `Qt6::Widgets` | All UI widgets, dialogs, layouts |
| `Qt6::Svg` | Rendering the embedded SVG icon |

**Minimum Qt version:** 6.2 (LTS)  
**Tested with:** Qt 6.11.2 (gcc_64, Linux)

### System libraries (pulled transitively)

| Library | Purpose |
|---|---|
| `libstdc++` / `libgcc` | C++ runtime |
| `libGL` | Qt Widgets GPU compositing |
| `libfontconfig`, `libfreetype` | Font rendering |
| `libxcb` and friends | X11 / Wayland platform plugin |

These are present on any modern Ubuntu/Debian/Fedora desktop install.

---

## Build-time Dependencies

See [Build.md](Build.md) for the complete list of tools needed to compile from source.

---

## No internet connection required

QuestaLauncher does **not** make any network requests at runtime.  
The "Install Questa" and "License Request" buttons simply open URLs in your default browser using `QDesktopServices::openUrl`.
