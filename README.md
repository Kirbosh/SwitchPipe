<p align="center">
  <img src="./docs/banner.png" alt="Switch-NewPipe" width="848"/>
</p>

<p align="center">
  <strong>A free, open-source YouTube client for Nintendo Switch homebrew.</strong><br>
  Inspired by <a href="https://github.com/TeamNewPipe/NewPipe">NewPipe</a> &mdash; no Google account required, no ads, no tracking.
</p>

<p align="center">
  <a href="./README_kr.md">한국어</a>
</p>

---

## Screenshots

| Home Feed | Player (720p) |
|:-:|:-:|
| ![home](./docs/preview2.jpg) | ![player](./docs/preview1.jpg) |

## Install

1. Make sure your Switch has **Atmosphere CFW** with the Homebrew Menu
2. Download `SwitchPipe.nro` from the [latest release](../../releases/latest)
3. Copy it to `sdmc:/switch/SwitchPipe.nro`
4. Launch from the Homebrew Menu (it appears as **SwitchPipe**)

## What You Can Do

- Browse **Home**, **Search**, **Subscriptions**, **Library**, and **Settings**
- Watch YouTube at up to **1080p** (video+audio adaptive streaming) or 720p HLS
- **Search from any tab** with `ZL` — no need to switch to the Search tab first
- **In-player settings menu** (`+`): switch subtitles, cycle audio tracks, drop to a lower quality
- **Like**, **Subscribe**, and **post comments** on the details screen when signed in
- Personalized **home recommendations** and **subscriptions** feed when signed in
- Sign in with cookies directly from **Settings ▸ Login session** (or the Subscriptions tab)
- Save watch history and favorites locally
- Faster navigation: connections are kept alive and video details are cached across the app
- English & Korean UI

## Controls

### Main UI

| Button | Action |
|--------|--------|
| `A` | Play video from list |
| `Y` | Open video details |
| `X` | Refresh / Reset defaults |
| `ZL` | Search (from any tab) |
| `RB` | Manage login session (Subscriptions tab) |

On the details screen, `LB` opens **More**, which now includes **Like**, **Subscribe**, and **Post comment** (sign-in required for subscribe/comment).

### Player

| Button | Action |
|--------|--------|
| `A` | Pause / Resume |
| `B` | Exit player |
| `+` | Open player settings (subtitles / audio track / quality) |
| `Up / Down` | Volume |
| `X / Y` | Toggle OSD overlay |

In the settings menu: `Up / Down` to move, `A` to select, `+` to close.

## Login (Optional)

Switch-NewPipe uses cookie import for YouTube login. Full in-app Google OAuth is not
practical on Switch homebrew (no WebView, no client secret), so signing in means
importing your account cookies once — after that, like/subscribe/comment and
personalized feeds all use that session.

**How to set up:**

1. Export your YouTube cookies from a browser (using a cookie export extension)
2. Either:
   - Save the file as `sdmc:/switch/switch_newpipe_auth.txt` and open **Settings ▸ Login session ▸ Load File**, or
   - Paste the cookie string directly via **Settings ▸ Login session ▸ Paste Cookie**
   - (The Subscriptions tab's `RB` button offers the same options)

Supported formats: raw `Cookie` header, JSON `{"cookie_header":"..."}`, or Netscape `cookies.txt`.

Once logged in, your **Subscriptions** tab, **personalized Home recommendations**, and
the **Like / Subscribe / Comment** actions become available.

## Playback Quality

Configure in **Settings** tab:

| Mode | Description |
|------|-------------|
| **Standard 720p** | Tries 720p HLS first, falls back gracefully |
| **High 1080p** | Streams 1080p AVC video with a separate audio track; falls back to 720p/progressive if unavailable |
| **Compatibility** | Prefers progressive MP4 (video+audio combined) |
| **Data Saver** | Lower quality around 480p to save bandwidth |

You can also drop to a lower quality on the fly from the in-player settings menu (`+`).

## Data Files

All data is stored on your SD card:

| File | Purpose |
|------|---------|
| `sdmc:/switch/switch_newpipe.log` | Debug log |
| `sdmc:/switch/switch_newpipe_settings.json` | Settings |
| `sdmc:/switch/switch_newpipe_library.json` | Watch history & favorites |
| `sdmc:/switch/switch_newpipe_session.json` | Login session |
| `sdmc:/switch/switch_newpipe_auth.txt` | Cookie import (you provide this) |

## Build from Source

Requires Docker and a host C++ compiler.

```bash
# Full build (portlibs + app)
./build.sh

# App only (after first full build)
./build.sh --app-only

# Clean everything
./build.sh --clean
```

Output: `cmake-build-switch/switch_newpipe.nro`

<details>
<summary>Host validation tools (for development)</summary>

```bash
make host
./build/host/switch_newpipe_host
./build/host/switch_newpipe_host --search Zelda
./build/host/switch_newpipe_host --resolve 'https://www.youtube.com/watch?v=dQw4w9WgXcQ'
```

</details>

## Known Limitations

- Seek is not yet supported
- 1080p decodes H.264/AVC only (VP9-only videos fall back to 720p)
- No in-app Google OAuth (cookie import only)
- Like/Subscribe/Comment require an imported login session; comment posting depends on the video allowing comments
- Subtitles depend on the video providing caption tracks
- Channel pages are not fully browsable yet
- Comments and playlists load first page only

## Tech Stack

- **UI**: [Borealis](https://github.com/natinusala/borealis) (native Switch UI framework)
- **Playback**: mpv + FFmpeg (hardware-accelerated on Switch)
- **Networking**: libcurl + custom YouTube innertube API client
- **Build**: CMake, Docker, devkitPro toolchain

## License

This project is for educational purposes. It is not affiliated with YouTube, Google, or NewPipe.
