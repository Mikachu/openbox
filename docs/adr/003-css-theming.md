# ADR-003: CSS-Based Theme System

## Status

**Accepted** - 2024-02-01

## Context

Openbox 3.x themes use XML-based format with custom syntax:

```xml
<theme>
  <window>
    <active>
      <titlebar>
        <bg>solid</bg>
        <color>#4c4c4c</color>
      </titlebar>
    </active>
  </window>
</theme>
```

Problems with this approach:

1. **Verbose**: XML is verbose for styling
2. **Limited features**: No variables, nesting, or calculations
3. **No tooling**: No syntax highlighting, linters, or preview tools
4. **Learning curve**: Custom syntax requires documentation
5. **No dark mode**: No automatic theme variants

## Decision

We will **adopt CSS for theming** with a custom CSS parser and properties:

```css
/* theme.css */
@theme-name "Nightfall";

window {
  --titlebar-bg: #2d2d2d;
  --titlebar-fg: #e8e8e8;
}

window.active {
  background: var(--titlebar-bg);
  color: var(--titlebar-fg);
}

window.inactive {
  opacity: 0.8;
}

button.close {
  background: url("close.svg");
  size: 16px 16px;
}
```

### Features

- **CSS Variables**: Theme-wide custom properties
- **Nesting**: Hierarchical selectors
- **Media queries**: `prefers-color-scheme` for dark mode
- **Import**: Modular themes with `@import`
- **Gradients**: Modern gradient support

## Consequences

### Positive

- **Familiar**: Millions of developers know CSS
- **Tooling**: Existing editors, linters, formatters
- **Expressive**: More powerful than XML
- **Maintainable**: Variables reduce duplication
- **Accessible**: Dark mode built-in

### Negative

- **Parser complexity**: Need robust CSS parser
- **Breaking change**: Old themes need conversion
- **Performance**: CSS parsing overhead

### Migration

Provide `ob-theme-convert` tool to transform XML themes to CSS.

## Alternatives Considered

### Option A: Enhanced XML
Add variables and includes to existing XML format.

- **Rejected**: Still custom syntax, limited ecosystem

### Option B: JSON
JSON-based theme format.

- **Rejected**: No comments, no nesting, verbose

### Option C: TOML
TOML configuration for themes.

- **Rejected**: Good for data, awkward for styling

## Implementation

See [Theme Specification](../reference/theme-spec.md) and [Theme Tutorial](../tutorials/theme-creation.md).

## References

- [CSS Variables](https://developer.mozilla.org/en-US/docs/Web/CSS/--*)
- [CSS Nesting](https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_nesting)
