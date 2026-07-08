# Pull Request

## What

<!-- Brief description of the change. Use the conventional commit format:
     `type(scope): Description starting with action verb` -->

## Why

<!-- Context, motivation, link to issue(s). For backports from GeneralsX /
     TheSuperHackers, cite the source commit. For non-obvious changes, explain
     the rationale. -->

## How

<!-- Optional: implementation details, alternatives considered. -->

## Test plan

<!-- How was this verified? Windows MinGW build? Both renegade.exe and
     renegadeserver.exe? Linux config-only check? Replay tested? -->

- [ ] Builds clean on Windows MinGW x64 (`cmake --build build/gcc-test --target renegade`)
- [ ] Builds clean on Windows MinGW x64 (`cmake --build build/gcc-test --target renegadeserver`)
- [ ] `cmake --preset linux` configures without errors
- [ ] Touched the right files for the scope of the change
- [ ] Added `// OpenW3D @keyword author DD/MM/YYYY Description` comment(s) where needed

## Backport

<!-- Only for backports from upstream projects (GeneralsX, TheSuperHackers).
     Cite the source commit. -->
<!-- Example:
     Backported from TheSuperHackers/GeneralsGameCode commit 98f1db9929
     (SortingRenderer merge, June 2026). OpenW3D-specific adaptations listed
     in the commit body. -->

## Notes

<!-- Anything reviewers should know: known issues, follow-up work, breaking
     changes, ABI compatibility, etc. -->
