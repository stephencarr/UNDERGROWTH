# UNDERGROWTH milestone contract

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
