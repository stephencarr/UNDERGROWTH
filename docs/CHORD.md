# CHORD organism — 0.5.0-hw1 candidate

Status: draft, not yet published. Shared C implementation with a matching
experimental FM1 build; final full-gate and listening evidence follow below.
The fifth track complements Kick, Chatter, ROOT and Backbeat. It always renders
four chord tones. Tone/Colour morph independently of the chosen harmony.

## Playing and shaping

Select CHORD with ALGORITHM (or the Expanded card / computer key 5).
The physical keyboard plays semitone roots F2–G4 at the default octave; OCT±
shifts it within C2–C5, with out-of-range keys silent. Computer Q–I plays C3–C4
while CHORD is selected. F / Hear chord triggers a finite draft audition.
SEL auditions the draft or selected trig/rest without enabling it.

| Page | KNOB1 | KNOB2 | KNOB3 | KNOB4 |
| --- | --- | --- | --- | --- |
| EDIT Sound | Tone | Colour | Decay | Send |
| EDIT Notes/velocity | Root | Gate | Velocity | Slot |
| EDIT Harmony | Slot | Inversion | Spread | Gate |
| EDIT Palette tones | Slot | Tone 2 | Tone 3 | Tone 4 |
| ENV | Attack | Release | Sweep | Decay |
| SEQ, closed locks | Selected root | Selected gate | Selected velocity | Selected slot |
| FX track | Send | — | — | — |

Tone opens the two-pole low-pass; Sweep adds a falling filter envelope. Colour
moves from sine warmth toward band-limited saw/pulse hollowness and brightness.
Decay spans short stabs through a held sustain at its top. Attack reaches about
two seconds and Release about four seconds. For a swell, raise Attack and Decay,
then hold a key; the gate still matters. Gate is 10–150% of a sequencer step.
STOP releases both tail banks. Panic clears them and shared effects immediately.
Sends feed the existing parallel delay/reverb; global FX controls are unchanged.

Initial audition settings (percent, with Send zero):

| Intent | Tone | Colour | Decay | Attack | Release | Sweep |
| --- | --- | --- | --- | --- | --- | --- |
| Rounded dub stab | 20 | 35 | 20 | 0 | 15 | 55 |
| Bright hollow chord | 90 | 85 | 40 | 0 | 25 | 25 |
| Ambient held swell | 55 | 45 | 100 | 70 | 80 | 15 |

These are starting points for listening, not preset buttons or approval claims.
Add reverb after comparing dry articulation.

## Harmony and score ownership

The eight initial slots are major {0,4,7,12}, minor {0,3,7,12}, dominant seventh
{0,4,7,10}, major seventh {0,4,7,11}, minor seventh {0,3,7,10}, sus4 {0,5,7,12},
power {0,7,12,19}, diminished {0,3,6,9}. Root zero is fixed; the other three
intervals are editable from +1 to +24 semitones, permitting doubled/unison upper
tones but always keeping a tone above the root. Inversion raises the first 0–3
tones by an octave; Spread opens alternating tones into higher octaves. Range
handling folds high notes by octaves rather than clamping every voice to one
frequency. These are four-tone voicings, not a mono or root-only mode.

Each slot remembers its intervals, inversion and spread for the session.
Selecting a slot loads that actual definition into the draft. Ordinary trig entry
captures the definition plus root, gate, velocity and sound controls; it excludes
Send. Editing a slot later does not rewrite existing trigs. Selecting a slot
inside a trig menu copies its current definition into that trig. Individual
trig tones can then be edited on Trig tones. Reactivation preserves authored data.

Continuous sound/ENV parameters support locks and motion. Harmony values also
support explicit locks and motion, but the sounding chord's pitches/voicing latch
on its note onset; they do not retune a held bank halfway through a note. Hits
record the played root and harmony, Both also captures touched controls, and
Motion changes existing hits only. One Undo restores the take, including palette
edits made outside a take. A rest remains silent while edited; SEL can hear it.

HOME lower key 5 mutes CHORD. HOME SELECT page 4 exposes its level; first-four-track
mix controls remain on page 1. USB MIDI channel 5 accepts chord-root note-on/off
and velocity; ROOT remains on channel 3. USB-loss/Panic and updater safeguards
remain mandatory. No device access or flashing is part of this implementation.

## Voice and resource budget

Two fixed banks of four tones: at most eight oscillators, with an old chord
released when a new one starts. If both banks are active, the quieter envelope
is stolen with a short transition. This is bounded overlap of monophonic chord
events, not an unlimited held-key chord stack. There is no sample table, delay
arena or additional pool buffer. Generated ROOT texture and reverb remain shared
as before. Palette/score snapshots and oscillator/filter state use data/bss.

Initial cross-build: app 130448 bytes, data+bss 45044/98304, pool 312908/344064.
These numbers require refresh from the final build report. Host timing with all
five tracks, ROOT grains and reverb is comparative evidence only; physical FM1
CPU/P/X testing and listening remain outstanding.

## Verification and limits

Native checks cover all four chord tones in the rendered spectrum, slow attack,
release, eight-slot/voicing corners at 8/44.1/48/96 kHz, fixed bank bounds, silent
boot/empty Play, finite one-shot audition, Stop/Panic, independent snapshot
harmony, palette Undo, copy/refusal, rests, scoped recording/motion and panel/mix.
Real mouse/touch browser checks exercise fifth-track selection, palette isolation,
sequence/live AudioWorklet output, trig menus, mixer and Expanded controls.
Native/WASM parity includes chord events and high-bit harmony parameters.

The complete milestone gate must pass with a real 0.5.0-hw1 package, including
platform-host chord/MIDI behavior, before this candidate can be called complete.
This is original synthesis; no Caravel or third-party chord DSP was copied.
Sample import, scenes/morphing, arp, saving and the full UI overhaul remain future
roadmap work. Chord names/scale mapping and larger polyphony are not implemented.

Dry dub/bright/ambient and shared-room ambient recordings can be reproduced with
`python3 tools/chord_audition.py`. The isolated fixtures contain no clipped samples;
peaks were .1041, .2645, .1624 and .3528 respectively. Slow-attack and sustained
articulation are exercised without depending on reverb. These measurements do
not replace the owner's listening assessment.
