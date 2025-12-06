# BeamMP Launcher — macOS Experimental Fork

This is an **experimental fork** of the BeamMP Launcher, focused on getting **BeamNG.drive multiplayer running on macOS** using CrossOver + D3DMetal.

👉 **This is NOT an official BeamMP launcher**  
👉 **Windows users should use the official release**

This fork exists only for testing and research purposes.

---

## ⚠️ Status (Experimental)

Multiplayer **launches**, **connects to servers**, and **vehicles sync correctly** — but stability still varies.

Common issues I'm investigating:
- occasional freezes during heavy physics events  
- reconnect loops  
- TCP/UDP desync in some situations  
- sensitivity to Wine/CrossOver versions

This fork includes several fixes:
- improved zlib compression/decompression handling  
- corrected TCP/UDP packet reception (partial reads, buffer issues)

Some of these problems are also reported on the official BeamMP GitHub.
A few of them match issues that I already fixed in this fork
(for example the zlib decompression failures that appear when the server sends a high volume of data quickly).

Because of this, please keep in mind that the official launcher itself
is not always perfectly stable — especially under heavier network load.
So not everything you experience is caused by macOS or this fork.

https://github.com/BeamMP/BeamMP-Launcher/issues

---

## 🔧 How to Build (macOS → Windows cross‑compile)

A simplified build guide:

### Requirements  
- Homebrew  
- CMake  
- Python 3  
- llvm-mingw (ucrt, macOS universal) — https://github.com/mstorsjo/llvm-mingw/releases  
- vcpkg (handled automatically)

### Steps
```
git clone https://github.com/Alien4042x/BeamMP-Launcher.git
cd BeamMP-Launcher
chmod +x build_windows_on_macos.sh
./build_windows_on_macos.sh
```

Output:
```
release/BeamMP-Launcher.exe
```

⚠️ **Important:**  
To ensure the launcher works correctly, you must also copy all required DLL files from release folder:

## 🚀 How to Install & Run on macOS (CrossOver / CX)

This fork is meant for macOS users running BeamNG.drive through **CrossOver**.

### 1️⃣ Download BeamMP Client
Before using this launcher, download the official BeamMP client:
https://beammp.com/download

Do NOT run the downloaded Windows launcher — we will replace it.

### 2️⃣ Download macOS Experimental Launcher Build
Go to the **Releases** tab of this fork and download the ZIP containing:
- `BeamMP-Launcher.exe`
- required `.dll` files

Extract the ZIP.

### 3️⃣ Copy the launcher into your CrossOver Bottle
Move all extracted files into:

```
/Users/<your username>/<Crossover/CXBottles>/Steam/drive_c/users/crossover/AppData/Roaming/BeamMP-Launcher
```

(If the folder doesn't exist, run the official client once — it will create it.)

### 4️⃣ Start Steam (inside CrossOver)
BeamMP requires Steam running in the same bottle.

Open CrossOver → start the Steam app from that bottle.

### 5️⃣ Run the Launcher
Double‑click:

```
BeamMP-Launcher.exe
```

A black console window should appear first — then the BeamMP launcher UI.

### 6️⃣ First-time Setup
On first launch:
- open BeamNG through Steam
- check that the **Multiplayer** tab appears in the main menu
- close the game

❗ *Sometimes the first run doesn’t hook correctly.*  

### 7️⃣ Start Multiplayer
Launch the BeamMP launcher again, then run the game through it.

### 8️⃣ Successful Load Indicator
If you see:

```
Mod caching directory: ./Resources
```

…then everything is hooked correctly and MP should start working.

---

## 🧪 macOS Compatibility Notes

This fork currently targets:
- CrossOver 25  
- Steam BeamNG (Windows build)

This project **does not modify the game** — only the launcher.

---

## 📫 Feedback / Issues

If you want to test or report bugs:
- open a GitHub Issue  
- or comment under my latest YouTube video showcasing BeamNG multiplayer on macOS

---

## License

This project remains licensed under the original **GNU AGPL v3**.  
BeamMP Launcher © 2024 BeamMP Ltd., team & contributors.
