# MikanUI - TypeScript-based CEF Application

MikanUI is a Chromium Embedded Framework (CEF) application that provides a web-based UI for controlling the Mikan application. It's written in TypeScript and uses the existing Mikan TypeScript bindings.

## Architecture

- **C++ (CEF Host)**: Handles window creation, CEF initialization, and native operations
- **TypeScript**: UI logic using the `MikanClient` from `bindings/typescript`
- **HTML/CSS**: Modern web UI with connection panel and dynamic page loading

## Development

### Prerequisites

- Node.js and npm installed
- TypeScript will be installed via npm

### Building

The TypeScript source files are automatically compiled during the CMake build process:

1. **Automatic (CMake)**:
   ```bash
   # CMake will automatically run npm install and compile TypeScript
   cmake --build build --target MikanUI
   ```

2. **Manual (for development)**:
   ```bash
   cd src/MikanUI
   npm install
   npm run build       # Compile once
   npm run watch       # Watch for changes
   ```

### Project Structure

```
src/MikanUI/
├── src/
│   ├── cpp/                # C++ source files
│   │   ├── main.cpp       # CEF entry point
│   │   ├── MikanUIApp.cpp/h    # CEF application handler
│   │   └── MikanUIClient.cpp/h # CEF client handler
│   └── ts/                 # TypeScript source files
│       ├── app.ts         # Main application logic
│       └── native-bridge.ts    # C++ ↔ JavaScript bridge
├── resources/
│   ├── index.html         # Main HTML page
│   ├── main_menu.html     # Main menu page
│   ├── css/
│   │   └── style.css      # Styles
│   └── js/                # Compiled JavaScript output (generated)
│       ├── app.js
│       └── *.js.map
├── package.json           # npm configuration
└── tsconfig.json          # TypeScript configuration
```

### Visual Studio Project Organization

Files are organized into filters in the Visual Studio project:
- **CPP** - C++ source and header files
- **TypeScript** - TypeScript source files
- **CSS** - Stylesheet files
- **JavaScript** - Compiled JavaScript files
- **HTML** - HTML page files

## Features

### Connection Panel

- **Server URL Input**: Enter WebSocket URL (default: `ws://localhost:8080`)
- **Connect Button**: Manually connect to Mikan server
- **Launch Button**: Launch Mikan.exe via C++ and auto-connect

### Dynamic Page Loading

Once connected, MikanUI:
1. Sends `GetAppStageInfo` request to Mikan
2. Loads the appropriate HTML page based on app stage:
   - `MainMenu` → `main_menu.html`
   - `VideoSourceSettings` → `video_source_settings.html`
   - `AnchorSetup` → `anchor_setup.html`
3. Listens for `AppStageChanged` events to switch pages dynamically

### TypeScript Bindings

The app uses the existing MikanClient TypeScript bindings located in:
- `bindings/typescript/MikanClient.ts`
- `bindings/typescript/MikanRequestManager.ts`
- `bindings/typescript/MikanEventManager.ts`
- `bindings/typescript/types/`

## Adding New Pages

1. Create a new HTML file in `resources/` (e.g., `my_page.html`)
2. Add the page to the `showAppStage()` switch statement in `src/app.ts`
3. Implement page-specific initialization in `initializePage()`
4. Use `mikanClient` global for WebSocket communication

## Communication

### JavaScript → C++

Use the `NativeBridge` class:
```typescript
import { nativeBridge } from './native-bridge';

const response = await nativeBridge.launchMikan();
```

### JavaScript → Mikan (WebSocket)

Use the `MikanClient`:
```typescript
const response = await mikanClient.getAppStageInfo();
```

### Mikan → JavaScript (Events)

Listen for events via `MikanClient`:
```typescript
mikanClient.on('AppStageChanged', (data) => {
    // Handle event
});
```
