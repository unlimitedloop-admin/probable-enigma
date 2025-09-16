# Git Branching Guidelines

This project follows a structured branching model inspired by Git Flow.  
Please follow the conventions below when creating or managing branches.

---

## 🔧 Branch Types and Naming Conventions

| Type         | Prefix        | Example                           | Description                          |
|--------------|---------------|------------------------------------|--------------------------------------|
| Feature      | `feature/`    | `feature/scene-manager`            | New feature implementations          |
| Bugfix       | `fix/`        | `fix/input-deadzone-bug`           | Bug and defect fixes                 |
| Refactoring  | `refactor/`   | `refactor/fade-controller`         | Code restructuring or cleanup        |
| Documentation| `docs/`       | `docs/readme-update`               | Documentation and code comments      |
| Experiment   | `experiment/` | `experiment/hud-overlay-test`      | Experimental code or PoC branches    |
| Release      | `release/`    | `release/v1.0.0-beta`              | Pre-release integration branches     |
| Hotfix       | `hotfix/`     | `hotfix/crash-on-exit`             | Critical production hotfixes         |

---

## 🛠️ Rules

- Never commit directly to `master` or `develop`.
- Always create a new branch from the latest `develop`.
- Use lowercase letters and `-` as delimiters within the branch name.
- Keep branch names short and descriptive.
- Avoid using personal names in branch names (use purpose-based identifiers instead).
- Pull Requests must include a clear description of the purpose and scope of changes.

---

## ✅ Examples

- `feature/palette-fade-support`
- `fix/gamepad-deadzone`
- `refactor/scene-loader`
- `docs/coding-guidelines`
- `experiment/sprite-scaling`

---

## 🔄 Merge Flow

1. Feature branches → `develop`
2. Release branches → `master` + tag version (e.g., `v1.0.0`)
3. Hotfix branches → `master`, then merge back into `develop`

---

## 📌 Tip

Consider using the following commit message format:

```
[<module>] <action> <description>
```

Examples:

- `[core] Implemented sequencer branching logic`
- `[apps] Refactored scene component state transitions`
- `[input] Converted input controller to singleton`


# Coding Standards and Guidelines
This document outlines the coding standards and guidelines for the RetroEngine project. Adhering to these standards ensures code consistency, readability, and maintainability across the project.

## General Guidelines
- Follow the established folder structure.
- Keep code DRY (Don't Repeat Yourself).
- Write clear and descriptive commit messages.
- Use meaningful names for variables, functions, and classes.
- Include comments and documentation where necessary.

## Code Style
- Use 4 spaces for indentation (no tabs).
- Limit line length to 80 characters.
- Use camelCase for variable and function names.
- Use PascalCase for class names.
- Place opening braces on the same line as the declaration.

## Documentation
- Update documentation for any public API changes.
- Include usage examples in the documentation.
- Keep README files up to date with the latest information.

## Coding Guidelines
- Reference the coding guidelines in the `docs/【ゲーム開発】romhacking_PG-001_コーディング規約.md` file for detailed coding practices. (in Japanese)
