# CSS Themes for Openbox 4

## Overview

Openbox 4 introduces a modern CSS-based theming system that replaces the legacy `themerc` format. CSS themes provide:

- **Familiar syntax**: Standard CSS3 properties and selectors
- **Variables**: CSS custom properties for easy customization
- **Dark mode**: Automatic switching via `prefers-color-scheme`
- **Cascade**: Inheritance and specificity for complex themes
- **Animations**: CSS animations for window transitions
- **Live reload**: Changes apply without restart

## Quick Start

### Creating a New Theme

```bash
mkdir -p ~/.themes/MyTheme/openbox-4
cat > ~/.themes/MyTheme/openbox-4/theme.css << 'EOF'
:root {
  --bg: #2d2d2d;
  --fg: #e0e0e0;
  --accent: #5e81ac;
}

window {
  background: var(--bg);
  border: 1px solid var(--accent);
}

titlebar {
  background: var(--bg);
  height: 30px;
}

title {
  color: var(--fg);
  font-family: sans-serif;
  font-size: 10pt;
}
EOF
```

### Converting Legacy Themes

```bash
# Convert themerc to CSS
openbox-themerc2css ~/.themes/OldTheme/openbox-3/themerc -o theme.css

# With dark mode support
openbox-themerc2css ~/.themes/OldTheme/openbox-3/themerc -o theme.css --dark
```

## CSS Structure

### Selectors

Openbox uses semantic selectors for window elements:

| Selector | Description |
|----------|-------------|
| `window` | Window frame |
| `window:focus` | Focused window |
| `window:unfocused` | Unfocused window |
| `window:maximized` | Maximized window |
| `window:shaded` | Shaded/rolled window |
| `titlebar` | Window titlebar |
| `title` | Window title text |
| `button` | All window buttons |
| `button.close` | Close button |
| `button.maximize` | Maximize button |
| `button.iconify` | Iconify button |
| `button.shade` | Shade button |
| `button.desk` | Desktop button |
| `grip` | Resize grip |
| `grip:corner` | Corner resize grip |
| `handle` | Bottom resize handle |
| `menu` | Context menu |
| `menu-title` | Menu title/header |
| `menu-item` | Menu item |
| `menu-item:hover` | Selected menu item |
| `menu-separator` | Menu separator |
| `osd` | On-screen display |

### Properties

#### Window Properties

```css
window {
  /* Background */
  background: #2d2d2d;
  background: linear-gradient(to bottom, #3d3d3d, #2d2d2d);
  
  /* Border */
  border: 1px solid #5e81ac;
  border-radius: 4px 4px 0 0;
  
  /* Shadow */
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
}
```

#### Titlebar Properties

```css
titlebar {
  /* Size */
  height: 28px;
  padding: 4px 8px;
  
  /* Background */
  background: #3d3d3d;
  background: linear-gradient(to bottom, #4d4d4d, #3d3d3d);
  
  /* Border */
  border-bottom: 1px solid #2d2d2d;
}
```

#### Title Properties

```css
title {
  /* Text */
  color: #e0e0e0;
  font-family: "DejaVu Sans", sans-serif;
  font-size: 9pt;
  font-weight: bold;
  text-align: center; /* left, center, right */
  
  /* Shadow */
  text-shadow: 0 1px 2px rgba(0, 0, 0, 0.5);
}
```

#### Button Properties

```css
button {
  /* Size */
  size: 18px;
  margin: 0 2px;
  
  /* Appearance */
  border-radius: 3px;
  background: transparent;
  border: 1px solid transparent;
}

button:hover {
  background: #4d4d4d;
  border-color: #5e81ac;
}

button:active {
  background: #2d2d2d;
}

/* Per-button styling */
button.close:hover {
  background: #bf616a;
}
button.maximize:hover {
  background: #a3be8c;
}
button.iconify:hover {
  background: #ebcb8b;
}
```

#### Menu Properties

```css
menu {
  background: #2d2d2d;
  border: 1px solid #4d4d4d;
  border-radius: 4px;
  padding: 4px 0;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.4);
}

menu-item {
  height: 24px;
  padding: 0 16px;
  color: #e0e0e0;
  font-size: 9pt;
}

menu-item:hover {
  background: #5e81ac;
  color: #ffffff;
}

menu-separator {
  height: 1px;
  margin: 4px 8px;
  background: #4d4d4d;
}
```

## CSS Variables

Define reusable values with CSS custom properties:

```css
:root {
  /* Colors */
  --bg-primary: #2d2d2d;
  --bg-secondary: #3d3d3d;
  --accent: #5e81ac;
  
  /* Typography */
  --font-main: "DejaVu Sans", sans-serif;
  --font-mono: "JetBrains Mono", monospace;
  
  /* Spacing */
  --space-sm: 4px;
  --space-md: 8px;
  --space-lg: 16px;
  
  /* Border radius */
  --radius: 4px;
}

/* Use variables */
window {
  background: var(--bg-primary);
  border-radius: var(--radius);
}

title {
  font-family: var(--font-main);
}
```

