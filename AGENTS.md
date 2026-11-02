# UNDERGROWTH milestone contract

## Start here

Read [product orientation](docs/PRODUCT.md) for the instrument's purpose, current workflow and limits, then [engineering orientation](docs/ENGINEERING.md) for architecture, setup and validation. Read the current [hardware-first workflow and research](docs/UX_DESIGN.md) before changing navigation or key roles. Consult [the roadmap](docs/ROADMAP.md) for accepted decisions and [hardware notes](docs/HARDWARE.md) before firmware work. Keep these guides current when behavior or architecture changes; historical research is not an implementation specification.

Stephen requires every milestone to end with a flashable FM1 build, as well as the browser prototype. This requirement was accepted on 2026-11-01.

- Audio, sequencing, panel navigation, edit semantics and screen text belong in shared C. Browser and device wrappers handle their respective IO only.
- Run `tools/check_milestone.sh` before calling a milestone complete. It must produce a real `.fwsc`, validate its identity/CRCs/loader and pass the pinned platform's memory/instruction checks. Missing compiler/dependencies or a failed package gate means the milestone is incomplete; a host executable or WASM file is not firmware.
- Keep Bluetooth, Wi-Fi and CPU2 off unless Stephen requests them. Start empty and silent. Keep current session-only behaviour until saving is explicitly implemented.
- Preserve the pinned platform updater and crash-recovery path. Never use force-install flags, replace a bootloader, access a connected device or flash hardware merely to complete a software milestone. Physical flashing requires explicit separate authorization.
- Label builds experimental until real FM1 validation. Desktop CPU timings and native/WASM parity do not establish device performance or recovery success.
- Review upstream licence notices before distributing a firmware bundle. Do not assign an original-project open-source licence without asking Stephen.
- End completed changes with https://stephencarr.github.io/UNDERGROWTH/ and explicitly state published versus draft. Do not publish an incomplete hardware milestone as though it were ready.

## Review gate for rapid prototyping

- Monitor this repository’s PR reviews, inline comments and review workflow results, including recently merged PRs.
- Before publishing substantive code changes, let their review finish and read its findings. A successful review workflow means the review ran; it does not mean there were no findings.
- Stephen accepts all noncritical tech debt. Only critical risks block release: unintended flashing/writes, serious loss or corruption of user data, or failures of similar consequence. Assess the concrete impact rather than treating every review severity label as a blocker.
- Record noncritical findings as accepted/deferred debt. Do not expand implementation, optional testing or cleanup to seek a zero-comment review. Documentation-only changes and accepted debt do not need additional review cycles.
- Complete the mandatory milestone gate and verify fixes to critical issues, then ship. Never access or flash connected hardware without separate explicit authorization.

## PR previews are part of delivery

- Use a feature branch from current `main`, push it, and open/update a PR. Provide a Railway preview for user audition before treating a functional change as ready to ship. Do not stop at a local commit or give the shared base URL as though it contains the PR.
- Follow the [PR preview procedure](docs/ENGINEERING.md#railway-browser-previews). Automatic PR environments copy `preview-base` (which follows `main`); each PR gets its own branch, deployment and domain. Branch pushes update that PR's preview. Do not point `preview-base` at a feature branch or manually replace another PR's preview.
- For a review/preview handoff, use a ready-for-review PR. If a draft has no environment, mark it ready and check again. If PR environments were enabled after the PR was opened and no environment appears, reopen your own PR once to refresh its event. Never merge merely to obtain a preview.
- Verify Railway deployment `SUCCESS` for the PR's latest commit and use the domain returned for that PR environment. Check `/version.json` and browser audio/input when network access permits. If access is blocked, explicitly report that only deployment metadata was verified; do not claim a live browser pass.
- Include the PR link and its preview link in the handoff, with the tested commit and any validation limits. Keep experimental firmware and the mandatory milestone gate; preview hosting does not replace firmware validation, PR review, or listening acceptance.
- Railway previews are for audition. State that GitHub Pages is unchanged until the change is actually released there. Merging, release publication and physical flashing retain the existing review and authorization rules above.
