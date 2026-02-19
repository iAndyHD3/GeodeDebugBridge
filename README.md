# Geode Debug Bridge

This mod opens a websocket server that broadcasts geode logs, that's about it, nothing else on the C++ part.

There's a web UI available which is only a single [html file](resources/Geode%20Debug%20Bridge.html) located in the resouces folder. You can open the Web UI using the keybind `Alt+L` (configurable in geode settings)

Finally a **good** and pleasent way to view and query geode logs.

![ezgif-5648391c059f495c](https://github.com/user-attachments/assets/3be0c982-85db-4705-91ee-f6e0cb596013)

Features:

- WebSocket connection to a single Geode instance with IP + port inputs (defaults to localhost:51500)
- Auto-connects on page load
- Connect / Disconnect button
- Structured columns: Timestamp, Thread, Source (color-coded by hash), Mod ID, Tag, Severity badge, Message
- Column visibility toggles via Cols dropdown
- Severity filter pills: D / I / W / E, toggleable independently
- Smart search bar with field syntax: mod:, sev:, tag:, src:, thr:, plus free-text message search
- Full autocomplete dropdown on the search input — field prefix suggestions, severity options with colored badges, live dynamic values - (mod IDs, tags, sources, threads) collected from incoming logs
- Autocomplete keyboard navigation: ↑↓, Tab/Enter to accept, Esc to dismiss
- Click any log row to expand / collapse multi-line messages
- Auto-scroll that pauses when you scroll up to review history

# Usage:

## Windows

just install the mod and press Alt+L, the page should auto connect to the websocket server

## Android 

Make sure you have the mod installed and that you are connected to ADB, it will not work otherwise. 
Get your device's IP using `adb devices` and set that in the web UI. 

# Disclaimer

For transparency's sake, 99% of this has been vibe-coded. If you're not okay with that I don't care 👍