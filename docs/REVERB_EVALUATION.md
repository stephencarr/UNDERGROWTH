# Reverb evaluation — 2 November 2026

Status: 0.4.3-hw1 candidate. Human listening and physical FM1 CPU acceptance
remain open. The 0.4.2 room passed signal tests but failed the owner's listening
requirement: full settings should swallow the source, with ambience below full
Decay. This comparison separates implicit Synth send locks from the room sound.

## Sources inspected

| Candidate | Inspected source | Decision |
| --- | --- | --- |
| Previous UNDERGROWTH room | 99640c8, four combs/two allpasses per side | Sparse early response, short useful midrange tail; replaced. |
| Mutable Instruments Clouds | eurorack f4cfa7697095a28d292bff758beb51deb49b5a86, clouds/dsp/fx/reverb.h and fx_engine.h | Selected MIT-licensed topology: four input diffusers, cross-coupled allpass/delay tank and modulation. |
| Mutable Instruments Plaits | Same revision, plaits/dsp/fx tree | No standalone reverb in inspected tree. Its synthesis engines do not supply this shared effect. |
| JIANT | af8cdb8ee2a45a6ea3d43f20f259a2a2578b7c14, firmware/src/fx.c | Includes Freeverb-like ROOM and other models; fixed-point/reduced-rate implementation. GPL-3.0-only; read as reference, no code copied. |
| FM1-DUB | 501c4cf22bb2c5ffdcf43467890127de623825dd, firmware/src/dsp/amb.c | Dattorro plate, input diffusers and multi-tap stereo tank, run at half rate. GPL-3.0-only; no code copied. |
| X0X | ca2d377e3b9522579245152a81781a575f4d2fea, firmware/src/dsp/fxbus.c | Eight-line room network with input diffusion, int16 storage and device-specific block optimisation. GPL-3.0-only; no code copied. |

Source repositories: [Mutable Instruments](https://github.com/pichenettes/eurorack),
[JIANT](https://github.com/weavejam/JIANT), [FM1-DUB](https://github.com/sbegueria/fm1-dub),
[X0X](https://github.com/charlesvestal/fm1-x0x). Revisions above pin the inspection;
community firmware establishes implementation precedent, not UNDERGROWTH CPU fit.

## Port and cost

`core/room.h` adapts Clouds' topology and accumulator/previous-read allpass
semantics into shared C. It is not a bit-exact module emulation. Changes:
explicit floating-point circular lines, rate-scaled lengths, two modulated long
delays, new damping/decay mappings, stereo width and aux return, Panic/bypass.
Clouds' extra first-diffuser memory smear is omitted; tank modulation remains.
The complete Emilie Gillet MIT notice is retained in source, standalone HTML
and firmware distribution notices. Original project licensing remains undecided.

There are ten delay lines and eight allpasses per sample; two tank delay reads
interpolate. Fixed storage is 49,200 floats (196,800 bytes), supporting 8–96 kHz.
Clouds originally uses 16,384 uint16 entries at 32 kHz (32 KiB). This port spends
more memory to avoid quantized low-level tails and resampling dependencies.
Rate-scaled tank delays preserve room time across supported sample rates.
Feedback is bounded at .96 per branch; no freeze or unbounded feedback control.
Controls smooth over approximately 5 ms. No render allocation or IO.

Initial cross-build: app 119884 bytes, data+bss 40572/98304, pool 312908/344064;
identity FM-1_731000403, CRC/loader/instruction checks passed. These figures
must be refreshed from the final build report if subsequent code changes affect
them. Desktop dense-pattern rendering was roughly 0.16 s without room / 0.20 s
with room per ten seconds of audio in one run, with grains on. This is comparative
host cost, not a device CPU percentage. Physical CPU/P/X comparisons remain open.

## Routing ownership

ROOT first-entry snapshots keep Sound/Character slots 0–10 except Synth send 3
(mask 2039). They never capture atmosphere slots. Explicit send locks/motion
still exist and reactivation preserves them. Native checks cover stopped edits,
restart, Rejoin, intentional send locks, takeover, locked-rest audition and motion
Undo. Mouse/touch checks enter notes before raising both sends. FX displays
LIVE effective sends during playback, draft sends while stopped. Knob edits in
playback start at the effective send; Rejoin restores intentional automation.

Sends feed delay and reverb in parallel. At full Reverb, each fully sent layer's
dry gain fades to zero; unsent layers keep their dry gain. Delay return recedes
at maximum Reverb. ENV requires Enabled ON, Level and PLAY, with no steps needed;
Stop fades ENV while the room continues. Panic clears sources and both tails.

## Reproducible listening comparisons

Run `python3 tools/reverb_audition.py --output work/audition`. Optionally pass
`--reference work/previous-core.so`, compiled from 0.4.2 with `-O2 -ffp-contract=off
-shared -fPIC`, for matched A/B files. The tool compiles current shared C, never
accesses devices, and writes an HTML player, WAVs and JSON metrics.

Each recording is twelve seconds: four seconds of ROOT notes or continuous ENV
on an empty sequence, then eight seconds of tail. Source, master and send remain
identical between models. Full Reverb, Damping 45%, Width 80%, Delay return zero;
Decay 25%, 55%, 100%, plus dry references. No peak or loudness normalisation.
The original generated ENV texture is deliberately retained for this comparison.

Representative 44.1 kHz output (RMS in the second second after stopping source):

| Source / room | Decay 25% | Decay 55% | Decay 100% |
| --- | --- | --- | --- |
| ROOT previous | below .00005 | below .00005 | .0141 |
| ROOT Clouds-derived | .0082 | .0214 | .1204 |
| ENV previous | below .00005 | below .00005 | .0062 |
| ENV Clouds-derived | .0016 | .0031 | .0070 |

No sample in these fixtures reaches the .95 output clamp; largest ROOT peak .5991,
ENV peak .0640. Continuous-source wet/dry checks at default Decay report about
-0.51 dB ROOT / -0.58 dB ENV. These describe level and persistence, not pleasantness.

The independent impulse regression checks >99% nonzero tail occupancy from
200–1000 ms, finite/limited peak, substantial second-second energy even at 25%
Decay and decreasing late energy at four sample rates. At 44.1 kHz/55% Decay,
second-second energy is .0521 versus .7526 in the first second; at maximum it is
.2992 versus .9804. This tests temporal fill separately from instrumental RMS.
A fully wet attack remains much softer than dry; allpass feedthrough means it
is not literally silent before a first delay reflection.

Listen for grain texture dissolving into space, note attacks becoming submerged
at full send/return, useful shorter settings, stereo motion, resonant ringing,
level pumping and the usefulness of the Decay travel. These measurements alone
cannot accept the requested enveloping sound. No listening approval is claimed.
