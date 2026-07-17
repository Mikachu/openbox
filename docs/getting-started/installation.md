# Installing Openbox

This guide covers installing Openbox 4 on various Linux distributions.

## System Requirements

- **Operating System**: Linux with X11 or Wayland
- **Memory**: 256 MB RAM minimum (512 MB recommended)
- **Storage**: 50 MB for Openbox, 200 MB with dependencies
- **Display**: Any X11 or Wayland compatible display

## Quick Install

### From Package Manager (Recommended)

=== \"Ubuntu/Debian\"\n\n    ```bash\n    sudo apt update\n    sudo apt install openbox4\n    ```\n\n=== \"Fedora\"\n\n    ```bash\n    sudo dnf install openbox4\n    ```\n\n=== \"Arch Linux\"\n\n    ```bash\n    sudo pacman -S openbox4\n    ```\n\n=== \"openSUSE\"\n\n    ```bash\n    sudo zypper install openbox4\n    ```\n\n## Building from Source\n\n### Dependencies\n\n=== \"Ubuntu/Debian\"\n\n    ```bash\n    sudo apt install -y \\\n        build-essential meson ninja-build \\\n        libxml2-dev libglib2.0-dev libcairo2-dev \\\n        libpango1.0-dev libx11-dev libxinerama-dev \\\n        libxrandr-dev libxft-dev libstartup-notification0-dev\n    ```\n\n=== \"Fedora\"\n\n    ```bash\n    sudo dnf install -y \\\n        gcc meson ninja-build \\\n        libxml2-devel glib2-devel cairo-devel \\\n        pango-devel libX11-devel libXinerama-devel \\\n        libXrandr-devel libXft-devel \\\n        startup-notification-devel\n    ```\n\n=== \"Arch Linux\"\n\n    ```bash\n    sudo pacman -S --needed \\\n        base-devel meson \\\n        libxml2 glib2 cairo pango \\\n        libx11 libxinerama libxrandr libxft \\\n        startup-notification\n    ```\n\n### Build Steps\n\n1. **Clone the repository**:\n   ```bash\n   git clone https://github.com/openbox/openbox.git\n   cd openbox\n   ```\n\n2. **Configure the build**:\n   ```bash\n   meson setup build --prefix=/usr\n   ```\n\n3. **Compile**:\n   ```bash\n   ninja -C build\n   ```\n\n4. **Install**:\n   ```bash\n   sudo ninja -C build install\n   ```\n\n## Post-Installation\n\n### Create Configuration Directory\n\n```bash\nmkdir -p ~/.config/openbox\n```\n\n### Copy Default Configuration\n\n```bash\ncp /etc/xdg/openbox/rc.xml ~/.config/openbox/\ncp /etc/xdg/openbox/menu.xml ~/.config/openbox/\n```\n\n### Starting Openbox\n\nFrom a display manager:\n- Select \"Openbox\" from the session menu\n\nFrom the command line:\n```bash\nexec openbox\n```\n\n## Verification\n\nCheck installation:\n```bash\nopenbox --version\n```\n\nExpected output:\n```\nOpenbox 4.0.0\nCopyright (c) 2002-2024 Openbox Development Team\n```\n\n## Troubleshooting\n\n### Build Errors\n\n**Missing dependencies**: Install all development packages listed above.\n\n**Rust not found**: Install Rust from [rustup.rs](https://rustup.rs/).\n\n### Runtime Issues\n\n**No display**: Ensure `$DISPLAY` is set.\n\n**Permission denied**: Check file permissions on executables.\n\n## Next Steps\n\n- [Quick Start Guide](quickstart.md)\n- [Configuration Guide](../user-guide/configuration.md)\n- [Theming Guide](../user-guide/theming.md)\n