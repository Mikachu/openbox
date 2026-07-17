# Creating Openbox Themes

This tutorial walks you through creating a complete Openbox theme using the new CSS-based theme system.

## Prerequisites

- Openbox 4.0 or later installed
- A text editor
- Basic knowledge of CSS

## Theme Structure

A theme consists of:

```
my-theme/
├── theme.css          # Main theme file
├── metadata.json      # Theme metadata
├── close.svg          # Window button icons
├── maximize.svg
├── minimize.svg
└── icon-48x48.png     # Theme preview icon
```

## Step 1: Create Theme Directory

```bash
mkdir -p ~/.local/share/themes/MyTheme/openbox-4
cd ~/.local/share/themes/MyTheme/openbox-4
```

## Step 2: Theme Metadata

Create `metadata.json`:

```json
{
  \"name\": \"MyTheme\",
  \"author\": \"Your Name\",
  \"version\": \"1.0.0\",
  \"description\": \"A modern dark theme for Openbox\",
  \"license\": \"GPL-2.0-or-later\",
  \"variant\": \"dark\"
}
```

## Step 3: Basic Theme CSS

Create `theme.css`:

```css
/* Theme variables */
:root {
  --bg-primary: #2d2d2d;
  --bg-secondary: #3d3d3d;
  --fg-primary: #e8e8e8;
  --fg-secondary: #a0a0a0;
  --accent: #4a9eff;
  --border: #1a1a1a;
  --radius: 4px;
}

/* Window styling */
window {
  border: 1px solid var(--border);
  border-radius: var(--radius);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
}

/* Active window */
window.active {
  background: var(--bg-primary);
  color: var(--fg-primary);
}

/* Inactive window */
window.inactive {
  background: var(--bg-secondary);
  color: var(--fg-secondary);
  opacity: 0.95;
}

/* Titlebar */
titlebar {
  height: 32px;
  padding: 0 8px;
  font-family: \"Sans\", sans-serif;
  font-size: 14px;
  font-weight: 500;
}

window.active titlebar {
  background: linear-gradient(to bottom, #3d3d3d, #2d2d2d);
}

window.inactive titlebar {
  background: var(--bg-secondary);
}

/* Window buttons */
button {
  size: 20px 20px;
  margin: 6px 2px;
  border-radius: 3px;
  transition: background 150ms ease;
}

button:hover {
  background: rgba(255, 255, 255, 0.1);
}

button:active {
  background: rgba(255, 255, 255, 0.2);
}

button.close {
  background: url(\"close.svg\") center no-repeat;
}

button.close:hover {
  background-color: #e74c3c;
}

button.maximize {
  background: url(\"maximize.svg\") center no-repeat;
}

button.minimize {
  background: url(\"minimize.svg\") center no-repeat;
}
```

## Step 4: Add Window Icons

Create simple SVG icons. Example `close.svg`:

```svg
<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"20\" height=\"20\" viewBox=\"0 0 20 20\">
  <path fill=\"#e8e8e8\" d=\"M6 6l8 8M6 14l8-8\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\"/>
</svg>
```

## Step 5: Dark Mode Support

Add automatic dark mode detection:

```css
/* Default (dark) theme */
:root {
  --bg-primary: #2d2d2d;
  --bg-secondary: #3d3d3d;
  --fg-primary: #e8e8e8;
}

/* Light mode override */
@media (prefers-color-scheme: light) {
  :root {
    --bg-primary: #f5f5f5;
    --bg-secondary: #e0e0e0;
    --fg-primary: #333333;
    --border: #cccccc;
  }
  
  window {
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
  }
}
```

## Step 6: Advanced Styling

### Window Types

```css
/* Dialog windows */
window[type=\"dialog\"] {
  border-radius: 8px;
}

/* Modal dialogs */
window[type=\"dialog\"].modal {
  box-shadow: 0 0 0 9999px rgba(0, 0, 0, 0.5);
}

/* Utility windows (palettes, toolboxes) */
window[type=\"utility\"] {
  border: 1px solid var(--border);
  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.2);
}

/* Splash screens */
window[type=\"splash\"] {
  border-radius: 12px;
  border: none;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
}
```

### Window States

```css
/* Maximized windows - no shadow, no border-radius */
window.maximized {
  border-radius: 0;
  box-shadow: none;
  border: none;
}

/* Fullscreen - completely undecorated */
window.fullscreen {
  border: none;
  box-shadow: none;
}

/* Shaded (rolled up) */
window.shaded {
  border-radius: var(--radius) var(--radius) 0 0;
}

/* Sticky (on all desktops) */
window.sticky titlebar::before {
  content: \"📌\";
  margin-right: 6px;
  font-size: 12px;
}
```

### Animations

```css
/* Smooth transitions */
window {
  transition: opacity 200ms ease, transform 200ms ease;
}

window.minimized {
  opacity: 0;
  transform: scale(0.9);
}

/* Button hover animation */
button {
  transition: transform 150ms ease, background 150ms ease;
}

button:hover {
  transform: scale(1.1);
}
```

## Step 7: Testing Your Theme

1. **Activate the theme**:
   ```bash
   obctl theme set MyTheme
   ```

2. **Reload without restarting**:
   ```bash
   obctl theme reload
   ```

3. **Test with different window types**:
   - Open various applications
   - Test dialogs, utilities, splash screens
   - Check both active and inactive states

## Step 8: Distribution

Package your theme for distribution:

```bash
cd ~/.local/share/themes
tar czf MyTheme-v1.0.0.tar.gz MyTheme/
```

Users install with:
```bash
tar xzf MyTheme-v1.0.0.tar.gz -C ~/.local/share/themes/
```

## Best Practices

### Performance

- Use CSS variables for reusable values
- Minimize complex gradients
- Use SVG for scalable icons
- Avoid heavy box-shadows on many windows

### Accessibility

- Ensure sufficient contrast (WCAG AA minimum)
- Make buttons large enough to click (20x20px minimum)
- Test with high contrast mode
- Support keyboard navigation

### Compatibility

- Test on different screen densities
- Verify RTL (right-to-left) language support
- Check with various GTK/Qt themes

## Common Patterns

### Glass/Transparent Effect

```css
window.active {
  background: rgba(45, 45, 45, 0.95);
  backdrop-filter: blur(10px);
}
```

### Border-Only Theme

```css
window {
  border: 2px solid var(--accent);
  background: transparent;
}

titlebar {
  background: var(--bg-primary);
}
```

### Minimal Theme

```css
window {
  border: none;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
}

titlebar {
  height: 28px;
  background: transparent;
}

button {
  opacity: 0;
  transition: opacity 200ms;
}

window:hover button {
  opacity: 1;
}
```

## Troubleshooting

### Theme not showing

- Check file permissions: `chmod 644 theme.css`
- Verify theme directory structure
- Check Openbox logs: `journalctl -u openbox`

### Icons not loading

- Verify SVG syntax
- Check file paths in CSS
- Ensure icons are in theme directory

### Performance issues

- Reduce shadow complexity
- Disable animations if needed
- Use simpler gradients

## Resources

- [Theme Gallery](https://openbox.org/themes)
- [CSS Reference](../reference/theme-spec.md)
- [Video Tutorial: Theme Creation](videos.md)

## Next Steps

- Share your theme on [Openbox Themes](https://openbox.org/themes)
- Submit to distribution repositories
- Join the community for feedback
