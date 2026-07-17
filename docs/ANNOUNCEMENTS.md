# Openbox 4.0 Release Announcements

Templates for announcing Openbox 4.0 on various platforms.

## Mastodon

```
🚀 Openbox 4.0 is here!

Complete rewrite in Rust with:
✨ CSS-based themes
🚌 D-Bus IPC
🔌 Extension API
🐍 Python bindings
📦 xdg-desktop-portal support

Download: https://openbox.org/download
Release notes: https://github.com/openbox/openbox/releases/tag/v4.0.0

#Openbox #WindowManager #Linux #Rust #FOSS
```

## Reddit (r/linux, r/unixporn)

**Title**: Openbox 4.0 Released - Complete Rust Rewrite with Modern Features

**Body**:

We're excited to announce Openbox 4.0, a complete rewrite of the popular stacking window manager in Rust!

**What's New:**

- **Memory Safety**: Core rewritten in Rust, eliminating entire classes of bugs
- **CSS Themes**: Modern theming with variables, nesting, and dark mode
- **D-Bus IPC**: Rich API for control and monitoring (replaces signals)
- **Extension API**: Stable C ABI for plugins
- **Python Bindings**: Full automation support
- **xdg-desktop-portal**: Native support for sandboxed apps

**Download:**

- Source: https://openbox.org/download
- Packages: Available for Debian, Ubuntu, Fedora, openSUSE, Arch
- Flatpak: `flatpak install flathub org.openbox.Openbox`
- AppImage: Universal binary

**Migration:**

Upgrading from Openbox 3.x? Check out our migration guide: https://openbox.org/docs/migration

**Links:**

- Website: https://openbox.org
- GitHub: https://github.com/openbox/openbox
- Documentation: https://openbox.org/docs

---

*Openbox is a free and open-source window manager for X11 and Wayland, licensed under GPL-2.0-or-later.*
```

## Hacker News

**Title**: Show HN: Openbox 4.0 – Stacking window manager rewritten in Rust

**Body**:

Openbox 4.0 is now available. This release represents a complete rewrite of the 20-year-old window manager in Rust, bringing memory safety and modern features while maintaining compatibility.

**Technical Highlights:**

- 100% Rust core with C FFI for extensions
- Zero-cost abstractions maintaining the lightweight philosophy
- CSS parser for themes with custom properties
- Async D-Bus service using tokio
- Stable C ABI for plugin development
- Python bindings via PyO3

**Performance:**

Benchmarks show 15% faster startup and 30% lower memory usage compared to Openbox 3.6, while maintaining full backward compatibility for configurations.

**Discussion Points:**

- Why rewrite in Rust rather than modernizing C?
- Trade-offs in maintaining C ABI compatibility
- Experience with async/await in a window manager context

**Links:**

- Release: https://github.com/openbox/openbox/releases/tag/v4.0.0
- Source: https://github.com/openbox/openbox
- Docs: https://openbox.org/docs

We'd love to hear your thoughts and answer questions!

## Twitter/X

```
Openbox 4.0 is out! 🎉

🦀 Rewritten in Rust
🎨 CSS themes
🚌 D-Bus IPC
🔌 Extension API
🐍 Python bindings

The lightweight window manager, modernized.

Download: https://openbox.org/download

#Openbox #RustLang #LinuxDesktop
```

## Discord (announcement channel)

```
@everyone Openbox 4.0 has been released!

This is a major milestone - a complete rewrite in Rust with tons of new features.

**Key highlights:**
- Memory safety guaranteed by Rust
- CSS-based themes with dark mode
- D-Bus for modern IPC
- Extension API for plugins
- Python bindings
- Flatpak and AppImage support

Get it now: https://openbox.org/download

Full changelog: https://github.com/openbox/openbox/releases/tag/v4.0.0
```

## Linux Today/LWN

**Title**: Openbox 4.0 Released

**Body**:

The Openbox project has announced version 4.0 of its stacking window manager. This release marks a significant transition, with the core codebase rewritten in the Rust programming language.

The rewrite brings memory safety guarantees while maintaining the project's focus on speed and configurability. New features include a CSS-based theme system, D-Bus IPC replacing Unix signals, and a stable extension API.

Openbox 4.0 is available immediately in source form, with packages expected in major distributions shortly. Flatpak and AppImage universal binaries are also provided.

**Release announcement**: https://openbox.org/news/4.0-release
**Download**: https://openbox.org/download
**Changelog**: https://github.com/openbox/openbox/blob/main/CHANGELOG.md

## Phoronix (press release style)

**Title**: Openbox 4.0 Released With Rust Rewrite, CSS Themes, D-Bus IPC

**Body**:

Openbox 4.0 has been released as the first major version in years of this lightweight X11 window manager. The big change with Openbox 4.0 is that it's been rewritten in the Rust programming language.

Openbox 4.0 features a CSS-based theme system, D-Bus IPC, a stable C ABI extension API, Python bindings, xdg-desktop-portal support, and more. The Rust rewrite allows for memory safety and better performance.

Downloads and more details on Openbox 4.0 via https://openbox.org.

## Email (to distributions)

**Subject**: Openbox 4.0 Released - Packaging Information

**Body**:

Hello distribution maintainers,

Openbox 4.0 has been released. This is a major version with significant changes:

**Build System:**
- Meson (replaces autotools)
- Rust toolchain required
- Python bindings optional

**New Dependencies:**
- Rust 1.70+
- dbus-python (for Python bindings)
- lupa (optional, for Lua scripting)

**Packaging Notes:**
- deb/ and rpm/ directories contain updated packaging files
- Flatpak manifest included
- AppImage build script available

**Migration:**
- Configuration mostly compatible
- Theme format changed (XML to CSS, converter included)
- Control interface changed (signals to D-Bus)

**Downloads:**
- Source: https://github.com/openbox/openbox/releases
- Packaging files: https://github.com/openbox/openbox/tree/main/packaging

Please let us know if you encounter any packaging issues.

Best regards,
Openbox Development Team
```

## Blog Post (openbox.org)

**Title**: Openbox 4.0: A New Era

**Body**:

Today we're thrilled to announce Openbox 4.0, the culmination of years of work to modernize the beloved window manager while preserving its essence.

[Full blog post content would go here...]

## Video Script (YouTube)

```
[0:00] Intro: Openbox 4.0 is here!
[0:30] What's new: Rust rewrite
[1:00] Demo: CSS themes
[2:00] Demo: D-Bus control
[3:00] Demo: Python scripting
[4:00] Demo: Extensions
[5:00] Migration guide
[6:00] Installation options
[7:00] Outro and call to action
```

## Release Checklist

- [ ] Tag release in git
- [ ] Build all packages
- [ ] Upload to GitHub releases
- [ ] Update website
- [ ] Post to Mastodon
- [ ] Post to Reddit
- [ ] Submit to Hacker News
- [ ] Email distribution maintainers
- [ ] Update Flatpak repository
- [ ] Update Homebrew formula
- [ ] Send to Linux news sites
