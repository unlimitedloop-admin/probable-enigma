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