## Dark Mode

Support automatic dark/light switching:

```css
:root {
  /* Light mode defaults */
  --bg: #ffffff;
  --fg: #000000;
}

@media (prefers-color-scheme: dark) {
  :root {
    --bg: #2d2d2d;
    --fg: #e0e0e0;
  }
}
```

Toggle via settings or environment:

```bash
# Force dark mode
OPENBOX_THEME_MODE=dark openbox

# Force light mode
OPENBOX_THEME_MODE=light openbox
```

## Animations

Add smooth transitions:

```css
@keyframes scale-in {
  from { transform: scale(0.95); opacity: 0; }
  to { transform: scale(1); opacity: 1; }
}

window.mapped {
  animation: scale-in 200ms ease-out;
}

button {
  transition: background 150ms ease;
}
```

## Property Reference

### Colors

| Property | Description | Example |
|----------|-------------|---------|
| `background` | Background color/gradient | `#2d2d2d` |
| `color` | Text color | `#e0e0e0` |
| `border-color` | Border color | `#5e81ac` |
| `box-shadow` | Shadow effect | `0 2px 4px rgba(0,0,0,0.3)` |

### Dimensions

| Property | Description | Example |
|----------|-------------|---------|
| `width`, `height` | Size | `28px` |
| `padding` | Internal spacing | `4px 8px` |
| `margin` | External spacing | `0 2px` |
| `border-width` | Border thickness | `1px` |
| `border-radius` | Corner rounding | `4px` |

### Typography

| Property | Description | Example |
|----------|-------------|---------|
| `font-family` | Font name | `"DejaVu Sans"` |
| `font-size` | Text size | `9pt`, `12px` |
| `font-weight` | Text weight | `normal`, `bold` |
| `text-align` | Text alignment | `left`, `center`, `right` |
| `text-shadow` | Text shadow | `0 1px 2px black` |

## Theme Locations

Openbox searches for themes in order:

1. `~/.themes/Name/openbox-4/theme.css`
2. `~/.local/share/themes/Name/openbox-4/theme.css`
3. `/usr/share/themes/Name/openbox-4/theme.css`
4. `/usr/local/share/themes/Name/openbox-4/theme.css`

## Migration from Themerc

### Mapping Reference

| Themerc | CSS |
|---------|-----|
| `window.active.border.color` | `window:focus { border-color: ... }` |
| `window.active.title.bg` | `titlebar { background: ... }` |
| `window.active.label.text.color` | `title { color: ... }` |
| `window.active.button.unpressed.bg` | `button { background: ... }` |
| `padding.width` | `titlebar { padding: ... }` |
| `window.label.height` | `titlebar { height: ... }` |
| `border.width` | `window { border-width: ... }` |
| `menu.items.bg` | `menu { background: ... }` |
| `menu.items.active.bg` | `menu-item:hover { background: ... }` |

## Tools

### `openbox-themerc2css`

Converts legacy themerc files to CSS:

```bash
# Basic conversion
openbox-themerc2css themerc -o theme.css

# With theme name
openbox-themerc2css themerc -o theme.css -n "My Theme"

# With dark mode
openbox-themerc2css themerc -o theme.css --dark

# Verbose output
openbox-themerc2css themerc -o theme.css -v
```

## Examples

See the included example themes:

- **Clearlooks** - Classic light theme with gradients
- **Nightfall** - Modern dark theme with accent colors
- **Minimal** - Clean, distraction-free design

Located in `/usr/share/openbox/themes/`.

## Debugging

Enable CSS debug output:

```bash
OPENBOX_CSS_DEBUG=1 openbox
```

This prints parsed CSS rules and computed values to stderr.

## Best Practices

1. **Use variables** for colors to enable easy customization
2. **Support dark mode** with `prefers-color-scheme`
3. **Test at different DPI** using relative units (`em`, `rem`)
4. **Keep animations subtle** - avoid motion sickness
5. **Validate CSS** with `openbox --validate-theme`
6. **Use fallbacks** for older Openbox versions

## Compatibility

- Openbox 4.0+: Full CSS theme support
- Openbox 3.x: Legacy themerc format (auto-detected)
- GTK themes: Use `gtk-theme` property for integration

## See Also

- [Openbox Wiki](https://openbox.org/wiki/)
- [CSS3 Specification](https://www.w3.org/TR/css-syntax-3/)
- [CSS Variables](https://developer.mozilla.org/en-US/docs/Web/CSS/Using_CSS_custom_properties)
