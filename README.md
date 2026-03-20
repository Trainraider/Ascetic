# Ascetic

A web browser that blocks images, video, and other media by default. Protect your senses from media that may inflame the passions.

Built on GTK4, Libadwaita, and WebKitGTK. This is Linux only because WebKitGTK is Linux only. (Maybe it can compile on BSDs)

Alpha software. Many features and internal signals are not yet implemented and much of the current behavior will eventually be user-configurable.

Images and SVGs are blocked via WebKit's content filter API. The only exceptions are CAPTCHA providers (Google, hCaptcha, Cloudflare, Arkose) so bot challenges still work. Video and audio are also disabled entirely. Some media may get through. There are many different ways media ends up on a web page.

The user agent is set to Safari on macOS. This avoids fingerprinting issues and lets the browser pass Cloudflare checks.

The URL bar distinguishes URLs from search queries. Anything that isn't a navigable address becomes a Brave search query. Third-party cookies are blocked.

## Dependencies

- GTK4
- Libadwaita 1
- WebKitGTK 6.0
- Ada (URL parsing)
- Libpsl (public suffix list)
- Blueprint compiler
- Meson

## Building

```sh
make          # release build
make debug    # debug build
make install  # install system-wide (requires sudo)
make clean    # remove build artifacts
```

The Makefile wraps Meson. Build output goes to `builddir/`.

## License

Copyright (C) 2026 Robert Rapier. See CC0 1.0 Universal.
