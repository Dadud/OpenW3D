# Contributing to OpenW3D

## Before You Start

OpenW3D is a community-maintained fork. All contributors are expected to follow the project's norms. If you're unsure whether a change is welcome, open an Issue to discuss it first.

## Workflow

### 1. Fork and Branch

Fork the repository on GitHub, then clone your fork:

```bash
git clone https://github.com/YOURNAME/OpenW3D.git
cd OpenW3D
git submodule update --init --recursive
```

Create a branch per logical change:

```bash
git switch -c fix/my-fix
git switch -c feat/my-feature
git switch -c refactor/area-improve
```

**Branch naming conventions:**
- `fix/` — bug fixes
- `feat/` — new features
- `refactor/` — code improvement without behavior change
- `docs/` — documentation only
- `ci/` — CI/CD changes

### 2. Make Small, Focused Changes

One PR per logical change. "Everything I touched over the weekend" PRs are hard to review and hard to revert.

**Good:** "Add clock_gettime fallback for Get_CPU_Rate on Linux"  
**Bad:** "Fix Linux build and also refactor audio initialization"

If your work naturally spans multiple areas, break it into multiple PRs or open an Issue to discuss the split.

### 3. Test In-Game

CI passing means the code compiles — not that it runs correctly. For rendering, networking, or gameplay changes, you must test in-game (or on a LAN server) before requesting review.

If you don't have a build environment, note that in the PR and ask a reviewer to test it.

### 4. Draft PRs for Work in Progress

Open a **Draft Pull Request** while you're still developing. This signals the change isn't ready for review and prevents accidental merges.

Convert to a regular PR once:
- The change is complete
- CI is green
- Self-review done

### 5. PR Description

Every PR needs:
- What changed and why
- How to test it
- Any known limitations

Use the PR template if one exists.

### 6. Response to Review

Review comments are not personal criticism — they're how the team maintains quality. Address them directly:

- **Resolved** — you made the requested change. Leave a brief note confirming.
- **Needs discussion** — explain why you chose a different approach. A reviewer may agree or reopen the discussion.
- **Blocking** — if a reviewer asks for a significant redesign, consider whether the change is still worth pursuing before reworking it.

PRs with unresolved blocking comments will not be merged.

## What Gets Merged

OpenW3D is an open project but not a fully open door. Changes are evaluated against:

1. **Correctness** — does it break existing functionality?
2. **Portability** — does it compile on Linux (GCC/Clang) and Windows (MSVC/MinGW)?
3. **Maintainability** — does it add technical debt or obscure the architecture?
4. **Scope** — is it in-scope for the project? (See Issue tracker for roadmap direction)

Large changes (new subsystems, backend rewrites, renderer overhauls) should be discussed in an Issue before opening a PR. The team is small — surprise PRs that require significant review time may be closed with an invitation to open an Issue first.

## What Doesn't Get Merged

- Changes that hard-depend on proprietary or commercial SDKs not available to all contributors
- Changes that break the Linux or macOS build
- Changes that add warning-heavy code to the codebase (treat warnings as errors on Linux CI)
- Cosmetic refactors mixed with functional changes
- Agent-generated PRs without playtesting evidence (the community is skeptical — small focused PRs with in-game testing are the exception, not the norm)

## CI

OpenW3D runs GitHub Actions on every PR and push:

| Job | Platform | Notes |
|-----|----------|-------|
| `openw3d` | Windows (MSVC x86/x64, MinGW x86/x64) | Full build, warnings as errors |
| `openw3d-linux` | Ubuntu (GCC) | Partial: wwmath, wwutil, wwdebug, etc. |

All jobs must pass before merge. If a job fails:

1. Read the error — most failures are obvious (missing include, typo, etc.)
2. Fix in your branch — push to update the PR
3. Don't ask a reviewer to diagnose CI failures for you

## Code Style

No auto-formatter is currently enforced. Use your judgment:

- **Indent:** 4 spaces (not tabs)
- **Line length:** Keep under 120 characters where practical
- **Naming:** Match the surrounding code (this codebase mixes Westwood's Hungarian notation with modern conventions — consistency with the existing file takes priority)
- **Headers:** Prefer including only what's needed; avoid massive precompiled-header-style includes

## Commit Messages

Use clear, conventional subject lines:

```
fix(wwlib): guard OutputDebugStringA with #if defined(_WIN32)

Closes #45
```

```
docs: add BUILD.md with Linux and Windows build instructions
```

Format: `type(scope): short description`. The first line is a summary — it should complete the sentence "This commit will..."

## Communication

- **GitHub Issues** — bug reports, feature requests, build problems
- **GitHub Discussions** — design proposals, architecture questions
- **Discord** — real-time coordination (link in repo description)

For significant changes, open an Issue first to gauge interest before investing time in a PR.

## License

By contributing, you agree that your contributions will be licensed under GPL v3 (same as the project).
