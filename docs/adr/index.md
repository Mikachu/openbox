# Architecture Decision Records (ADRs)

This section contains records of important architectural decisions made in Openbox 4. Each ADR describes the context, decision, and consequences of a significant change.

## What are ADRs?

Architecture Decision Records (ADRs) capture important architectural decisions made along with their context and consequences. They serve as:

- **Historical record** - Why we made certain choices
- **Onboarding aid** - New developers understand the codebase
- **Decision framework** - Future decisions can build on past reasoning

## ADR Index

| ADR | Title | Status | Date |
|-----|-------|--------|------|
| [ADR-001](001-rust-rewrite.md) | Rewrite Core in Rust | Accepted | 2024-01-15 |
| [ADR-002](002-dbus-ipc.md) | Replace Signals with D-Bus | Accepted | 2024-01-20 |
| [ADR-003](003-css-theming.md) | CSS-Based Theme System | Accepted | 2024-02-01 |
| [ADR-004](004-wayland-support.md) | Native Wayland Backend | Proposed | 2024-02-15 |
| [ADR-005](005-async-event-loop.md) | Async/Tokio Event Loop | Accepted | 2024-03-01 |

## Status Meanings

- **Proposed** - Under discussion, not yet decided
- **Accepted** - Decision made, being implemented
- **Deprecated** - Decision reversed or superseded
- **Superseded** - Replaced by a newer ADR

## Contributing ADRs

To propose a new ADR:

1. Copy `adr/template.md` to `adr/XXX-short-name.md`
2. Fill in all sections
3. Submit as PR for discussion
4. ADR number assigned on merge

See [Contributing Guide](../dev-guide/contributing.md) for details.
