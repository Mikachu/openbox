# Contributing to Openbox

Thank you for your interest in contributing to Openbox! This guide will help you get started.

## Code of Conduct

We are committed to providing a friendly, safe and welcoming environment for all.

- Be respectful and inclusive
- Welcome newcomers
- Focus on constructive feedback
- Respect different viewpoints and experiences

## Getting Started

### Prerequisites

- Git
- Rust 1.70+ (for core)
- C compiler (for legacy components)
- Meson and Ninja
- pkg-config

### Development Setup

```bash
# Clone the repository
git clone https://github.com/openbox/openbox.git
cd openbox

# Install dependencies (Ubuntu/Debian)
sudo apt-get install -y \
    build-essential meson ninja-build \
    libxml2-dev libglib2.0-dev libcairo2-dev \
    libpango1.0-dev libx11-dev libxinerama-dev \
    libxrandr-dev libxft-dev

# Build
meson setup build
ninja -C build

# Run tests
meson test -C build
```

## Contribution Workflow

### 1. Find or Create an Issue

- Check [existing issues](https://github.com/openbox/openbox/issues)
- Comment on an issue to express interest
- Or create a new issue for bugs/features

### 2. Fork and Branch

```bash
# Fork on GitHub, then:
git clone https://github.com/YOUR-USERNAME/openbox.git
git checkout -b feature/your-feature-name
```

Use descriptive branch names:
- `feature/description` - New features
- `fix/description` - Bug fixes
- `docs/description` - Documentation
- `refactor/description` - Code refactoring

### 3. Make Changes

- Follow our [Code Style Guide](code-style.md)
- Add tests for new functionality
- Update documentation
- Keep commits focused and atomic

### 4. Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation
- `style`: Formatting
- `refactor`: Code restructuring
- `test`: Tests
- `chore`: Maintenance

Example:
```
feat(wm): add window snapping to screen edges

Implements magnetic edges for easier window management.
Closes #123
```

### 5. Test

```bash
# Run all tests
./scripts/run-tests.sh

# Run specific test suites
pytest tests/python/ -v
cargo test --workspace
meson test -C build
```

### 6. Submit Pull Request

1. Push to your fork
2. Open PR against `main` branch
3. Fill out PR template
4. Link related issues
5. Request review from maintainers

## PR Review Process

1. **Automated checks** must pass:
   - CI builds (GCC, Clang)
   - Tests
   - Static analysis
   - Code coverage

2. **Code review** by at least one maintainer

3. **Merge** once approved and CI passes

## Areas for Contribution

### Good First Issues

Look for issues labeled `good-first-issue`:
- Documentation improvements
- Test coverage
- Small bug fixes
- Translation updates

### Feature Development

Check the [roadmap](https://github.com/openbox/openbox/projects) for planned features.

### Documentation

- User guides
- API documentation
- Translation
- Video tutorials

## Coding Standards

See [Code Style Guide](code-style.md) for detailed standards.

Quick reference:
- **Rust**: `cargo fmt`, `cargo clippy`
- **C**: Follow existing style, run `clang-format`
- **Documentation**: Markdown, clear and concise

## Committer Guidelines

For maintainers with merge access:

### Merging PRs

1. Ensure CI passes
2. Code review completed
3. Squash commits if messy history
4. Use merge commit for feature PRs
5. Update CHANGELOG.md

### Release Process

1. Update version in `meson.build`
2. Update CHANGELOG.md
3. Tag release: `git tag -a v4.0.0 -m "Release 4.0.0"`
4. Push tags: `git push origin v4.0.0`
5. CI creates release automatically

## Communication

- **GitHub Issues**: Bug reports, feature requests
- **GitHub Discussions**: General questions
- **Discord**: Real-time chat (invite link in README)
- **Mailing List**: openbox@lists.openbox.org

## License

By contributing, you agree that your contributions will be licensed under GPL-2.0-or-later.

## Questions?

- Check [FAQ](../user-guide/faq.md)
- Ask in [GitHub Discussions](https://github.com/openbox/openbox/discussions)
- Join our [Discord](https://discord.gg/openbox)

Thank you for contributing to Openbox!
