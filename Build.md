# Building QuestaLauncher from Source

## Prerequisites

| Tool | Minimum version | Install (Ubuntu/Debian) |
|---|---|---|
| GCC or Clang | GCC 11 / Clang 14 | `sudo apt install build-essential` |
| CMake | 3.16 | `sudo apt install cmake` |
| Qt6 (Core, Widgets, Svg) | 6.2 | see below |
| Git | any | `sudo apt install git` |

### Installing Qt6

**Option A — Qt Online Installer (recommended, gets the latest)**
```bash
# Download from https://www.qt.io/download-open-source
chmod +x qt-online-installer-linux-*.run
./qt-online-installer-linux-*.run
# Select Qt 6.x → Desktop gcc 64-bit
```

**Option B — System packages (Ubuntu 22.04+)**
```bash
sudo apt install \
    qt6-base-dev \
    qt6-svg-dev \
    libqt6svg6-dev \
    cmake \
    build-essential \
    ninja-build
```

> Ubuntu 20.04 ships Qt 5 only. Use the Qt Online Installer or a PPA for Qt6.

---

## Clone

```bash
git clone https://github.com/OpenNoorIlm/QuestaLauncher.git
cd QuestaLauncher
```

---

## Build

### Standard CMake build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The binary is produced at `build/QuestaLauncher`.

### With a custom Qt installation (Qt Creator / Qt Online Installer path)

```bash
cmake -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=/home/$USER/Qt/6.11.2/gcc_64
cmake --build build --parallel
```

Replace `/home/$USER/Qt/6.11.2/gcc_64` with your actual Qt installation prefix.

---

## Running

```bash
./build/QuestaLauncher
```

You can also pass a project folder as the first argument and the path field will be pre-filled:

```bash
./build/QuestaLauncher /path/to/my/sv/project
```

---

## Packaging

### .deb package

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=/home/$USER/Qt/6.11.2/gcc_64 \
      -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build --parallel
cd build && cpack -G DEB
```

The `.deb` file is written to `build/packages/deb/`.

### AppImage

Requires [linuxdeployqt](https://github.com/probonopd/linuxdeployqt/releases) on your `PATH`.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=/home/$USER/Qt/6.11.2/gcc_64 \
      -DBUILD_APPIMAGE=ON
cmake --build build --parallel
cmake --build build --target AppImage
```

The `.AppImage` is written to `build/packages/appimage/`.

---

## Building inside Qt Creator

1. Open `CMakeLists.txt` in Qt Creator (**File → Open File or Project…**).
2. Select your Qt 6 kit.
3. Click **Build** (or press `Ctrl+B`).

Qt Creator handles `CMAKE_PREFIX_PATH` automatically for the selected kit.

---

## Troubleshooting

| Error | Fix |
|---|---|
| `Could not find Qt6` | Set `-DCMAKE_PREFIX_PATH` to your Qt install prefix |
| `QStyle: incomplete type` | Make sure `<QStyle>` is included — already fixed in current source |
| `vsim: No such file` | Configure the vsim path in **⚙ CONFIG** inside the app |
| License badge stays red | The `.dat` file path in **⚙ CONFIG** must exist on disk at launch time |
