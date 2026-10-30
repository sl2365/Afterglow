# Afterglow — Changelog

## 1.78.0 — Per-Target Depth and Delay Clouds — 30 October 2026

### Added

- Added a saved, host-automatable 0-100% Depth knob to every assigned sequencer-target row. The existing bipolar Sequencer DEPTH remains the master shape control; each row knob independently scales only that destination in normalised parameter space.
- Added vertical drag, Shift-fine drag, wheel adjustment and a 100% double-click reset to each compact target Depth knob. The enabled LED and remove cross retain their existing functions, and ANIMATE follows the final depth-scaled destination value.
- Added a restrained energy-normalised upper voice to positive Pitch Lush transpositions. It strengthens octave-up shimmer as P MIX rises and participates in the normal feedback path without adding a new control or changing other modes.

### Fixed

- Kept Stereo and Multi-Tap read heads at their selected times while Diffusion Size changes. The allpass network now grows only the cloud after each tap instead of moving the immediate tap earlier to compensate for the network span.
- Replaced the Targets list's font-dependent Unicode scroll triangles—which could appear as stray characters or ellipses—with native vector chevrons. Both indicators are now clickable while wheel scrolling remains available.

### Saved state

- Advanced saved-state schema to 40 and appended all 16 target Depth parameters after every established host parameter. Schema 32-39 states receive the exact former behaviour at 100% Depth, while new presets and projects restore every row independently.
- Updated the external converter to emit complete schema-40 presets with neutral 100% target Depth values.

## 1.77.0 — Twin Tape Timing Cleanup — 30 October 2026

### Changed

- Hidden both the `MULTI` label and its LED selector while Twin Tape is selected, rather than displaying a control that cannot be used in that mode.
- Preserved the complete stored Multi-Tap setup. Selecting any other mode restores the selector and its previous state without changing its parameters.

### Preserved

- Left all audio processing, Twin Tape transport and character, preset conversion, parameter IDs and saved-state schema 39 unchanged.

## 1.76.0 — LoFi Playtime Preset Conversion — 29 October 2026

### Added

- Added Yum Audio LoFi Playtime `.lofiplaytimepreset` files to the external preset converter for individual files, unzipped folder trees and ZIP banks.
- Mapped every directly corresponding source value to Twin Tape: shared free/synchronised Basetime, both speeds and directions, independent Age, Shine and Gain, Feedback, Spread, Mix and Freeze. Source note indices are advanced past Afterglow's later 1/128 entry so their audible divisions remain unchanged.
- Updated generated preset XML to complete current schema-39 states, including neutral sequencer values and the Twin Tape parameters. New conversions no longer masquerade as schema 34 or rely on compatibility migration to fill newer controls.
- Added explicit conversion notes for every Playtime head and for values outside Afterglow's available range.

### Changed

- Updated the portable converter batch and instructions to recognise Playtime presets alongside the existing formats.
- When a 0%-Spread Playtime preset disables one tape, the converter mirrors the enabled transport into Afterglow's second averaged head. This preserves the intended single pitch/reverse layer without leaking an unrelated head; the report identifies the approximation because Twin Tape has no per-head power parameter.

### Preserved

- Left the v1.75.0 Twin Tape audio engine, approved pitch and reverse transports, Feedback and every existing preset conversion unchanged.

## 1.75.0 — Twin Tape Age — 29 October 2026

### Changed

- Rebuilt the independent `AGE I` and `AGE II` response around their intended tape-wear role. Each head now develops clearly audible but separately moving slow warble, wandering speed error and restrained faster flutter, producing the expected two-tape chorus instead of a nearly inert control.
- Changed dropouts from a tiny continuous attenuation into bounded wear events that become more evident towards the top of AGE without muting a head or destabilising Feedback. Raised signal-tracking texture and soft tape compression enough to make the complete control range useful.
- Separated per-head AGE from the SHINE cutoff calculation. SHINE retains the established approved brightness response, while AGE no longer needs to darken the head before its movement can be heard.

### Preserved

- Left the approved v1.72.0 forward pitch transport, v1.74.0 reverse transport, Freeze, Feedback, both character pages and every non-Twin mode unchanged.
- Retained saved-state schema 39 and every parameter ID; existing presets keep their exact values.

## 1.74.0 — Twin Tape Reverse Transport — 29 October 2026

### Changed

- Reverted Twin Tape's complete forward Slow/Normal/Fast path to the approved v1.72.0 fixed-window implementation. The v1.73.0 timing-linked window did not retain the preferred musical pitch-shifted echo and has been removed rather than kept as a compatibility path.
- Rebuilt only Twin Tape's live Reverse timing around longer, speed-aware Hann-overlapped loops. Reverse Normal now spans one selected Time, Reverse Slow spans two Times and Reverse Fast spans half a Time while retaining their established 1x, 0.5x and 2x reverse pitch rates.
- Routed reverse Time changes through the responsive Colour-style virtual transport before calculating each splice. Manual or automated Time movement now eases continuously into the reverse loop instead of rigidly rescaling every grain directly from the knob.

### Preserved

- Left the approved v1.72.0 forward pitch character, both character pages, Freeze, Feedback, per-head processing and every non-Twin mode unchanged.
- Retained saved-state schema 39 and every parameter ID. No obsolete forward algorithm or compatibility selector remains.

## 1.73.0 — Twin Tape Transport Calibration — 29 October 2026

### Changed

- Calibrated Twin Tape's forward transport from the supplied clean and eight-state continuous reference recording at a 505 ms Basetime. Slow and Fast retain their exact half-speed and double-speed octave shifts.
- Replaced Twin Tape's inherited fixed 80 ms forward pitch window with a dedicated window equal to half of the active head Time. At the recorded 505 ms setting, Slow now renews at approximately 505 ms and Fast at approximately 252.5 ms, matching the measured reference motion instead of producing an unrelated dense granular rate.
- Kept the established Hann-overlapped, click-resistant pitch path, per-head character, shared character page and feedback topology. Only Twin Tape's forward splice period changed; the normal and reverse transports are untouched.

### Preserved

- Retained saved-state schema 39 and every parameter ID. This is an engine calibration with no preset conversion or compatibility branch.

## 1.72.0 — Twin Tape Character Pages — 29 October 2026

### Added

- Added an amber LED page switch between Twin Tape's independent `AGE I`/`SHINE I`/`AGE II`/`SHINE II` controls and the original `DIST`/shared `AGE`/`WOW`/`FLUTTER` controls. Switching pages changes only the visible attachments; every value remains live and untouched.
- Enabled the established input Distortion circuit and all sixteen Dist Types in Twin Tape instead of silently forcing DIST to zero.
- Added a shared Twin Tape AGE stage. It applies restrained common bandwidth wear and level-compensated asymmetric tape saturation after each head's independent Age and Shine processing.

### Preserved

- WOW and FLUTTER already moved the Twin Tape read heads through the common click-resistant modulation path; they are now visible on the alternate page without changing that established DSP.
- Retained saved-state schema 39 and every existing parameter ID. The panel selector is an editor view choice rather than a preset parameter, so it adds no compatibility branch or redundant saved state.

## 1.71.0 — Twin Tape — 29 October 2026

### Added

- Added the `Twin Tape` mode after every existing mode, retaining all released mode indices. It runs two independent windowed virtual-tape heads using the Left and Right Time controls as their cycle lengths.
- Added six click-resistant speed/direction states per head: Reverse Fast, Reverse Normal, Reverse Slow, Slow, Normal and Fast. Slow and Fast use approximately half- and double-speed playback for octave-down and octave-up layering.
- Added independent Age, Shine and Gain parameters for both tape heads. Age supplies separately moving warble, restrained dropouts, signal-tracking texture and soft tape saturation; Shine controls per-head bandwidth before the heads are mixed and regenerated.
- Added Twin Tape stereo routing to Spread. At 0% both heads remain complete stereo layers; positive and negative extremes assign the heads to opposite sides in either order.
- Added a dedicated preallocated Twin Tape Freeze capture. Each head retains its own captured cycle and remains responsive to speed, direction, Age, Shine and Gain while frozen.
- Added all eight Twin Tape controls to the sequencer target list and saved-state schema 39.

### Interface

- Added Twin Tape to the alphabetised Mode selector with its own muted rose-brown theme colour and two-line contextual help.
- Reused existing panels only while Twin Tape is selected: SHIFT/DETUNE become `TAPE I`/`TAPE II`; DIST/AGE/WOW/FLUTTER become the four Age/Shine controls; HI CUT/LO CUT become `GAIN I`/`GAIN II`. The stored controls return unchanged when another mode is selected.
- Multi-Tap remains stored but is visibly unavailable in Twin Tape because the two tape heads already provide its parallel timing topology.

### Preserved

- Appended the new mode and parameters without changing any established mode index, parameter ID or preset meaning. Schema 32-38 states receive neutral Twin Tape defaults when loaded.
- Kept every existing delay algorithm and its DSP unchanged.

## 1.70.0 — Phaser Feedback Stability — 29 October 2026

### Fixed

- Corrected an engine-level gain error exposed by the converted `PHASER - Supa Res` preset. Its 3 ms delay, 87.4% Feedback and 30% internal Phaser resonance could produce a worst-case outer-loop gain of approximately 1.056, allowing a slowly swept resonant peak to grow even though Feedback was below 100%.
- Added a fixed analytical normalisation derived from the Phaser's current internal resonance. It is applied only to the Phaser contribution returning to the outer delay loop; it has no detector, envelope, attack or release and therefore cannot pump.
- Feedback at 100% now remains nominally unity at the Phaser's worst resonant peak, while settings above 100% retain intentional regenerative and self-oscillating behaviour.

### Preserved

- Kept the audible first echo, internal Phaser resonance, notch sweep, diffusion, filters and wet-output safety unchanged. Diffusion was already after the Phaser/resonance stage; moving it could alter the texture but could not correct the excess loop gain because its all-pass network does not reliably remove energy.
- Kept every other delay mode, parameter ID, preset value and saved-state schema 38 unchanged. No preset-name exception or conversion compatibility branch was added.

## 1.69.1 — Uhbik-D Factory v1 Conversion — 29 October 2026

### Fixed

- Recognised the older `#AM=Uhbik-D` product identifier used by the Uhbik Factory v1 bank as the same supported delay format as the newer `#AM=Uhbik Delay` identifier.
- Unsupported u-he `.h2p` products inside a mixed source folder or ZIP are now identified as skipped in `Conversion Notes.txt` instead of aborting conversion of the complete bank. A source containing no supported presets still fails clearly rather than reporting a false successful conversion.

### Preserved

- Kept all v1.69.0 Uhbik parameter mappings, delay DSP, parameters and saved-state schema 38 unchanged.

## 1.69.0 — Uhbik Delay Preset Conversion — 29 October 2026

### Added

- Added one-time conversion of u-he Uhbik Delay `.h2p` presets from individual files, unzipped folder trees or ZIP banks while retaining their source hierarchy.
- Converted the four strongest audible Uhbik taps into Afterglow Multi-Tap timing, Pan and Level. Uhbik's global Speed is materialised into each musical tap position before it is matched to the nearest Afterglow tempo division.
- Added direct or closest mappings for Feedback, Low/High Cut, Soft Clip, Mix, Output and bypass.
- Converted Uhbik's built-in smooth LFO to MOD AMT/RATE and its irregular Flutter to WOW/FLUTTER. Autonomous modulation-matrix LFO or Mod Noise assignments to Speed or tap time add conservative approximate movement.

### Conversion boundaries

- Afterglow has four tap heads rather than five. The least audible source head is omitted and identified in each conversion report.
- Uhbik's per-tap feedback selectors form a separate feedback-head topology and can remain active on inaudible taps. Afterglow has one shared Multi-Tap feedback loop, so the source selection is reported rather than assigned to unrelated switches.
- Arbitrary MIDI, envelope, Mapper, Pan, gain and filter modulation routes, continuously variable tap positions between Afterglow divisions, and Uhbik's separate Input/Wet gains have no direct equivalents. Every material omission or approximation is written to `Conversion Notes.txt`.

### Changed

- Renamed the general external-preset script from `valhalla_to_afterglow.py` to `preset_converter.py` and updated the Windows batch launcher and instructions.
- Kept all delay DSP, parameters and saved-state schema 38 unchanged; this release changes only the offline converter and displayed version metadata.

## 1.68.0 — 1/128 Timing — 29 October 2026

### Added

- Added 1/128 as the shortest Stereo and Multi-Tap tempo-sync division, ahead of 1/64 triplet in the musically ordered timing range.
- Updated the external preset converter so all newly converted ValhallaDelay, Deelay PE and ColourCopy presets use the revised timing indices.

### Changed

- Shifted the established Stereo and Multi-Tap division indices up by one to accommodate 1/128 at the correct start of the list.
- Adjusted all 737 supplied Afterglow presets by changing only `divisionL`, `divisionR` and the four `tapNDivision` values. Every timing value therefore retains the exact musical duration it had before 1/128 was inserted.
- Shifted new-instance Stereo and Multi-Tap division defaults to their equivalent revised indices.

### Compatibility

- No runtime migration or backwards-compatibility branch was added. Presets outside the supplied adjusted bank and plugin states embedded in existing DAW projects must be corrected manually if they use tempo-synchronised timing.
- Retained saved-state schema 38 and every parameter ID.

## 1.67.0 — Sequencer Range Targets and Extended Timing — 29 October 2026

### Added

- Made the Sequencer START and END labels draggable modulation targets. Their stored values remain the reference points, while the previous stable sequence value moves the active playback range without creating a circular current-step dependency.
- Added 2/1, 3/1 and 4/1 timing choices to Stereo and Multi-Tap tempo sync. All three were appended after 1/1, preserving every existing timing index and saved preset exactly.
- Increased the delay capacity from 8 to 16 seconds so 4/1 remains available down to 60 BPM instead of being silently truncated at ordinary slower tempos.
- Added a contextual-help heading and description for the Preset Browser button.

### Changed

- Set new-instance Sequencer HUE, SAT and LUM defaults and double-click resets to 50%.
- Replaced generic `AFTERGLOW` and `SEQUENCER` contextual-help headings with the hovered control name throughout the interface.
- Echo Offset now visibly greys out, including its guide, handle and label, whenever the selected mode does not use it.

### Preserved

- Deliberately left 1/128 out rather than appending it in an illogical position or shifting the established division indices used by existing presets and automation.
- Appended START and END after every established Sequencer target choice, retaining all existing target indices and preset assignments.
- Retained saved-state schema 38 and every parameter ID without conversion logic.

## 1.66.0 — ColourCopy Preset Conversion — 29 October 2026

### Added

- Added one-time conversion of u-he ColourCopy `.h2p` preset files to Afterglow presets. The existing converter accepts individual files, unzipped banks and ZIP banks while retaining their folder structure.
- Added exact parsing of ColourCopy's 21 TIME BASE choices and its quarter-speed to four-times-speed RATE scaling. Left and Right tap positions are materialised into Afterglow's independent timing controls.
- Added direct or closest available mappings for Regen, feedback routing, Width, time-based LFO targets, continuous Colour position, Saturation, Brightness, AMP ducking, Mix and Output.

### Conversion boundaries

- Feedback-only ducking, AMP LFO modulation, input routing and gain, feedback phase inversion, dynamic LFO behaviour, noise-floor selection and MIDI Note Control are reported rather than silently assigned to unrelated controls.
- ColourCopy's independent tap position and full feedback-loop period cannot both be represented by Afterglow's single timing value per channel. The audible first taps are retained and the difference is recorded in each conversion report.
- ColourCopy's triangle LFO and continuously variable stereo phase use the nearest available Afterglow sine/phase behaviour, with every approximation written to the report.

### Preserved

- Kept the restored v1.65.2 Colour AGE character and all delay DSP unchanged. This release changes only the external one-time preset converter plus displayed version metadata.
- Retained saved-state schema 38 and all parameter IDs without preset migration code.

## 1.65.2 — Colour AGE Character Restore — 29 October 2026

### Changed

- Removed the v1.64.1 feedback-level restoration from Colour. Strong AGE settings once again retain their original, more obvious cumulative loss and character instead of compensating that behaviour away inside the feedback loop.

### Preserved

- Kept Colour's AGE companding, bandwidth softening, texture and asymmetric saturation exactly as introduced in v1.64.0.
- Kept the v1.65.0 virtual-clock modulation, manual Time movement, cubic interpolation and every other mode unchanged.
- Retained saved-state schema 38 and all parameter IDs without conversion logic.

## 1.65.1 — Bipolar Sequencer Depth — 29 October 2026

### Fixed

- Restored DEPTH as a bipolar sequence-shape control. Negative values shrink the complete curve, 0% preserves the curve exactly and positive values expand smaller curves towards the available Unipolar or Bipolar boundary.
- Restored the centre detent and 0% double-click reset on the DEPTH slider while retaining the clearer DEPTH name and target-relative modulation introduced in v1.60.0.

### Preserved

- Kept saved-state schema 38 and the established `sequencerPeak` parameter ID. No conversion or compatibility branch was added because no presets use the temporary unipolar DEPTH behaviour.

## 1.65.0 — Colour Clock Modulation — 29 October 2026

### Changed

- Routed Colour's MOD, WOW and FLUTTER movement into the same responsive virtual-clock targets used by manual Left and Right Time changes. Modulation now follows the approved continuous transport trajectory before cubic interpolation rather than being added afterwards.
- Gave every Multi-Tap Colour head independent Left and Right clock state so stereo modulation follows the same bounded movement model as the main Stereo timing path.
- Shortened the contextual descriptions that could wrap onto a third line, including DIFF SIZE and Colour AGE, so the fixed information strip no longer clips their top or bottom edges.

### Preserved

- Kept Colour's manual Time response, cubic reader, AGE character, feedback-length correction and audible wet level unchanged.
- Retained the 0.25x-4x Colour playback bounds for automated movement, preventing deep or fast modulation from stalling, reversing or teleporting a read head.
- Left every other mode's established MOD, WOW and FLUTTER behaviour unchanged, with saved-state schema 38 and all parameter IDs intact.

## 1.64.1 — Colour AGE Feedback — 29 October 2026

### Fixed

- Restored part of the circulation loss introduced by strong Colour AGE settings. Fully coloured repeats now retain more feedback presence and decay instead of becoming disproportionately short or weak.

### Preserved

- Left the audible first echo and wet-output level unchanged; correction applies only to the recirculating Colour signal.
- Defined the sub-unity correction as a fraction of the remaining distance to unity. It is mathematically unable to cross unity below 100% Feedback and fades to zero at and above 100% and during Freeze, preserving the established self-oscillation and captured-loop behaviour.
- Used a fixed AGE/Feedback curve with no envelope follower, attack, release or time-varying gain, so the correction cannot pump.
- Retained saved-state schema 38 and every parameter ID without migration code.

## 1.64.0 — Colour Character — 29 October 2026

### Added

- Added a dedicated repeat-character path to Colour. AGE now morphs from the exact clean cubic delay at 0% into restrained companding, progressively softer two-stage bandwidth, very low signal-tracked texture and asymmetric saturation.
- Applied the same Colour character model to Stereo and Multi-Tap timing, with independent stereo state and stable preallocated processing.

### Changed

- Placed Colour's character before the shared EQ, diffusion and feedback routing so each circulation accumulates the selected tone naturally instead of applying a cosmetic filter only to the final output.
- Enabled AGE in Colour and added mode-specific contextual help describing its clean-to-coloured range.

### Preserved

- Kept the v1.63.0 virtual-clock trajectory, 0.25x-4x movement bounds, cubic interpolation, Left/Right timing behaviour and all other modes unchanged.
- Kept AGE 0% mathematically on the direct Colour read. The new compander, filters, texture and saturation remain fully blended out at that setting.
- Retained saved-state schema 38 and all existing parameter IDs. Colour uses the established AGE parameter, so no new compatibility or migration code was added.

## 1.63.0 — Colour Transport — 29 October 2026

### Added

- Added the `Colour` delay mode with independent Left and Right virtual-clock read heads. Time changes now create a continuous Doppler transition rather than a fixed-head crossfade.
- Added a dedicated four-point cubic delay reader for Colour. Both sample value and slope remain continuous as its read heads move, improving clarity during slow sweeps, quick changes and direction reversals.
- Extended the same Colour transport and interpolation to Multi-Tap timing.

### Changed

- Bounded Colour's effective moving-head playback range from 0.25x to 4x and added short velocity easing. The heads follow active Time gestures promptly without Tape II's deliberately slower mechanical inertia, while large discontinuous changes cannot stall, reverse or teleport the read position.
- Added Colour to the alphabetised Mode menu with a subdued warm-brown mode theme and context-sensitive help.

### Preserved

- Kept Colour's first stage deliberately clean, without fixed drift, noise, saturation or head loss. Existing Mod, Wow, Flutter, EQ, DIST, feedback, Link, Tempo Sync and Ping-Pong controls remain available.
- Appended Colour after every released mode, retaining all existing mode indices, parameter IDs, preset meanings and saved-state schema 38 without a migration layer.

## 1.62.0 — Regenerative Feedback Colour — 29 October 2026

### Changed

- Added a common unity-slope saturation after every mode's existing feedback processing. It remains completely bypassed through 100% Feedback, then fades in smoothly across the displayed 100-200% regenerative range.
- Gave loud self-oscillation a denser harmonic shape and lower common large-signal headroom without adding automatic reverb, diffusion, an envelope follower or mode-specific preset exceptions.

### Preserved

- Kept the complete 0-100% Feedback curve, ordinary preset sound, decay calibration, small-signal regenerative gain, per-mode colour, Freeze capture and v1.61.0 stereo-linked wet-output safety unchanged.

## 1.61.0 — Stereo-Linked Wet Safety — 29 October 2026

### Changed

- Replaced the former per-channel output soft clip, which permitted wet peaks up to +3.5 dBFS, with a stereo-linked static safety knee on the actual audible wet pair.
- Kept the safety stage exactly transparent below -2 dBFS and placed its asymptotic sample ceiling at -0.5 dBFS. MIX and OUT remain afterwards, retaining intentional output-level control.

### Fixed

- Prevented extreme Feedback and DIST combinations from creating unexpectedly excessive wet output while leaving the Feedback curve, feedback-loop duration, per-mode processing, Freeze and ordinary preset levels unchanged.
- Applied one shared instantaneous gain to both wet channels, preserving the stereo image without an envelope follower, attack/release behaviour or compressor pumping.

## 1.60.0 — Relative Sequencer Targets — 28 October 2026

### Changed

- Renamed the visible Sequencer `PEAK` control to `DEPTH` and `MOVE` to `OFFSET`, including their host-visible and target-list names. Their established parameter IDs remain stable so released presets and host projects continue to load normally.
- Changed DEPTH into a direct 0–100% modulation amount. Zero holds every target at its saved value and 100% supplies the largest available excursion.
- Updated the Randomise tooltip and result message to match its actual scope: delay controls, sequence steps and target slots are randomised, while sequencer controls, power and Freeze remain unchanged.
- Widened the DEPTH and OFFSET captions without moving their sliders, keeping their draggable `::` markers fully inside the sequencer control panel.

### Fixed

- Made every target use its saved knob value as the modulation reference. Bipolar sequencing moves equally above and below that value using the nearer available boundary; Unipolar sequencing moves from that value toward the top of the parameter range.
- Corrected DEPTH and OFFSET processing to operate in the displayed Unipolar 0–1 or Bipolar −1–+1 domain before returning to the sequencer's canonical storage range.
- Fixed Unipolar Pan and other targets being multiplied by their saved normalised value, which confined modulation to a narrow section of controls positioned near their minimum.
- Removed the unequal negative/positive Bipolar scaling that made animation appear to change speed on opposite sides of an off-centre target value. A centred Pan target now sweeps symmetrically from hard left through centre to hard right.
- Advanced saved-state schema to 38 and converted the former signed Peak trim into the equivalent new DEPTH amount. Existing neutral Peak states become 100% DEPTH rather than unexpectedly disabling their sequences.

## 1.59.0 — Sequencer Animation and Converter Calibration — 28 October 2026

### Added

- Added an `ANIMATE` switch to the sequencer. When enabled, assigned delay controls and the assignable Sequencer Attack, Release, Peak and Move controls visually follow the effective modulation value without writing those moving values into the preset or host automation. Disabling it restores the normal static control display immediately.

### Changed

- Moved only the DIFF AMT, DIFF SIZE, MOD AMT and MOD RATE captions three pixels right while retaining their knob and value positions.
- Made the delay-randomisation control square like the preset Save controls and enlarged its bold `!!!` icon.
- Changed the Echo Offset track and centre guide to a contrasting light grey.

### Fixed

- Corrected Deelay PE Duck Release conversion using its logarithmic 10–1000 ms source display; a stored midpoint now converts to 100 ms instead of 659 ms.
- Corrected Deelay PE Mod Rate conversion using its linear 0.02–10 Hz source display; Abstract Concept now converts to 3.0 Hz instead of 0.34 Hz.
- Stopped discarding Deelay PE Spread when its Mono switch is active. Because Deelay's millisecond time offset and Afterglow's spatial Spread are different controls, the converter now preserves the bipolar source position as the closest available approximation and records that distinction in its conversion notes.

## 1.58.0 — Echo Offset, Delay Randomisation and Preset Colours — 28 October 2026

### Added

- Added a dedicated 0–100% Echo Offset parameter and compact horizontal fader. It offsets the reverse/granular read timing relative to the main delay in Chaotic, Pure Reverse, Chaotic Rev and Forward Rev, including their Multi-Tap paths.
- Added the `!!!` delay-randomisation button. It randomises the delay controls, Mode, DIST Type and Echo Offset while deliberately leaving the sequencer, power and Freeze untouched. Random Feedback remains below 100% and random Output never exceeds 0 dB for safer auditioning.

### Changed

- Stored Sequencer HUE, SAT and LUM in plugin/preset state rather than `Settings.ini`, allowing every preset to retain its own sequence colour.
- Moved the closed-browser SEQ button so its right edge aligns with the MIX/OUT panel and placed Echo Offset and Random immediately to its left. The browser-open SEQ position remains unchanged.
- Evenly spaced Save, Save As, Open Folder and SEQ down the browser action column.
- Updated the README release-badge colour and screenshot references to `Afterglow1.jpg`, `Afterglow2.jpg` and `Afterglow3.jpg`.

### Fixed

- Corrected Deelay PE conversion so `preset_echoOffset` maps to the dedicated Echo Offset control instead of being misinterpreted as a Right-Time difference. Both Afterglow time channels now retain Deelay PE's displayed Time value.

## 1.57.3 — Deelay Timing, Tape and Colour Readability — 28 October 2026

### Changed

- Renamed the `DRY / WET` control to `MIX`, including its host-visible parameter name, without changing its stable parameter ID or behaviour.
- Restricted Sequencer LUM to the step display. Assigned-target labels and the HUE, SAT and LUM knob accents now retain a readable mid luminance while continuing to follow HUE and SAT.

### Fixed

- Converted Deelay PE's `preset_timeMs` as its actual skewed 0–1 knob position rather than mistaking it for seconds. Verified source values now reproduce the displayed 4 ms, 19 ms, 20 ms and 40 ms timings.
- Converted Deelay PE's combined Tape motion into both Afterglow Wow and Flutter at two-thirds strength; a 69% source setting now produces approximately 46% on each control rather than being omitted.

## 1.57.2 — Verified Deelay Choice Mapping — 28 October 2026

### Fixed

- Rebuilt Deelay/Deelay PE Mode conversion from the plugin's visible list order and one-based stored item IDs. Normal now remains the clean Digital family, while Rev Forw maps to Forward Rev instead of shifting every source choice by one.
- Replaced positional DIST conversion with an explicit source-to-Afterglow table. This accounts for Deelay's distinct order, including Asym, Tube, Rectify, Samurai and Stressed.
- Stopped changing a Normal Deelay preset into Tape I merely because its independent Tape amount was non-zero. The unsupported overlay is now reported rather than silently changing Mode.
- Lowered only the draggable Left and Right timing captions by two pixels so `:: L` and `R ::` align with the central TIME/NOTE heading.

## 1.57.1 — Label and Converter Corrections — 28 October 2026

### Fixed

- Preserved both colons in every draggable `:: LABEL` marker at small GUI scales while retaining exactly one space between the marker and caption. The caption itself remains centred over its knob, including the Left and Right timing headings.
- Moved the complete HUE/SAT/LUM group left so all three compact knobs and labels remain inside the sequencer panel.
- Clamped LUM to 20–100%, preventing the sequencer accent and assigned-target labels from becoming unreadably dark.
- Corrected Deelay/Deelay PE distortion conversion to the same zero-based indexing used by its mode field.

## 1.57.0 — Sequencer Colour and Self-Modulation — 28 October 2026

### Added

- Added a compact LUM control beside HUE and SAT. It adjusts the luminance of the sequence, target-label and colour-control accent and persists in `Settings.ini`.
- Added Sequencer Attack, Release, Peak and Move as draggable modulation targets. These self-modulating controls use the previous evaluated sequence value, avoiding a circular calculation in the current step.

### Fixed

- Corrected Sixth Sample Deelay/Deelay PE mode conversion to its zero-based order: Normal, Reverse Forward, Pure Reverse, Chaos, Reverse Chaos, Frequency and Pitch.
- Kept draggable parameter captions visually centred over their knobs. Their `::` marker now sits one measured space beside the centred caption without shifting it.
- Removed the transient yellow hover colour from draggable labels, preventing labels crossed during a drag from remaining highlighted.
- Matched Host Sync, Sequencer On and Bipolar typography to the other sequencer labels.

## 1.56.0 — Sequence Tools and Deelay PE Conversion — 28 October 2026

### Added

- Added Sixth Sample Deelay PE `.sspreset` support to the external preset converter, including ZIP archives and ordinary unzipped folder trees. Directly corresponding timing, routing, feedback, stereo, ducking, diffusion, modulation, filtering, distortion and dry/wet settings are translated into Afterglow parameters.
- Added Copy and Paste commands to the step-grid menu so a sequence can be transferred between presets.

### Fixed

- Integrated every draggable `::` marker into its parameter caption with consistent spacing, preventing handles from drifting beyond narrow panel edges.
- Made the sequence Reset command restore the sequence saved in the current preset rather than a generic shape.

### Changed

- Replaced the step-grid context menu with the exact Zero, Max, Min, Random, Reset, Save Sequence, Copy and Paste workflow. Zero respects the bipolar zero crossing, while Min follows the active unipolar or bipolar range.
- Matched the step-grid context menu to Afterglow's mode-coloured popup styling and reduced its text size slightly for the denser command list.
- Updated the preset-conversion batch file and instructions to describe both supported external formats.

## 1.55.4 — Hover Help and Compact SEQ — 27 October 2026

### Fixed

- Added hover-driven footer help to every delay dial, including its label and value editor. The hover text reads the current context-sensitive description, so mode-dependent and Tempo Sync controls remain accurate.
- Restored a step-grid tooltip explaining that steps can be clicked or dragged and that right-click opens the pattern tools.
- Reduced the SEQ show/hide button from 50 to 36 pixels and centred it beneath the preset-browser action column, keeping it inside the browser panel.

### Changed

- Added hover descriptions for delay-mode, distortion, timing, Freeze, Multi-Tap and power controls alongside the existing adjustment/click feedback.

## 1.55.3 — Sequencer Workflow Polish — 27 October 2026

### Fixed

- Integrated the Stereo Time target handles into the visible `:: L` and `R ::` headings, making the complete headings draggable instead of leaving detached handles beside the knobs.
- Repositioned the SEQ show/hide control inside the preset-browser panel while the browser is open, without moving it in the normal delay view.

### Changed

- Replaced the SEQ LED-and-label treatment with the same rounded button styling used by the other compact interface actions.
- Renamed the sequencer `COLOUR` control to `HUE`.
- Changed Nudge to matching square Previous/Next arrow buttons.
- Added concise tooltips to every sequencer control except the editable step grid.
- Assigned delay-target labels now use the live sequencer HUE and SAT colour, immediately identifying every parameter controlled by the sequence.

## 1.55.2 — Sequencer Controls and Clock — 25 October 2026

### Fixed

- Restored dependable target modulation by evaluating the active sequence whenever its ON switch is lit. With no assigned targets the sequence remains harmless, because each parameter still verifies its own target assignment before accepting modulation.
- Increased the interface refresh from 10 Hz to 30 Hz. This removes the visible pause-and-catch-up aliasing between the GUI timer and fast sequence rates while retaining lightweight grid-only repainting.

### Changed

- Reduced Colour to the same compact 30x30 size as Theme, moved it to the upper panel's right edge and added an adjacent `SAT` knob for independent sequence-colour saturation.
- Stored sequence saturation beside sequence hue in `Settings.ini`, independently of audio presets.
- Aligned Host Sync, Waveform, User, the Rate/Start/End groups, range link and Direction around one visual centreline, then reduced the upper panel and expanded-interface heights by six pixels.
- Changed the sequencer ON LED to the established amber Link colour and Host Sync to the established green Tempo Sync colour.
- Replaced the Peak and Move circular thumbs with flat yellow tabs and lengthened their vertical rails.

## 1.55.1 — Sequencer Interface and Timing Targets — 25 October 2026

### Fixed

- Restored the moving current-step highlight whenever the sequencer is enabled, including before a modulation target has been assigned.
- Kept the Stereo and Multi-Tap Time drag handles available under Tempo Sync. A Time target now modulates the displayed millisecond value when Sync is off and the corresponding musical-note division when Sync is on.

### Changed

- Enlarged the upper sequencer panel and returned Colour, Rate, Start, End, Attack and Release to the same 60x57 rotary-control size and shape used by the delay interface.
- Changed the sequencer rotary indicators to Afterglow's established restrained yellow and made slider value text regular weight while retaining bold control labels.
- Removed the separate Waveform and User captions. Their selectors now display `WAVEFORM` and `USER` directly when no item is selected.
- Matched the Waveform, User and Direction selectors to the Mode and Dist Type selectors, including dimensions, fill, text, outline and yellow arrow treatment.
- Increased the expanded interface height to give the enlarged upper controls proper vertical spacing.

## 1.55.0 — Sequencer Correction — 25 October 2026

### Fixed

- Kept delay parameters completely independent of the sequencer unless it is enabled and has at least one active target. The lightweight sequence clock remains available for the visible current-step indicator, while pattern evaluation and parameter modulation are skipped without an active target.
- Replaced the unsupported text glyphs used by target-removal and Start/End link controls with native vector icons, so both display consistently on every host and system font configuration.

### Changed

- Moved Rate, Start, End, range link and Direction into the upper sequencer control panel beside Colour, Waveform and User.
- Moved the centred Nudge controls beneath Attack and Release, enlarged the step area and reduced the expanded interface height.
- Removed step subdivisions and their saved parameters completely. The 32 main steps now occupy the full grid height.
- Matched sequencer label and value typography to the delay interface, narrowed the target list and removed the unnecessary lane-A naming throughout the interface.
- Advanced the saved-state schema to 36. Schema-35 subdivision data is discarded during migration; all main step values, controls and target assignments are retained.

## 1.54.1 — Sequencer Isolation and Layout — 25 October 2026

### Fixed

- Made the sequencer a genuine audio no-op while its switch is off. Stored target assignments now return their untouched Afterglow values instead of being evaluated against a maximum sequence value, eliminating the delay-time and parameter glitches heard with the sequencer disabled.
- Stopped the free-running pattern and its 32-step pattern calculations while the sequencer is disabled. Target assignments and the current pattern remain stored for reactivation.
- Drew the Attack/Release transition envelope over the step bars, including the individual values of two- and three-part subdivisions, so their smoothing is visible before listening.
- Limited Colour to the sequence bars, transition curve and colour knob. Labels, switches, LEDs, general controls and the target list retain their consistent interface colour.

### Changed

- Replaced the Peak and Move rotary controls with the original Seqwencer-style centred vertical sliders and returned them to the right side of the lane-control area.
- Returned the step-nudge buttons to the lower-left lane position and added a `NUDGE` caption.
- Narrowed the target list and used the recovered width for the original lane-control arrangement.
- Removed the separate bottom control panel. Rate, Start, End, range link and Direction now share the enlarged main sequencer panel with the lane controls, steps and targets.

### Compatibility

- Retained state schema 35 and every v1.54.0 sequencer parameter ID. Existing Afterglow presets, user sequences and host automation remain directly compatible.

## 1.54.0 — Sequencer — 25 October 2026

### Added

- Added the complete sequencer workflow from Seqwencer as an expandable lower panel. The new `SEQ` button opens a second panel beneath Afterglow while leaving the established delay-control layout unchanged when closed.
- Added 32 editable steps, individual Half/Two/Three subdivisions, Start/End range, linked range movement, Loop/Bounce/Reverse/Played/Random directions, host sync, 14 musical rates, Bipolar mode, Attack, Release, Peak and Move.
- Added the Seqwencer waveform bank, step reset/random/copy/paste/nudge commands and reusable user sequences stored in `Afterglow/Sequences`.
- Added 16 independently enabled modulation-target slots. Supported Afterglow knob labels can be dragged directly onto the target list, including Stereo and Multi-Tap timing, Feedback, Ducking, Diffusion, Modulation, Pitch, filters, character controls, Dry/Wet and Output.
- Enabled MIDI input for the Played direction, which restarts the sequencer at the beginning of each newly played phrase.
- Saved the sequencer panel state, complete pattern, subdivisions, range, controls and target assignments with plugin states and presets. The interface also remembers whether the lower sequencer panel was open and its A Colour setting.

### Fixed

- Kept the selected preset-browser folder visible after loading a preset, including folders below the currently visible part of a long tree.
- Limited Peak/Move range analysis to subdivisions that are actually active, preventing hidden subdivision values from altering the visible sequence shape.

### Compatibility

- Advanced the saved-state schema to 35. Public schema 32-34 presets are completed with a neutral, unassigned sequencer state when loaded, so existing sounds remain unchanged and cannot inherit modulation targets from the previously loaded preset.
- Seqwencer's second lane, internal FX and PHI integration are deliberately excluded; Afterglow receives only the single sequencer and its parameter-target workflow.

## 1.53.2 — Stable Preset Browser Position — 24 October 2026

### Fixed

- Kept the selected preset folder visible after loading a preset. Loading from the browser now refreshes only the preset list instead of destroying and rebuilding the complete folder tree, so folders lower in a long hierarchy no longer jump back out of view.
- When the folder tree genuinely must be rebuilt, it now scrolls the selected folder back into view after restoring the expanded path and selection.

### Compatibility

- Retained saved-state schema 34, every parameter ID, preset format and all plugin DSP from v1.53.1. This release changes preset-browser navigation only.

## 1.53.1 — Correct Clarity Translation — 24 October 2026

### Fixed

- Corrected the new Clarity conversion from Crystal to Digital + Duck. Crystal remains exclusively Afterglow's renamed counterpart to Valhalla Quartz.
- Retained Clarity's converted Ducking, Mod Amount, Mod Rate, Age, Drive, filtering and all shared timing/feedback parameters while selecting the correct Digital base mode.

### Compatibility

- Retained saved-state schema 34, every parameter ID and all plugin DSP from v1.53.0. Only the one-time Clarity preset translation and displayed version changed.

## 1.53.0 — Complete Mode-Family Preset Translation — 24 October 2026

### Fixed

- Corrected the converter's mode decoder, which previously clamped every Valhalla mode after RevPitch to Pitch Rev. All 17 populated ValhallaDelay 3.0.5 algorithm slots are now recognised.
- Mapped composite Valhalla modes onto existing Afterglow building blocks instead of adding redundant algorithms: BBDuck to Analogue + Duck, Clarity to Crystal + Duck, DuckTape to Chromatic + Duck, PitchDuck to Pitch + Duck, and RichPitch to Pitch Lush.
- Mapped the explicit Valhalla `Ducking` preset value directly to Afterglow's Duck control. The supplied Clarity example now converts its 88.4% Ducking setting to 88.4% Duck instead of 0%.
- Preserved modulation for BBDuck, Clarity, LoFi, Quartz, PhaserDDL and Analog; preserved Wow/Flutter for DuckTape and ChromeTape; and mapped RichPitch's character value to Pitch Mix.
- Reworded preset-browser help for the established single-click loading behaviour and replaced disk-and-pencil wording with the requested `Save As... button` description.

### Compatibility

- Retained saved-state schema 34, every parameter ID and all v1.52.0 DSP. This release changes one-time conversion and interface wording only.

## 1.52.0 — Hard Clip and Preset Workflow — 24 October 2026

### Added

- Added Hard Clip as a clean, symmetrical distortion type with no noise generator or tone filtering. DIST progressively raises the input gain, lowers the effective clipping threshold and blends the bounded result with the dry input.
- Extended the one-time converter and Windows launcher to accept ordinary unzipped preset folders as well as ZIP archives. Folder contents are scanned recursively and their hierarchy is retained.

### Fixed

- Extended Lo Cut from 20-2,000 Hz to 10-2,000 Hz and Hi Cut from 1,000-20,000 Hz to 200-20,000 Hz, matching the complete source ranges. The high-pass stage now bypasses exactly at 10 Hz.
- Updated preset conversion to map both complete filter ranges.
- Aligned the Theme label vertically with the version label and gave both the same centred alignment behaviour.
- Changed the two-column preset browser to load the chosen preset on a single click. Keyboard Return loading remains available.

### Compatibility

- Appended Hard Clip after every existing distortion index, preserving the stored selection of all existing presets and states.
- Retained saved-state schema 34 and every parameter ID. Existing Lo Cut values remain valid without migration.

## 1.51.1 — Preset Conversion Launcher — 24 October 2026

### Fixed

- Shortened the Diff Amt contextual description so its heading and complete help text remain inside the fixed two-line information area at every supported GUI scale.

### Added

- Added a double-clickable Windows preset-conversion launcher and dedicated `Source Presets` / `Converted Presets` folders.
- The launcher uses the project's portable Python 3.15.1 runtime and converts every Valhalla preset ZIP in one run while retaining each bank's folder structure.
- Added concise local instructions for supplying source banks and installing the resulting `.agpreset` folders.

### Compatibility

- Retained saved-state schema 34, every parameter ID and all audio processing from v1.51.0.

## 1.51.0 — Continuous Time-Dependent Processing — 24 October 2026

### Fixed

- Analysed the supplied Time-adjustment recording and confirmed that the audible ticks were short broadband discontinuities during knob movement rather than the intended continuous Doppler pitch sweep.
- Traced the shared fault to secondary delay calculations following the raw block-rate Time parameter while the principal delay read head followed an 80 ms smoothed value. Modulation depth and Diffusion span could therefore jump by several samples at each mouse update even though the main read position remained continuous.
- Made modulation-range scaling, Digital and Crystal modulation safety, LoFi slow/scrape depth and every stereo Diffusion bank derive from the same per-sample effective Time position as the audible delay head.
- Made Multi-Tap Diffusion follow the continuously smoothed shortest active tap and interpolate with the existing Stereo/Multi-Tap transition. Changing a tap time or switching timing layouts no longer bypasses the correction.

### Verification

- Reproduced a 50-1000 ms knob sweep with parameter updates at approximately 62.5 Hz. At 100% Mod Amount, update-boundary peak discontinuity fell from approximately 0.125 to 0.025.
- With Diffusion active, the same boundary peak fell from approximately 0.93 to 0.02. Update boundaries are now comparable with neighbouring signal samples rather than producing isolated broadband spikes.
- Repeated the sweep in Brigade, Digital and Tape modes with clean, modulation-only, diffusion-only and combined settings. Clean Digital timing was already continuous and remains unchanged.

### Changed

- Retained each mode's intended Time character: analogue-style modes still create smooth pitch/varispeed movement, while Digital retains its click-resistant tap crossfade. The correction removes discontinuities without forcing every algorithm to use the same sonic transition.
- Left the v1.49.0 Feedback calibration and v1.50.0 BBD/Analogue compander topology unchanged.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing presets and host states receive continuous Time-dependent processing directly without migration code.

## 1.50.0 — Stable Analogue Compander Feedback — 24 October 2026

### Fixed

- Compared matched 252 ms BBD Modder recordings. The reference decayed continuously to approximately -70 dBFS, while Afterglow initially fell, reversed direction after roughly five seconds and stabilised near -20 dBFS around a 340-350 Hz resonance.
- Traced the self-excitation to the BBD instantaneous compander topology rather than its noise generator, modulation oscillator, preset data or global Feedback curve. Placing stateful filters between square-root compression and square expansion allowed particular filtered waveforms to manufacture loop gain below 100% Feedback.
- Made BBD compression, quantisation, signal-tracked noise and expansion one memoryless colour stage, followed by the two low-pass stages and high-pass stage. The filters now provide predictable loss instead of being enclosed by a nonlinear expander.
- Applied the same stable ordering to Analogue mode and to both modes' Multi-Tap paths, which shared the same latent compander fault.

### Verification

- Offline processor tests at 1, 1.5, 2, 5, 50, 252 and 1000 ms now decay without late re-triggering at BBD Modder's 95.6% Feedback and 100% Mod Amount.
- Verified Stereo BBD, Multi-Tap BBD, Stereo Analogue and Multi-Tap Analogue at 252 ms. All four continued falling through the complete thirty-second test instead of converging on a resonant limit cycle.
- Removed BBD noise entirely in an isolated diagnostic build and reproduced the original runaway unchanged, confirming that suppressing noise would not have corrected the fault. The established signal-tracked noise character therefore remains present.
- Preserved deliberate regeneration above unity: the existing 100-200% global Feedback mapping and its output safety remain unchanged, and a 120% BBD test still sustains a bounded coloured loop.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing BBD and Analogue presets receive the stable compander topology directly without migration code.

## 1.49.0 — Consistent Sub-Unity Feedback Tails — 24 October 2026

### Fixed

- Lengthened the shared Feedback tail below 100% after testing across the converted preset banks showed consistently shorter decays than the source effects, independent of delay mode or individual preset.
- Applied a mathematically uniform decay-duration calibration across the complete 0-100% range. Theoretical tail duration is now approximately 11% longer, adding roughly one, two or three seconds to tails that previously lasted about nine, eighteen or twenty-seven seconds.
- Preserved zero as silence, exact 100% as unity and the complete 100-200% regenerative mapping unchanged. The correction cannot leak into the deliberately controlled above-unity behaviour.

### Changed

- Kept the established conservative upper Feedback curve that replaced the former multi-minute 95% response, then applied the small duration correction to its result. This retains its shape instead of reintroducing a steep approach to unity.
- Kept the correction shared by every mode, timing style and preset. No mode multiplier, preset-name test, extra limiter or output-envelope processing was added.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing presets and host states receive the longer sub-100% decay directly without migration code.

## 1.48.0 — Stable Pitch Regeneration — 24 October 2026

### Fixed

- Compared matched LowLowLow recordings. The reference peaked near -9.4 dBFS and faded to approximately -78 dBFS, while Afterglow climbed to exactly 0 dBFS and remained there as an almost entirely sub-50 Hz signal.
- Extended the established transposition-aware regenerative loss from Pitch Rev to ordinary Pitch. Large downward shifts above 100% Feedback can no longer be held at unity by the generic saturation stage after their energy should have moved below the audible band.
- At LowLowLow's -24-semitone shift and 128.8% displayed Feedback, the correction places the effective recirculation below unity instead of allowing a permanent sub-bass/DC limit cycle. Feedback at or below 100%, zero-shift Pitch sounds, every non-pitch mode and Freeze remain unchanged.
- Corrected the one-time converter so Pitch retains its source Mod Rate and Mod Depth. LowLowLow's 100% modulation is no longer discarded, restoring the moving low-frequency texture instead of feeding a static pitch loop.

### Changed

- Kept both corrections parameter-driven. No preset-name test, fixed LowLowLow output trim, envelope follower or pumping gain control was added.
- Left the v1.47.0 Multi-Tap repeat-head routing and the v1.44.0 signal-tracked analogue-noise behaviour unchanged.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing Pitch states receive the safer regenerative behaviour directly. Converted Pitch presets should be regenerated to recover their source modulation values.

## 1.47.0 — Full-Strength Multi-Tap Regeneration — 24 October 2026

### Fixed

- Corrected the v1.46.0 dedicated repeat-head routing after testing showed no useful restoration of Fold Feeder's metallic Feedback effect.
- Removed audible tap panning from the internal repeat return. The selected head remains panned normally at the plugin output, but now enters both feedback channels at full strength instead of being reduced to approximately 35% and 94% by Tap 4's pan position.
- Preserved the selected head's processed waveform, modulation and level in the loop, while continuing to exclude the coincident four-tap sum that caused the original runaway.

### Changed

- Explicitly separated audible spatial placement from regeneration strength. Moving a Multi-Tap head across the stereo field no longer changes the effective Feedback percentage.
- Left Multi-Tap output level, global Feedback calibration, mode processing and all non-Multi-Tap paths unchanged.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing presets and host states receive the corrected repeat-head gain directly without migration code.

## 1.46.0 — Dedicated Multi-Tap Repeat Head — 24 October 2026

### Fixed

- Replaced v1.45.0's coherence trim with the correct Multi-Tap feedback topology. Turning up Feedback no longer loses Fold Feeder's metallic, ring-modulated movement merely to keep the four-head sum stable.
- Multi-Tap now takes its recirculating signal from the longest active repeat head. All enabled heads remain present at the audible output, but coincident heads cannot multiply the feedback loop because their sum is no longer used as the return source.
- Preserved each repeat head's full processed signal, modulation, level and equal-power pan position in the loop. Feedback therefore remains an audible creative control rather than a safety-scaled copy of the complete Multi-Tap output.
- Smoothed transitions between the summed audible path and dedicated repeat path with the existing Multi-Tap crossfade, avoiding a hard feedback-source switch.

### Changed

- Retained RMS normalisation exclusively for the audible Multi-Tap mix. Feedback routing is now structurally stable instead of being made safe by attenuating the entire summed result.
- Left the global Feedback curve, per-mode saturation, signal-tracked analogue noise and output safety unchanged.
- Kept the implementation shared by every Multi-Tap preset; Fold Feeder receives no preset-name exception.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing Multi-Tap presets and host states use the corrected repeat-head routing directly without migration code.

## 1.45.0 — Coherence-Safe Multi-Tap Feedback — 24 October 2026

### Fixed

- Compared matched Fold Feeder recordings. The reference peaked at approximately -6.25 dBFS and decayed below -80 dBFS, while Afterglow grew to digital full scale and sustained 50 ms sections near -1.7 dBFS.
- Traced the growth to four enabled Multi-Taps collapsing onto the same delay time. The existing RMS tap normalisation was correct for unrelated echoes but allowed coincident tap voltages to add coherently inside the feedback loop, turning the preset's displayed 100.2% Feedback into a substantially regenerative path.
- Added coherence-aware feedback normalisation derived from the active tap levels and equal-power pan gains. Coincident taps can no longer multiply the internal loop gain merely because several reads contain the same waveform.
- Applied the correction to normal stereo, Cross, Ratio and Ping-Pong feedback sources so no routing matrix can bypass it.

### Changed

- Kept the audible Multi-Tap output on its established RMS normalisation. The correction acts only on the recirculating return, so ordinary separated tap patterns do not become needlessly quieter at the plugin output.
- Kept the correction fully parameter-driven and shared by every Multi-Tap preset. Fold Feeder receives no preset-name test, hidden feedback value or dedicated limiter.
- Left the v1.44.0 signal-tracked analogue-noise correction, global Feedback curve, per-mode saturation and output safety unchanged.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing Multi-Tap presets and host states receive the corrected feedback normalisation directly without migration code.

## 1.44.0 — Signal-Tracked Analogue Noise — 24 October 2026

### Fixed

- Compared matched BBD Modder recordings and traced Afterglow's no-input pitched bursts to its permanent BBD noise floor. At the preset's 1 ms delay, 95.6% Feedback and maximum modulation, that continuously injected noise was repeatedly captured by the short resonant loop even though no note had been played.
- Reworked Brigade, Analogue, HiFi and LoFi noise so it follows actual signal activity below unity feedback. Once the delayed signal has decayed, these modes no longer retain an independent excitation source capable of restarting a short comb loop.
- Applied the same signal tracking to Multi-Tap's Brigade, Analogue, HiFi and LoFi noise paths, avoiding a second route to the same idle-loop fault.
- Preserved intentional self-oscillation behaviour: a bounded free-running noise component fades back in only above 100% loop gain and reaches its limited maximum at 125%.

### Changed

- Kept the correction entirely in the shared mode engine. BBD Modder, Feedback Harmor, Fold Feeder and LowLowLow receive no preset-name tests, hidden gain values or individual safety exceptions.
- Left delay timing, modulation depth, Feedback calibration, filtering, saturation and output safety unchanged in this stage so the source of any remaining tonal mismatch can be assessed separately.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing presets and host states receive the corrected noise behaviour directly without migration code.

## 1.43.0 — Unitary Ratio Feedback — 24 October 2026

### Fixed

- Completed the retained Ratio delay style with a true energy-preserving two-channel feedback matrix. Unlike ordinary Dual timing, the left and right returns now continually recombine, so unequal times build progressively denser, pseudo-reverberant echo patterns instead of remaining two unrelated repeat streams.
- Smoothed transitions between ordinary stereo, Ratio and Ping-Pong feedback topologies over the established 20 ms interval, avoiding a hard matrix switch while a tail is active.
- Made Freeze capture a complete left-plus-right Ratio circulation. Coupled Ratio tails no longer freeze as two unrelated channel periods.

### Changed

- Kept Ratio as a general Afterglow routing behaviour rather than a preset-name correction. Converted Ratio presets and host-created Ratio states use the same matrix, with no special cases.
- Replaced the converter note describing Ratio as unfinished; its already-materialised L/R timing now feeds the completed matrix.
- Adopted the supplied README revision with the new Digital and Analogue screenshots ahead of the existing main interface screenshot. No release history was added to the instructional README.
- Deliberately did not add an Era control. Afterglow continues to expose its useful tone-building components directly instead of adding a source-plugin compatibility control primarily for converted presets.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing converted Ratio presets gain the completed matrix immediately; no migration or compatibility branch was added.

## 1.42.0 — Delay-Relative Diffusion and Reliable Freeze — 23 October 2026

### Fixed

- Replaced Freeze's dependence on nominal unity feedback with a dedicated, preallocated wet-cycle capture. Filters, diffusion, pitch processing and mode saturation can no longer make a frozen loop collapse merely because their true loop gain is below one.
- Freeze captures independent stereo periods, a complete two-leg Ping-Pong circuit or the longest active Multi-Tap pattern, then crossfades to decay-free playback over the existing 20 ms smoothing interval. New input remains excluded, the dry path remains live and release returns smoothly to the still-running delay engine.

### Changed

- Rebuilt Diffusion Amount as the coefficient control inside the four-stage allpass cascade instead of a dry/diffused output blend. Values below 1% bypass the network; 68% now occupies the balanced attack/decay region and high values build a genuine diffusion tail.
- Made Diffusion Size relative to the displayed delay. Its span now runs from 2% to 100%, and standard modes shorten the straight delay by the same amount so changing Size does not move the overall echo. Pitch Rev retains the intentional additive-diffusion exception.
- Replaced the former fixed 60 ms diffusion store with four stage-proportional, preallocated buffers capable of the complete delay range. Left and right use different normalised stage spacings, while paired slow modulation scales with Size and cancels its net timing movement.
- Multi-Tap derives one shared span from the shortest enabled tap and subtracts that same span from every active tap, preserving the complete tap pattern rather than changing its spacing.
- Extended host tail reporting with the diffusion coefficient and span, so high-Amount reverb-like decays are not truncated merely because Feedback itself is low or zero.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing presets and host states receive the corrected Freeze and Diffusion behaviour directly; no legacy branch, preset-name exception or migration layer was added.

## 1.41.0 — Delay-Aware Modulation and Style Phase — 23 October 2026

### Fixed

- Replaced the shared fixed-width modulation sweep with delay-aware physical depth. The available range begins at 2.5 ms for near-zero delays, grows with delay time and reaches the established 35 ms maximum at 250 ms, so long-delay motion keeps the 1.39.0 calibration while flanger-length settings remain concentrated in the comb-filter range.
- At Warble Flanger's 1 ms Digital time and 84.7% Mod Amount, the requested excursion is now approximately 1.89 ms rather than 25.1 ms. Its causal forward bias produces an approximately 0.25-4.03 ms moving delay instead of spending most of the cycle in chorus and doubling territory.
- Applied the saved Delay Style to stereo modulation phase: Single uses matched motion, Dual uses 90-degree quadrature, and Ratio and Ping-Pong use 180-degree antiphase motion. Phaser and the shared periodic delay modulation now follow the same Style identity.
- Gave Quad's four tap pairs staggered modulation phases while retaining antiphase left/right motion. Digital and Crystal taps keep independent rounded-triangle state so one head no longer forces every tap through the same movement.
- Retained the 0.25 ms Digital/Crystal read floor by moving only the otherwise-negative portion of a short sweep forwards. Other centred modulation paths reduce only enough to remain above that floor.

### Changed

- Replaced the project README with the supplied badge-and-screenshot edition, kept the screenshot exactly once and removed the release number from its title and introduction. Release history, compatibility and versioning remain in this changelog rather than the instructional README.
- Kept the modulation correction entirely in the shared engine. No preset-name checks or Warble Flanger, Shuffler or Elektrik exceptions were added, and Diffusion, Era conversion and Feedback calibration are unchanged in this stage.

### Compatibility

- Retained saved-state schema 34 and every parameter ID. Existing presets and host states receive the corrected modulation depth and Style phase directly without migration or backward-compatibility code.

## 1.40.0 — Delay Style Foundation and Correct Ping-Pong Routing — 23 October 2026

### Fixed

- Replaced the former Ping-Pong cross-feedback approximation with its dedicated two-leg topology. New input begins on Left, the processed Left output transfers directly to Right, and FDBK is applied only when Right returns to Left. A complete stereo circuit now receives one Feedback multiplication rather than `feedback × feedback` loss.
- Removed the temporary 60-90% global Feedback boost that had compensated for that double attenuation. The established direct response now extends through 80%, while the previously matched near-unity 97-100% decay remains unchanged.
- Kept topology changes click-resistant by interpolating the complete normal-stereo and Ping-Pong loop inputs with the existing smoothed Ping-Pong control.

### Changed

- Added an internal Single/Dual/Ratio/Ping-Pong/Quad Delay Style parameter. The one-time Valhalla converter now preserves the original Style rather than discarding Ratio after materialising its audible timing.
- Timing-page actions keep the internal Style identity synchronised: Link selects Single, ordinary unlinked timing selects Dual, Ping-Pong selects its dedicated style and Multi-Tap selects Quad. Imported Ratio remains identifiable until the user deliberately changes the timing style.
- Regenerated the Richard Devine conversion bank as schema 34 so all thirty presets carry their exact source Style. Ratio timing remains materialised until its dedicated routing matrix is implemented in a later stage.

### Compatibility

- Advanced saved-state schema to 34. Published schema-32/33 states receive the missing controls only; earlier Single, Dual, Ping-Pong and Quad states are inferred from Link/Ping-Pong/Multi-Tap. Previously converted Ratio presets should be replaced by the regenerated bank because schema 33 did not retain that identity.

## 1.39.0 — Shared Modulation and Feedback Calibration — 23 October 2026

### Fixed

- Compared matched Shuffler captures and separated two engine-level differences from the earlier 1 ms Warble Flanger fault. Shuffler uses ordinary 250/500 ms timing, yet Valhalla's principal harmonic moved approximately three times farther than Afterglow's; therefore the remaining weak flange was not caused by the short-delay safety code.
- Increased the shared maximum delay-modulation excursion from 12 ms to 35 ms. Shuffler's 82.4% setting now requests approximately 23.8 ms rather than 8.1 ms, matching the measured pitch excursion much more closely. Digital, Brigade, HiFi and other users of the shared physical modulation depth benefit automatically; Phaser retains its existing normalised 0-100% sweep response.
- Extended the global Feedback calibration down to 60%. Shuffler's correctly converted 73.8% display value now maps to approximately 79.1% loop retention, addressing its measured 1.5-2 dB-per-second premature decay while leaving 0-60%, the matched 97% reference and exact 100% unity unchanged.

### Changed

- Kept all corrections in the shared DSP engine. No preset-name checks, Shuffler-specific multipliers or converted timing substitutions were added.
- Retained the v4 converter and preset bank unchanged: Shuffler already contains the correct 1/8 and 1/4 timing, 73.8% Feedback, 82.4% Mod Amount and 0.27 Hz Mod Rate.

### Compatibility

- Retained saved-state schema 33 and every public parameter ID. Existing presets and host states receive the corrected modulation and Feedback response automatically without migration code.

## 1.38.0 — Digital Flange Modulation and Era Conversion — 23 October 2026

### Fixed

- Restored the full Digital modulation-depth range at flanger-length delay times. The former safety clamp limited modulation to 45% of the base delay, so Warble Flanger's converted 84.7% amount at 1 ms was reduced to 0.45 ms and its spectrum remained nearly static.
- Replaced that truncation with a causal short-delay sweep: normal delays remain centred, while only depth that would cross a 0.25 ms safety floor receives a forward offset. Stereo and Multi-Tap Digital reads now share the same bounded rule without collapsing high-feedback sweeps into a near-zero loop.
- Matched Digital modulation to a smoothly rounded triangle LFO. Linked times use identical left/right motion, while unlinked channels retain a phase offset.

### Changed

- Extended the one-time Valhalla converter with Past/Present/Future parsing and a mode-aware mapping to the closest existing DIST family. The source Drive amount remains exact; conversion never invents extra distortion merely to make Era audible.
- Documented that this Era mapping is an approximation: Valhalla's per-mode hardwired filtering, saturation smoothing and Ghost diffusion topology do not reduce to one universal distortion selector.

### Compatibility

- Retained saved-state schema 33 and every public parameter ID. The Digital change is a DSP correction only; existing presets and host states require no migration or compatibility shim.

## 1.37.0 — Preset UI and Fine Pitch Detune — 23 October 2026

### Fixed

- Increased Pitch Detune resolution from one cent to one tenth of a cent without changing its +/-100-cent range or parameter identity. The one-time Valhalla converter now retains Elektrik's 0.4-cent stereo difference instead of rounding it to zero, restoring the preset's slow pitch-voice beating without adding artificial modulation to other sounds.
- Centred the floppy-disk body in both preset action buttons and enlarged the Save As pencil overlay so the two actions remain distinct at 26x26 pixels.
- Exposed the full two-pixel mode-coloured frame around the current-folder preset popup and matched its type size to the Mode and Dist Type menus.

### Changed

- Matched regular and highlighted Mode/Dist Type popup text to the selectors' text colour, and enlarged the header preset-name text and field slightly.
- Reflowed the full preset browser into two name columns while preserving current-preset marking, keyboard loading and per-cell mouse selection.

### Compatibility

- Retained saved-state schema 33, every existing parameter ID and the complete Detune range. Existing presets and host states load unchanged; the finer interval merely stops sub-cent values from being discarded.

## 1.36.0 — Pitch Rev Regeneration and Preset Workflow — 23 October 2026

### Fixed

- Analysed matching PHI captures of the converted Decimate preset. Valhalla fell from approximately -18 dBFS to -67 dBFS within four seconds, while Afterglow settled into a constant approximately -18 dBFS limit cycle whose energy was concentrated around 44 Hz.
- Added fixed transposition-aware loop loss to Pitch Rev only when Feedback is above unity. At Decimate's -20.26-semitone shift and 126.8% Feedback, the effective loop now falls just below unity instead of feeding a permanent sub-bass oscillator. The trim contains no envelope follower or time-varying gain, does not affect Feedback at or below 100%, fades out during Freeze, and leaves every other mode unchanged.

### Changed

- Replaced the filled preset-navigation triangles with the same two-segment yellow chevrons used by the combo boxes, rotated left and right. Both buttons now have identical 21x26 geometry.
- Clicking the current-preset field now opens a mode-coloured, single-column popup containing the current folder's presets. The list shows at most 20 rows, scrolls when more are present and initially reveals the loaded preset.
- Moved Save and Save As into a compact square-button column beside a narrower full-height preset list. Added a matching folder button that opens the selected preset folder in the operating system.
- Reduced the two save glyphs to fit the same 26x26 body used by the Preset Browser button.

### Compatibility

- Retained saved-state schema 33 and every existing parameter ID. This stage changes DSP calibration and editor workflow only.

## 1.35.0 — RevPitch Splice and Preset UI Refinement — 23 October 2026

### Changed

- Refined Pitch Rev's sub-40 ms grain windows so extremely short splice sizes retain a more distinct reversed, transposed micro-grain character instead of being blurred by the same broad overlap used at normal delay lengths. The window boundaries remain zero-valued and gain-normalised to avoid clicks or level dips.
- Increased free-time precision from whole milliseconds to 0.1 ms for Stereo and Multi-Tap timing. Short Ratio conversions can now retain values such as Decimate's 4.3 ms Right splice instead of rounding to 4 ms; ordinary values at 100 ms and above retain the compact whole-millisecond display.
- Made the normal-view preset name field wheel-scrollable. Its rounded dark styling now matches the Mode and Dist Type selectors, while adjacent previous/next buttons use the same compact body and corner treatment as Freeze with matching yellow triangular arrows.
- Raised and tightened the Momentary/Toggle label beside its LED.
- Removed the contextual-help strip while the preset browser is open and extended the browser panel to the established bottom margin. Save and Save As are now represented by floppy-disk and floppy-disk-with-pencil icons.
- Updated the one-time Valhalla converter to preserve 0.1 ms precision for Ratio and Quad materialisation.

### Compatibility

- Retained saved-state schema 33 and every existing parameter ID. The finer timing interval is accepted by existing states without a translation layer; whole-millisecond values load unchanged.

## 1.34.0 — Preset Navigation and Contextual Help — 23 October 2026

### Added

- Added a bordered current-preset name field with previous/next arrow buttons in the normal knob view. Navigation follows natural filename order within the current preset folder and wraps at both ends.
- Added saved-state reset values for every audio knob. Double-click now restores the value from the loaded preset, most recent Save/Save As, or host-saved state instead of forcing a fixed factory value.

### Changed

- Moved Mode, Dist Type and Freeze left, retained the preset-browser button in its established position, and moved the compact version label directly above Power.
- Replaced the oversized all-mode Feedback, Age and Mod Amount help with concise descriptions of only the currently selected mode, preserving the fixed two-line information area.
- Extended the one-time converter with an explicit Richard Devine bank selection; the completed bank retains the corrected timing, pitch and active-control rules from 1.33.1.

### Compatibility

- Retained saved-state schema 33 and every existing parameter ID. Preset navigation, contextual help and saved reset snapshots add no obsolete parameter or compatibility shim.

## 1.33.1 — Preset Calibration Corrections — 21 October 2026

### Fixed

- Expanded Pitch, Pitch Rev and Pitch Lush Shift from +/-12 to +/-24 semitones so Valhalla Pitch presets retain their complete transposition range. Detune remains +/-1 semitone, exactly equivalent to Valhalla's +/-100 cents.
- Enabled Age in Digital, Pitch and Pitch Rev and modelled it as floating-converter bit-depth reduction, including the Multi-Tap path.
- Cleared all preallocated delay, diffusion, mode, filter, modulation and distortion state on the audio thread after a preset or host state is loaded, preventing stale feedback from the previous state from appearing as a pitched self-noise burst.
- Corrected the one-time converter to interpret `DelayL_Ms` and `DelayR_Ms` as seconds rather than normalized knob positions, and to import only modulation/Wow/Flutter controls active in the selected source mode.

### Compatibility

- Retained saved-state schema 33 and every existing parameter ID. This bug-fix release adds no compatibility shim or obsolete state path.

## 1.33.0 — Ghost Frequency Controls — 21 October 2026

### Added

- Added dedicated saved `Ghost Frequency Shift` and `Ghost Frequency Detune` parameters in Hertz. Ghost Shift is bipolar from -1000 to +1000 Hz with fine control around zero; Ghost Detune adds 0-100 Hz of left/right separation.
- Reused the existing Pitch-panel knobs dynamically: Ghost shows the new Hertz controls, Pitch/Pitch Rev/Pitch Lush retain their semitone controls, and Phaser retains RES/STAGE.
- Applied the same Ghost frequency shifting to Multi-Tap reads instead of bypassing the Ghost shifter on that timing page.

### Compatibility

- Advanced saved-state schema to 33. Schema-32 public presets and host states receive only the two missing Ghost parameters during load; the 14 Hz migrated Detune retains the former fixed +/-7 Hz Ghost sound. No older pre-public schema path or general legacy alias was added.
- Updated the one-time ValhallaDelay converter to schema 33, including Ghost Hertz values and Quad Tap A-D timing/enable-state materialisation into Afterglow Multi-Tap.

## 1.32.1 — Preset Browser Refinement — 21 October 2026

### Changed

- Hid the redundant parent `Presets` row while retaining it as the browser's logical root, so nested folders keep their original tree structure without wasting a visible row.
- Retained state schema 32 and every parameter ID; this patch adds no compatibility shim or obsolete state path.

## 1.32.0 — First Public Release — 21 October 2026

### Changed

- Promoted Afterglow to version 1.32.0 for its first public release.
- Changed Tape I to RGB `61, 100, 6`, Brigade to `100, 35, 70`, HiFi to `193, 69, 33`, Ghost to `130, 67, 110` and Forward Rev to `89, 71, 107`.
- Corrected the visible `Foreward Rev` spelling to `Forward Rev` without changing its DSP, mode index or parameter ID.

### Compatibility

- Retained state schema 32 and the complete saved-state layout. There are no public preset files to migrate, so no conversion layer, legacy alias or obsolete code was added.
- Schema 32 is now the first public compatibility baseline. Future public releases must preserve it or migrate it deliberately when a genuinely incompatible change is required.

## 0.28.0 — Preset Browser and Icon Refinement — 21 October 2026

### Added

- Added a list-icon header button that replaces the inner control panels with a preset browser inside the existing coloured surround without increasing the GUI height.
- Added a recursive 30/70 folder-tree and preset-list layout for `Root\Afterglow\Presets`, preserving nested directory structure and natural name ordering.
- Added `.agpreset` state files, double-click and Return loading, direct Save of the current preset and Save As for new presets or copies. Save automatically invokes Save As until a current preset exists.

### Changed

- Replaced the wide `FREEZE` text control with a compact snowflake icon button using less-rounded corners. Momentary, Toggle and automation behaviour are unchanged.
- Darkened Crystal's cyan mode colour so the contextual help remains legible with Theme at maximum.
- Kept Theme, editor scale and browser navigation outside preset state; saved presets contain only current-schema plugin parameters. The active preset reference is retained only in host-project state so direct Save still targets the right file after the editor is reopened.
- Advanced the state schema to version 32 and updated all project metadata to 0.28.0.

### Compatibility

- This early-development release intentionally ignores state and preset files written by 0.27.0 and older builds. Insert a fresh 0.28.0 instance; no pre-1.0 compatibility code, state converter or obsolete alias was added.

## 0.27.0 — Freeze and Compact Help Layout — 21 October 2026

### Added

- Added a dedicated `FREEZE` button. Freeze smoothly excludes new input from the delay and moves the existing feedback loop to unity without changing the dry signal, selected mode, timing, modulation or stored FDBK setting.
- Added a neighbouring LED selector whose label changes between `MOMENTARY` and `TOGGLE`. Momentary is the default and freezes only while the button is held; Toggle latches and releases Freeze on successive clicks.
- Added host parameters for Freeze state and Freeze interaction mode so both can be automated and restored by a fresh 0.27.0 instance.

### Changed

- Moved the fixed two-line contextual help into the lower-left of the coloured surround and restricted it to 683 design pixels, leaving the area below the final two panels clear for future controls.
- Reduced the complete design height from 326 to 276 pixels after removing the former lower help area. The coloured surround now ends eight pixels above the window edge, matching its eight-pixel left and right margins.
- Shifted the Mode and Dist Type header controls left to make room for Freeze without moving Theme or Power.
- Increased only the active rotary value editor from 14 to 18 pixels high. Each containing slider gained the same four pixels, preserving the normal rotary and value appearance while preventing typed text from being clipped.
- Advanced the state schema to version 31 and updated all project metadata to 0.27.0.

### Compatibility

- This early-development release intentionally ignores state written by 0.26.2 and older builds. Insert a fresh 0.27.0 instance; no pre-1.0 compatibility code, state converter or obsolete alias was added.

## 0.26.2 — Controlled Feedback-Length Correction — 21 October 2026

### Fixed

- Removed the excessively steep 0.26.1 upper Feedback curve. It mapped a displayed 95.5% to approximately 99.4% internal retention, which could leave an audible tail for several minutes.
- Replaced it with one conservative global correction based on the decay relationship of a conventional feedback loop. A displayed 95.5% now maps to approximately 97.7% internal retention and 97% maps to approximately 98.5%.
- Kept 0-80% unchanged, 100% exactly at unity and the existing 100-200% regenerative range unchanged.
- Left every delay mode's filtering, modulation, nonlinear colour, stereo behaviour and feedback processing untouched. The correction changes only the shared Feedback control-to-retention mapping.
- Advanced the state schema to version 30 and updated all project metadata to 0.26.2.

### Compatibility

- This early-development release intentionally ignores state written by 0.26.1 and older builds. Insert a fresh 0.26.2 instance; no pre-1.0 compatibility code, state converter or obsolete alias was added.

## 0.26.1 — Global Feedback-Length Calibration — 21 October 2026

### Changed

- Recalibrated Feedback length globally rather than by delay mode. Modes continue to define how feedback sounds, but all modes use the same control-to-retention mapping.
- Left the complete 0-80% range unchanged.
- Added a continuous smooth calibration from 80-100% based on the matched 1/16-triplet, 97% test: Afterglow decayed in approximately six seconds where ValhallaDelay and Deelay PE both lasted approximately twelve seconds.
- A displayed 90% now maps to 95% internal retention, 95% to approximately 99.2%, 97% to approximately 99.8%, and 100% remains exactly unity. The existing mode processing, 100-200% regeneration and output-only safety are unchanged.
- Advanced the state schema to version 29 and updated all project metadata to 0.26.1.

### Compatibility

- This early-development release intentionally ignores state written by 0.26.0 and older builds. Insert a fresh 0.26.1 instance; no pre-1.0 compatibility code, state converter or obsolete alias was added.

## 0.26.0 — Expressive Feedback Restoration — 21 October 2026

### Changed

- Removed the 0.25.0 shared feedback conditioner because its second saturation stage, bass restraint and reduced regenerative ceiling progressively flattened repeat motion.
- Restored direct traditional 0-100% loop gain and the full displayed 100-200% regenerative mapping to 100-125% internal gain.
- Removed feedback-dependent wet attenuation below 100%, so raising Feedback no longer turns down the entire delayed signal.
- Restricted gradual feedback-level compensation to the genuinely regenerative region above 100% and kept it outside the loop. Existing per-mode nonlinear colour, repeat decay and delay-time varispeed motion now remain intact, while the final static safety curve continues to bound extreme output without compressor pumping.
- Advanced the state schema to version 28 and updated all project metadata to 0.26.0.

### Reference finding

- Analysis of Scott's EA-1 recording and Korg's EA-1 manual confirmed that the hardware's Delay Depth knob raised both delayed-signal level and feedback. The prominent 4:11-4:15 motion also contains delay-time-induced pitch movement of already-circulating repeats. Afterglow retains independent FDBK and Dry/Wet controls rather than silently combining them; Tape I and Tape II provide the continuously moving read head required for that performance technique.

### Compatibility

- This early-development release intentionally ignores state written by 0.25.0 and older builds. Insert a fresh 0.26.0 instance; no pre-1.0 compatibility code, state converter or obsolete alias was added.

## 0.25.0 — Controlled Regeneration and Naming Pass — 20 October 2026

### Changed

- Reworked the complete 100-200% Feedback range as controlled regeneration. It now follows a progressively flattening 100-108% internal-gain curve instead of reaching 125%, while the direct 0-100% response remains unchanged.
- Added a shared, fixed-cost in-loop conditioner with DC/subsonic removal, progressive low-bass restraint and smooth saturation. High feedback can now settle into a bounded drone without uncontrolled bass growth or harsh output clipping.
- Renamed the visible modes to `Tape I`, `Brigade`, `Chromatic`, `Chaotic`, `Pitch Lush`, `Pitch Rev`, `Chaotic Rev` and `Forward Rev`; their DSP mappings remain direct and unchanged.
- Darkened the Mode and Distortion Type popup hover background.
- Made the disabled Right Tempo Sync LED follow the effective Left state while Link L/R is active and visibly dimmed it; unlinking restores the separately stored Right Sync state.
- Advanced the state schema to version 27 and updated all project metadata to 0.25.0.

### Performance and safety

- The regenerative conditioner uses three preallocated state values per channel and constant-time one-pole/nonlinear processing with no audio-thread allocation or locking.
- Preserved finite-value sanitation, per-mode saturation and final output protection as independent safety layers.

### Compatibility

- This early-development release intentionally ignores state written by 0.24.0 and older builds. Insert a fresh 0.25.0 instance; no pre-1.0 compatibility code, state converter or obsolete alias was added.

## 0.24.0 — Traditional Feedback, Tape II and Narrower Layout — 19 October 2026

### Added

- Added `Tape II` as a distinct eighteenth delay algorithm in both Stereo and Multi-Tap timing.
- Added a preallocated, critically damped virtual transport per stereo channel and per tap. Time changes now accelerate, glide and decelerate continuously, with bounded playback-head speed rather than a fixed linear smoothing ramp.
- Added Tape II's independent two-stage head filtering, inherent wow/flutter, level-compensated asymmetric magnetic colour, Age response and controlled feedback return.

### Changed

- Replaced the hidden sub-100% Feedback taper with direct traditional regeneration: 90% now supplies 90% loop gain before mode-specific colour, and 100% remains the sustained threshold.
- Retained the moderated 100-200% control range, which still maps to bounded 100-125% internal drive.
- Changed the visible Mode and Distortion Type items from all capitals to readable title case while keeping established abbreviations and direct parameter mappings.
- Reduced regular knob width from 66 to 60 pixels and the control-centre grid from 78 to 70 pixels. Non-Timing panels were reduced accordingly and the full design width changed from 1004 to 926 pixels without reducing Multi-Tap space.
- Advanced the state schema to version 26 and updated all project metadata to 0.24.0.

### Performance and safety

- Tape II uses fixed preallocated state, no audio-thread allocation or locking, finite-value recovery and an explicit transport-speed bound that prevents the playback head outrunning the delay writer.
- Tape II processing is skipped when its smoothed mode blend is inactive; the existing per-mode feedback saturation, finite-value sanitation and output safety stages remain active.

### Compatibility

- This early-development release intentionally ignores state written by 0.23.0 and older builds. Insert a fresh 0.24.0 instance; no pre-1.0 compatibility code, state converter or obsolete alias was added.

## 0.23.0 — Reverse/Chaos Modes and Compact Popup/Layout Pass — 19 October 2026

### Added

- Added `Chaos`, `Pure Reverse`, `Reverse Chaos` and `Reverse Forward` as four distinct delay algorithms with Stereo and Multi-Tap rendering.
- Added dedicated smoothed random timing/pitch motion for Chaos, original-pitch overlapping reverse grains for Pure Reverse, their combined reverse-chaotic path for Reverse Chaos, and separate forward-audible/reverse-feedback paths for Reverse Forward.
- Added dedicated mode colours, contextual descriptions, smoothed mode blends, grain phases and bounded feedback characters for all four algorithms.

### Changed

- Sorted the visible Mode and Distortion Type menus alphabetically while retaining their direct parameter IDs and DSP mappings.
- Forced both selector popups to open downwards from the control with the first item at the top, regardless of the current selection.
- Inset the popup item drawing by three pixels per side and brightened the hover colour so the selection no longer overlaps the two-pixel mode-coloured border.
- Put tap numbers 1–4 inside the Multi-Tap On LEDs and removed the separate number column.
- Narrowed all non-Timing panels and reduced the design width from 1058 to 1004 pixels, retaining a consistent 78-pixel control-centre grid and leaving Timing unchanged.
- Changed the active Power LED to a clearer green.
- Advanced the state schema to version 25 and updated all project metadata to 0.23.0.

### Performance and safety

- Skips the new delay engines whenever their smoothed blend is inactive and keeps all grain, random-modulation, filter and diffusion state preallocated.
- Bounds Chaos timing and pitch motion, applies mode-specific feedback saturation, and preserves the existing finite-value sanitation and output safety stages.
- Gives Reverse Forward separate diffusion/filter banks for its audible forward and regenerating reverse paths, avoiding state collisions without allocation or locking in the audio callback.

### Compatibility

- This early-development release intentionally ignores state written by 0.22.0 and older builds. Insert a fresh 0.23.0 instance; no pre-1.0 compatibility code or obsolete aliases were added.

## 0.22.0 — Complete Distortion Bank and Multi-Tap Spacing — 19 October 2026

### Added

- Added the remaining seven Distortion Types in one release: `ASYM`, `RAZOR`, `PIXELATED`, `COMB`, `STRESSED`, `SAMURAI` and `FRESH`.
- Added dedicated per-channel state and character for asymmetric transfer shaping, bright ridged drive, sample-rate and bit reduction, short damped comb resonance, compressed folding, dead-zone metallic slicing and high-band excitation.

### Changed

- Moved the Multi-Tap `MULTI` and global `SYNC` controls to the bottom of the Timing panel.
- Raised the four tap rows, added vertical breathing room between them and widened the Time, Pan, Vol and On columns without increasing the panel size.
- Replaced the original multi-choice distortion blend with a direct 20 ms transition between only the previous and newly selected algorithms.
- Advanced the state schema to version 24 and updated all project metadata to 0.22.0.

### Performance and safety

- Runs only the selected distortion algorithm in steady state and at most two algorithms during a type transition, so completing the 15-choice bank does not scale steady-state CPU use with the menu size.
- Preallocated all Pixelated, Comb, filter, noise and DC-blocking state; the audio callback performs no allocation or locking.
- Bounded every new nonlinear path, added deterministic dither/noise where used and applied dedicated DC removal to asymmetric or folded circuits that can create offset.

### Compatibility

- This early-development release intentionally ignores state written by 0.21.0 and older builds. Insert a fresh 0.22.0 instance; no pre-1.0 compatibility code or obsolete aliases were added.

## 0.21.0 — Multi-Tap, Bumpy and Interface Colours — 19 October 2026

### Added

- Added a compact four-tap timing page inside the existing Timing panel. Its `MULTI` LED switches cleanly between the complete stereo page and four rows of Time, bipolar Pan, Volume and On controls without discarding either setup.
- Added one global Multi-Tap Sync LED. Each tap retains separate millisecond and note-division values, so changing the timing unit never destroys the other pattern.
- Added `BUMPY` as the eighth Distortion Type, using a frequency-shaped pre-emphasis path and smooth nonlinear ripples for an intentionally uneven character.

### Changed

- Renamed the visible `Analog` mode to British English `Analogue`, with no obsolete alias or state conversion.
- Changed the active Power light to a slightly yellow-green colour.
- Matched the Link L/R and Ping-Pong caption colour to the muted Theme caption.
- Advanced the state schema to version 23 and updated all project metadata to 0.21.0.

### Performance and safety

- Smoothed Multi-Tap page switching, time movement, pan, volume and each tap On control to prevent hard routing clicks.
- Used fixed RMS tap normalisation instead of an envelope detector, preventing uncontrolled multi-tap summing without gain pumping.
- Kept all four tap states, pitch phases, filters and deterministic noise generators preallocated; the audio callback performs no allocation or locking.
- Bounded and DC-filtered Bumpy with static compensation and the existing click-resistant distortion-type transition.

### Compatibility

- This early-development release intentionally ignores state written by 0.20.0 and older builds. Insert a fresh 0.21.0 instance; no pre-1.0 compatibility code or obsolete aliases were added.

## 0.20.0 — Analogue, Amp and Chrome Tape Age — 19 October 2026

### Added

- Added Analogue as a genuine thirteenth delay algorithm: a higher-bandwidth, lower-noise BBD-style path with fine companding texture, restrained clock motion and thicker asymmetric saturation.
- Added `AMP` as the seventh Distortion Type, using cascaded asymmetric Class-A-style preamp and power-stage shaping, an amp-like high-frequency roll-off and dedicated DC removal.
- Added a copper-brown Analogue mode colour and dedicated contextual descriptions for the new mode and distortion.

### Changed

- Strengthened Chrome Tape's AGE range so it now moves audibly from bright, open and clean to darker, more compressed head wear while retaining less transport movement than Tape.
- Moved the version label to approximately 30 pixels from the visible right edge of the complete logo artwork.
- Advanced the state schema to version 22 and updated all project metadata to 0.20.0.

### Performance and safety

- Kept Analogue's filters, deterministic noise and saturation state fully preallocated and skipped its substantial processing while its smoothed mode blend is inactive.
- Kept Amp bounded, statically gain-compensated and DC-filtered with no allocation, locking, oversampling buffer or envelope-driven gain pumping in the audio callback.

### Compatibility

- This early-development release intentionally ignores state written by 0.19.0 and older builds. Insert a fresh 0.20.0 instance; no pre-1.0 compatibility code or obsolete aliases were added.

## 0.19.0 — Chrome Tape, Lightning and Naming Cleanup — 18 October 2026

### Added

- Added Chrome Tape as a genuine twelfth delay algorithm with its own clean continuous-tape path, brighter two-stage head filtering, restrained inherent wow/flutter and higher controlled feedback headroom.
- Added `LIGHTNING` as the sixth Distortion Type, combining bounded hard clipping with signal-tracked white noise, dedicated DC removal and the established 20 ms type transition.
- Added a centred `v0.19.0` label immediately to the right of the supplied logo and aligned it with the Mode and Dist Type labels.
- Added a silver-blue Chrome Tape mode colour and dedicated contextual descriptions for the new mode and distortion.

### Changed

- Renamed the active RichPitch choice and interface text to `LushPitch`.
- Renamed the active Crackly distortion choice and interface text to `Crackle`.
- Shifted the Mode and Dist Type controls right to preserve clear spacing around the new version label.
- Advanced the state schema to version 21 and updated all project metadata to 0.19.0.

### Performance and safety

- Kept Chrome Tape's filter memory independent from Tape so mode transitions cannot expose stale state, while retaining fixed preallocated storage and click-resistant mode blending.
- Kept Lightning bounded, DC-filtered and statically gain-compensated, with deterministic per-channel noise state and no allocation, locking or envelope gain pumping in the audio callback.

### Compatibility

- This early-development release intentionally ignores state written by 0.18.0 and older builds. Insert a fresh 0.19.0 instance; no pre-1.0 compatibility code or obsolete aliases were added.

## 0.18.0 — Phaser Controls, RichPitch and Crackly — 18 October 2026

### Added

- Added dedicated host-automatable Phaser Resonance and Phaser Stages parameters. The stage selector provides exactly 2, 4, 6, 8, 10 and 12 stages.
- Added RichPitch as an eleventh delay algorithm, combining independent shifted and stereo-detuned read-head pairs at one delay time through a dedicated P MIX control.
- Added `CRACKLY` as the fifth Distortion Type, with deterministic signal-responsive crackle bursts, compact saturation, DC removal and the established 20 ms type transition.
- Added a rose RichPitch mode colour and mode-specific contextual descriptions.

### Changed

- Shortened the visible `Phaser DDL` mode name to `Phaser`.
- Kept AGE visible but disabled in Phaser; the Pitch panel now changes from SHIFT/DETUNE to active RES/STAGE controls only in that mode.
- Replaced RichPitch's AGE position with its dedicated P MIX control while retaining SHIFT and DETUNE in the Pitch panel.
- Moved only the Theme label down by two pixels, leaving the Theme knob and Power button unchanged.
- Advanced the state schema to version 20 and updated all project metadata to 0.18.0.

### Performance and safety

- Kept all twelve Phaser stages continuously warm and crossfaded the selected even-stage outputs during stage changes, preventing cold-state bursts and hard switching clicks.
- Smoothed Resonance, Stage and Pitch Mix automation and retained bounded Phaser resonance, feedback saturation, finite-value sanitisation and output safety limiting.
- Used fixed preallocated RichPitch phase state and deterministic per-channel Crackly state, with no allocation or locking in the audio callback.

### Compatibility

- This early-development release intentionally ignores state written by 0.17.0 and older builds. Insert a fresh 0.18.0 instance; no pre-1.0 compatibility code was added.

## 0.17.0 — Crystal, Phaser DDL, Rectify and Logo — 18 October 2026

### Added

- Added Phaser DDL as a genuine tenth delay algorithm. Its twelve-stage stereo phaser sits inside the delay feedback path, Mod Amount/Rate control a quadrature notch sweep, and Age becomes a visible `RES` control for internal phaser resonance.
- Added `RECTIFY` as the fourth Distortion Type, with full-wave folding, dedicated 18 Hz DC removal, static output compensation and the established 20 ms type transition.
- Embedded Artvera's supplied transparent 701×167 Afterglow logo into the plugin binary and used the complete uncropped artwork in the header.
- Added an indigo mode colour for Phaser DDL.

### Changed

- Renamed Quartz to Crystal throughout the current parameter choice, interface, contextual help and documentation. The underlying bright algorithm is unchanged.
- Changed the Mode and Distortion Type arrows to the same warm yellow as the knob indicators.
- Added a two-pixel current-mode-colour border to the popup lists opened from both header selectors.
- Made Theme and Power matching 30×30 controls, aligned them on the same lower header baseline and retained the Theme label's original position.
- Advanced the state schema to version 19 and updated all project metadata to 0.17.0.

### Performance and safety

- Kept Phaser DDL and Rectify allocation-free in the audio callback and skipped Phaser DDL's twelve-stage network while its smoothed mode blend is inactive.
- Bounded Phaser DDL's sweep frequency and resonance, soft-saturated its internal resonant return and retained the main feedback/output safety stages.
- Continuously warmed Rectify's DC-blocker state across type crossfades to avoid switching thumps or accumulated offset.

### Compatibility

- This early-development release intentionally ignores state written by 0.16.0 and older builds. Insert a fresh 0.17.0 instance; no pre-1.0 compatibility code was added.

## 0.16.0 — Quartz, Tube and Timing LED Colours — 18 October 2026

### Added

- Added Quartz as a genuine ninth delay algorithm with a pristine bright digital path, smoothed triangle modulation and a nearly colourless soft-knee feedback limiter.
- Added a dedicated Quartz diffusion state so transitions cannot expose stale diffusion memory belonging to another mode.
- Added `TUBE` as the third Distortion Type, using an asymmetric biased transfer curve for rounded valve-like warmth and compression.
- Added a cyan Quartz mode colour and contextual descriptions for Quartz and Tube.

### Changed

- Changed the Left and Right Tempo Sync LED colour to green and the Link L/R LED colour to amber. Ping-Pong retains its existing teal colour.
- Applied Quartz Low Cut, High Cut and steep Age-controlled low-pass filtering only to the audible return, outside the feedback loop, so successive repeats retain brightness.
- Extended Modulation and Age contextual help to describe their Quartz-specific behaviour.
- Advanced the state schema to version 18 and updated all project metadata to 0.16.0.

### Performance and safety

- Kept Quartz and Tube allocation-free in the audio callback and skipped their substantial work while their smoothed algorithm/type blends are inactive.
- Used a 20 ms three-way transfer-curve crossfade for click-resistant Distortion Type changes.
- Bounded Quartz delay modulation, retained finite-value sanitisation and used separate controlled saturation for its bright feedback path.

### Compatibility

- This early-development release intentionally ignores state written by 0.15.1 and older builds. Insert a fresh 0.16.0 instance; no pre-1.0 compatibility code was added.

## 0.15.1 — Theme, LoFi and Feedback Correction — 18 October 2026

### Fixed

- Made the seven-pixel colour strip permanently use the selected mode's exact colour at full opacity. Theme now fades only the surround behind it; at maximum the surround reaches the same colour and the unchanged strip blends into it naturally.
- Moved the independent `LED — TEMPO SYNC — LED` row directly beneath the Time knobs and above Link L/R and Ping-Pong.
- Corrected an unintended small-signal gain increase in LoFi's asymmetric nonlinear stage that made the mode sound mainly louder and longer rather than convincingly worn.
- Replaced Feedback's direct percentage-to-loop-gain mapping with a musical taper. A displayed 90% now produces approximately 78% internal regeneration, 100% remains the sustained threshold, and 100-200% maps to a bounded 100-125% driven range.

### Changed

- Strengthened LoFi's permanent two-stage bandwidth wear, stereo-decorrelated random pitch drift, inherent wow/flutter, scrape motion, coarse resolution and signal-tracking hiss.
- Added irregular tape-level motion to LoFi and normalised its asymmetric saturation around small signals so its tone changes without artificially extending the tail.
- Added static feedback-position output compensation to restrain dense tails without a detector, compressor or pumping behaviour.
- Updated contextual help and the test checklist for the corrected LoFi and Feedback behaviour.
- Advanced the state schema to version 17 and updated all project metadata to 0.15.1.

### Performance and safety

- Retained allocation-free audio processing, bounded LoFi delay modulation and finite-value sanitisation.
- Kept all feedback protection memoryless or parameter-derived so the level control cannot pump.

### Compatibility

- This early-development release intentionally ignores state written by 0.15.0 and older builds. Insert a fresh 0.15.1 instance; no pre-1.0 compatibility code was added.

## 0.15.0 — LoFi, Ping-Pong and Portable Interface Settings — 18 October 2026

### Added

- Added LoFi as a genuine eighth delay algorithm with two-stage worn-tape filtering, asymmetric odd/even harmonic colour, age-dependent resolution, signal-tracking noise and controlled feedback saturation.
- Added stereo-decorrelated random modulation for LoFi. Mod Amount and Mod Rate control the slow chaotic drift, while Age introduces faster scrape flutter and progressively rougher texture.
- Added a host-automatable Ping-Pong LED switch. It smoothly sums the delayed input into the Left tap, removes the initial Right feed and forces alternating cross-feedback without duplicating the Cross knob's normal stereo-input behaviour.
- Added separate host-automatable Left and Right Tempo Sync LED controls in a centred `LED — TEMPO SYNC — LED` row.
- Added fixed-aspect bottom-right GUI resizing from 75% to 200%.
- Added portable `Root\Afterglow\Settings\Settings.ini` persistence for GUI scale and Theme, and created `Root\Afterglow\Presets` ready for preset save/load and browsing.

### Changed

- Link L/R now applies the complete Left timing state to Right while linked. Right's independent milliseconds, division and Tempo Sync state remain untouched and return when unlinked.
- Moved Ping-Pong into the former shared Tempo Sync position and placed the two channel-sync LEDs directly beneath their respective Time knobs.
- Moved Theme out of the audio parameter tree so it remains a global interface preference rather than being stored in audio presets or projects.
- Made the foreground colour strip interpolate into the same surround colour and lose its distinct glow at full Theme. It remains drawn in front but becomes visually indistinguishable from the background.
- Added a teal LoFi mode colour and updated the contextual help for LoFi, Ping-Pong and independent sync.
- Advanced the state schema to version 16 and updated all project metadata to 0.15.0.

### Performance and safety

- Kept LoFi, Ping-Pong and all timing switching allocation-free in the audio callback.
- Smoothed Ping-Pong routing over 20 ms, bounded both LoFi modulation depths against their delay times, and retained finite-value sanitisation plus per-mode feedback saturation.
- Used low-cost interpolated random segments for chaotic modulation and skipped LoFi filter, noise and waveshaping work while its algorithm blend is inactive.
- Debounced settings writes so resize and Theme dragging do not write the INI file continuously.

### Compatibility

- This early-development release intentionally ignores state written by 0.14.0 and older builds. Insert a fresh 0.15.0 instance; no pre-1.0 compatibility code was added.

## 0.14.0 — RevPitch, Saturation and Theme Spacing — 18 October 2026

### Added

- Added RevPitch as a genuine seventh delay algorithm using two complementary Hann-windowed reverse grains per channel.
- Made Left and Right Time control their respective RevPitch splice lengths while Shift and Detune control reverse playback pitch.
- Placed the reversed, transposed result inside feedback so the effect accumulates across echoes.
- Added `SATURATION` as the second Distortion Type, using a separate open arctangent curve, gentler gain and static level compensation.
- Added a violet RevPitch mode colour and contextual descriptions for the new mode and distortion.

### Changed

- Replaced the Pitch phase-increment smoother with pitch-ratio smoothing shared safely by Pitch and RevPitch.
- Reversed Theme direction: maximum is now the full mode-coloured surround and minimum is charcoal.
- Removed the Theme knob's numerical value display and made double-click restore full colour.
- Kept the colour strip seven pixels thick, made it permanently bright and painted it over the fading surround.
- Added a two-pixel gap between the strip and panels, and extended the surround to create matching ten-pixel top, left and right panel insets.
- Advanced the state schema to version 15 and updated all project metadata to 0.14.0.

### Performance and safety

- Kept both new audio paths allocation-free, selected Distortion Type only once per audio block and added a 20 ms transfer-curve crossfade for click-resistant type changes.
- Bounded reverse splice length and pitch ratio, used complementary windows and interpolated delay reads, and sanitised RevPitch before feedback.
- Retained the 20 ms algorithm transition and skipped inactive RevPitch grain processing.

### Compatibility

- This early-development release intentionally ignores state written by 0.13.0 and older builds. Insert a fresh 0.14.0 instance; no pre-1.0 compatibility code was added.

## 0.13.0 — Adjustable Pitch and Dual Theme Treatment — 18 October 2026

### Added

- Added a host-automatable bipolar Pitch Shift control from -12.00 to +12.00 semitones, with +7.00 as the initial value that preserves the previous Pitch sound.
- Added a host-automatable bipolar Pitch Detune control from -1.00 to +1.00 semitone. It offsets Left and Right equally around Shift and reverses the channel relationship for negative settings.
- Added a dedicated vertical Pitch panel with centre detents and double-click zero resets for both controls.
- Added a host-automatable Theme control beside Power. It fades continuously from the full mode-coloured surround to a charcoal surround with the supplied thin, rounded mode-colour strip.

### Changed

- Replaced the fixed Pitch increment with separately smoothed left and right read-head increments derived from Shift and Detune.
- Made exact 0-semitone Pitch settings return the direct delayed read without overlapping-window combing.
- Changed every rotary indicator to a restrained warm yellow while preserving the existing LED, selector and disabled-state colours.
- Expanded the fixed interface to 1058 pixels to fit the Pitch panel while retaining equal six-pixel outer panel insets and keeping Dry/Wet–Output rightmost.
- Moved the control-panel tops down two pixels to give the new mode strip its own clean seven-pixel band.
- Advanced the state schema to version 14 and updated all project metadata to 0.13.0.

### Performance and safety

- Kept Pitch processing allocation-free and calculated parameter targets once per audio block.
- Smoothed both channel increments sample by sample, bounded every delay read and retained finite-value sanitisation plus controlled feedback saturation.
- Kept the 20 ms click-resistant algorithm transition and disabled Pitch read-head work whenever its blend is inactive.

### Compatibility

- This early-development release intentionally ignores state written by 0.12.0 and older builds. Insert a fresh 0.13.0 instance; no pre-1.0 compatibility code was added.

## 0.12.0 — Pitch Mode and Distortion Selector — 18 October 2026

### Added

- Added Pitch as a genuine sixth delay algorithm.
- Added two overlapping, Hann-windowed read heads per channel for stable +7-semitone transposition.
- Placed the Pitch result inside the feedback path so the interval accumulates across successive echoes.
- Added a host-automatable Distortion Type choice parameter and a header combo box beside Mode.
- Added the existing level-compensated input saturation as the first `SOFT CLIP` distortion type.
- Added a magenta interface colour and two-line contextual description for Pitch mode.

### Changed

- Renamed the visible Drive amount control to `DIST` and its host-facing name to `Distortion Amount`.
- Narrowed the Timing panel and complete fixed interface by 18 pixels while preserving matching six-pixel outer insets.
- Reduced the horizontal space between the Left and Right Time columns by 10 pixels.
- Moved the complete upper knob row up three pixels and the lower knob row down four pixels.
- Advanced the state schema to version 13 and updated all project metadata to 0.12.0.

### Performance and safety

- Kept Pitch processing allocation-free and bypassed it whenever its mode blend is inactive.
- Used bounded interpolated delay reads, complementary overlap windows, finite-value sanitisation and a controlled Pitch feedback saturation stage.
- Retained the existing 20 ms mode crossfade for click-resistant transitions into and out of Pitch.

### Compatibility

- This early-development release intentionally ignores state written by 0.11.0 and older builds. Insert a fresh 0.12.0 instance; no pre-1.0 compatibility code was added.

## 0.11.0 — Diffusion and Global Modulation — 17 October 2026

### Added

- Added smoothed, host-automatable Diffusion Amount and Diffusion Size controls from 0% to 100%.
- Added a four-stage stereo allpass diffusion network with independently offset left/right delay lengths.
- Added smoothed, host-automatable Mod Amount from 0% to 100% and Mod Rate from 0.02 to 10 Hz.
- Added quadrature stereo modulation across Digital, Tape, BBD, HiFi and Ghost.
- Added dedicated vertical Diffusion and Modulation panels.

### Changed

- Placed diffusion before the feedback matrix so its smear accumulates naturally across repeats.
- Constrained global modulation depth relative to very short delay times, preventing the moving read position from repeatedly reaching the one-sample floor.
- Centred the complete LED-and-label artwork for Link L/R and Tempo Sync inside their knob-aligned bounds.
- Expanded the fixed interface width to 992 pixels while retaining matching six-pixel coloured margins.
- Advanced the state schema to version 12 and updated all project metadata to 0.11.0.

### Performance and safety

- Allocated and cleared all diffusion memory in `prepareToPlay`; the audio callback remains allocation-free.
- Used bounded allpass coefficients, interpolated reads, parameter smoothing and finite-value sanitisation throughout the diffusion path.
- Disabled the global modulation oscillator work while Mod Amount is effectively zero.

### Compatibility

- This early-development release intentionally ignores state written by 0.10.0 and older builds. Insert a fresh 0.11.0 instance.

## 0.10.0 — Wet-Return Ducking and Stacked Panels — 17 October 2026

### Added

- Added smoothed, host-automatable Duck Amount from 0% to 100%.
- Added Duck Attack from 1 to 500 ms and Duck Release from 20 to 3000 ms.
- Added a stereo-linked peak envelope driven by the main dry input.
- Added a dedicated lower Ducking panel containing DUCK, ATTACK and RELEASE.

### Changed

- Applied ducking only to the audible wet return after stereo Spread and output safety processing. The dry path and internal feedback signal are not attenuated, so delay tails continue regenerating while ducked.
- Restored FDBK, CROSS and SPREAD to one horizontal row inside a reduced-height upper panel aligned with Hi Cut.
- Aligned the matching DUCK, ATTACK and RELEASE panel with Lo Cut.
- Moved both Time knobs and every other upper-row knob up four pixels while leaving all lower-row knobs in place.
- Centred Link L/R under Left Time and Tempo Sync under Right Time.
- Increased the fixed interface width to 824 pixels while preserving matching six-pixel coloured margins at both sides.
- Advanced the state schema to version 11 and updated all project metadata to 0.10.0.

### Performance and safety

- Kept duck detector and gain processing allocation-free inside `processBlock`.
- Smoothed the amount and time-derived envelope coefficients for click-resistant automation.
- Sanitised and bounded the detector envelope and final duck gain.

### Compatibility

- This early-development release intentionally ignores state written by 0.9.0 and older builds. Insert a fresh 0.10.0 instance.

## 0.9.0 — Knob Timing and Ghost Mode — 17 October 2026

### Added

- Added Ghost as a genuine fifth delay algorithm rather than a menu-only variation.
- Added a four-stage quadrature network and efficient recursive oscillator for opposite left/right frequency shifting.
- Added independent Ghost filtering, saturation, continuous delay movement and feedback-path processing.
- Added a purple Ghost interface colour and contextual description.

### Changed

- Removed the separate Left and Right Note combo boxes.
- Made both Time knobs attach directly to millisecond parameters with Tempo Sync off and note-division parameters with Tempo Sync on.
- Preserved all four independent timing values—left/right milliseconds and left/right divisions—when switching Sync or Link.
- Changed the timing divider dynamically between `L — TIME — R` and `L — NOTE — R`.
- Aligned the two Time knobs with the upper Hi Cut row.
- Reflowed FDBK and CROSS onto the upper row with SPREAD centred beneath them.
- Reduced the feedback panel and overall interface width while keeping Dry/Wet–Output at the far right.
- Corrected the coloured frame so the control panels have matching six-pixel left and right insets.
- Added `ms` to the free-time knob value displays.
- Reordered synced divisions from shortest to longest so clockwise knob movement always increases delay time.
- Advanced the state schema to version 10 and updated all project metadata to 0.9.0.

### Performance and safety

- Kept Ghost processing allocation-free inside `processBlock`.
- Used a recursive quadrature oscillator with periodic normalisation instead of per-sample trigonometric calls.
- Retained finite-value sanitisation, smoothed mode transitions, controlled feedback saturation and output safety limiting.

### Compatibility

- This early-development release intentionally ignores state written by 0.8.0 and older builds. Insert a fresh 0.9.0 instance.

## 0.8.0 — Stereo Spread and Timing Clarity — 17 October 2026

### Added

- Added a smoothed, host-automatable bipolar Spread control from -100% to +100% with a centre detent and double-click reset.
- Added a dedicated FDBK–CROSS–SPREAD panel.
- Added mode-coloured panel backgrounds: Digital blue, Tape green, BBD red and HiFi orange.

### Changed

- Applied Spread to the audible wet mid/side image after the feedback path, preserving Cross timing and regeneration while controlling stereo extent.
- Made timing ownership explicit: millisecond knobs are active only with Tempo Sync off, and Note selectors are active only with Tempo Sync on.
- Preserved both free-time and synced right-channel settings through Link L/R changes.
- Removed the `STEREO DELAY / sl23` subtitle and moved the Mode selector into that header position.
- Moved Dry/Wet–Output to the rightmost panel to reflect its final position in the audible signal chain.
- Kept Hi Cut above Lo Cut and widened the fixed interface by 22 pixels to maintain even panel spacing.
- Advanced the state schema to version 9 and updated all project metadata to 0.8.0.

### Fixed

- Added a 100 ms same-direction event guard to each combo box, preventing duplicate operating-system wheel events from producing two-item jumps while retaining immediate first-notch response.

### Compatibility

- This early-development release intentionally ignores state written by 0.7.1 and older builds. Insert a fresh 0.8.0 instance.

## 0.7.1 — Audible Mode Character and Wheel Fix — 17 October 2026

### Fixed

- Corrected Digital, Tape and HiFi sounding almost identical when Drive, Age, Wow and Flutter were low.
- Strengthened BBD's inherent identity so it is clearly darker and grainier without relying on extreme Age settings.
- Replaced JUCE's accumulated combo-box wheel behavior, which could ignore the first movement or jump two items, with exactly one item change per wheel event.
- Reset only the newly selected mode's private filter/read-head memory during a switch, preventing stale inactive state from reappearing while preserving the shared delay tail.

### Changed

- Kept Digital as the clean, stable reference path.
- Added inherent two-stage head loss, soft saturation and delay-dependent drift to Tape.
- Added stronger two-stage, delay-dependent bandwidth loss, coarser clock resolution, compander texture and saturation to BBD.
- Added restrained bandwidth shaping, gentle saturation, asperity and smaller reel-to-reel drift to HiFi.
- Smoothed Age before using it for nonlinear mode character.
- Reduced analogue-mode feedback headroom progressively from HiFi through Tape to BBD so their differences accumulate naturally across echoes.
- Swapped the filter control positions so Hi Cut is above Lo Cut.
- Advanced the state schema to version 8 and updated all project metadata to 0.7.1.

### Compatibility

- This early-development release intentionally ignores state written by 0.7.0 and older builds. Insert a fresh 0.7.1 instance.

## 0.7.0 — HiFi Mode and Grouped Interface — 17 October 2026

### Added

- Added straight, dotted and triplet 1/64 Tempo Sync choices.
- Added HiFi as a fourth, distinct delay mode with bright full-bandwidth studio-tape movement, restrained Age-controlled asperity texture and gentler feedback saturation.
- Extended click-resistant 20 ms mode switching across Digital, Tape, BBD and HiFi.
- Added dedicated panels for Dry/Wet–Output, Lo Cut–Hi Cut and Drive–Age–Wow–Flutter.

### Changed

- Reworked Link L/R so the right controls mirror the left while linked without overwriting the independent Right Time and Right Note parameters; unlinking restores their previous settings.
- Halved the gap between the Note selectors from 24 to 12 pixels and narrowed the timing knob spacing and panel width accordingly.
- Shortened the visible control names to `FDBK`, `LO CUT`, `HI CUT` and `OUT`.
- Inverted the top-right indication so the light means Power is on; the underlying host Bypass parameter retains its conventional meaning.
- Enabled Age in HiFi mode for subtle asperity texture.
- Advanced the state schema to version 7 and updated all project metadata to 0.7.0.

### Compatibility

- This early-development release intentionally ignores state written by 0.6.0 and older builds. Insert a fresh 0.7.0 instance.

## 0.6.0 — BBD Delay Mode — 17 October 2026

### Added

- Added BBD as a third, genuinely distinct delay algorithm.
- Added continuous pitch-changing BBD delay movement rather than Digital mode's clean read-head crossfade.
- Added per-channel delay- and Age-dependent BBD bandwidth limiting.
- Added compander behaviour, clock quantisation and low-level signal-tracking noise.
- Added independent BBD processing memory and controlled feedback saturation.
- Added a three-way 20 ms mode crossfade for click-resistant switching among Digital, Tape and BBD.

### Changed

- Enabled Age for both Tape and BBD; Digital continues to disable it.
- Moved every Timing control down four pixels, then moved the Note heading and selectors down four additional pixels.
- Replaced the dot-ended Timing headings with `L — TIME — R` and `L — NOTE — R`, centred over the corresponding controls.
- Increased the Timing panel height to preserve lower padding after loosening the layout.
- Advanced the state schema to version 6 and updated all project metadata to 0.6.0.

### Compatibility

- This early-development release intentionally ignores state written by 0.5.3 and older builds. Insert a fresh 0.6.0 instance.

## 0.5.3 — Timing-Panel Polish — 17 October 2026

### Added

- Added dotted and triplet 1/32 Tempo Sync divisions.
- Enabled mouse-wheel selection on Mode and both Note combo boxes.

### Changed

- Replaced the Bypass graphic with a properly centred hardware-style power symbol while retaining its subtle LED glow.
- Replaced the redundant Timing and per-control labels with compact line-and-dot `TIME` and `NOTE` dividers.
- Moved all Timing controls upward, reduced the panel height and made its left and right margins symmetrical.
- Advanced the state schema to version 5 and updated all project metadata to 0.5.3.

### Compatibility

- This early-development release intentionally ignores state written by 0.5.2 and older builds. Insert a fresh 0.5.3 instance.

## 0.5.2 — Compact Timing and Bypass — 17 October 2026

### Added

- Added a host-automatable power-style Bypass button at the top-right of the interface.
- Added a 20 ms dry/processed crossfade to prevent bypass switching clicks.
- Kept the delay engine running while bypassed so existing tails remain available when processing is re-enabled.
- Added double-click reset to the 50% centre of Dry/Wet.

### Changed

- Narrowed both Timing note selectors and reduced the complete Timing panel width.
- Replaced checkbox graphics for Link L/R and Tempo Sync with LED status controls.
- Advanced the state schema to version 4 and updated all project metadata to 0.5.2.
- Clarified the development order: ValhallaDelay functional coverage, then Deelay PE expansion, then LFO and possible Seqwencer-style target sequencing.

### Compatibility

- This early-development release intentionally ignores state written by 0.5.1 and older builds. Insert a fresh 0.5.2 instance.

## 0.5.1 — Interface and Feedback-Level Refinement — 17 October 2026

### Changed

- Converted Cross, Dry/Wet, Drive, Age, Wow and Flutter from decimal 0-1 displays to percentage 0-100% displays.
- Added a 50% centre detent and centre-origin indicator arc to Dry/Wet.
- Added a 0 dB centre detent and centre-origin indicator arc to Output.
- Applied a centre-skewed Output range so 0 dB is physically centred while retaining the -24 to +12 dB range.
- Grouped Left Time, Right Time, Left Note, Right Note, Link L/R and Tempo Sync inside a dedicated Timing panel.
- Advanced the state schema to version 3 and updated all project metadata to 0.5.1.

### Fixed

- Made the complete Right Time knob render in neutral grey while Link L/R disables it.
- Added a smooth memoryless ceiling to the audible wet return so regenerative feedback cannot produce an extreme uncontrolled output level.
- Added static Feedback-position output compensation above 100%, without an envelope follower or compressor, so the safety stage cannot create pumping.

### Compatibility

- This early-development release intentionally ignores state written by 0.5.0 and older builds. Insert a fresh 0.5.1 instance.

## 0.5.0 — Linked Timing and Accumulating Feedback — 17 October 2026

### Added

- Added a host-automatable Link L/R switch.
- Linked Right Time to Left Time in free-time mode.
- Linked Right Note to Left Note in Tempo Sync mode.
- Disabled the right timing controls while linked and kept their displayed values synchronised with the left controls.

### Changed

- Replaced the old 0-95% Feedback range with a conventional 0-200% percentage control.
- Made 0-100% produce progressively longer repeating decays.
- Made 100-200% drive the delay into intentional regeneration and self-oscillation.
- Added high-headroom soft limiting inside the Digital and Tape feedback loops so regenerative settings remain finite and controlled.
- Gave Tape feedback a lower saturation headroom so accumulating repeats compress and colour progressively.
- Advanced the state schema to version 2 and updated all project metadata to 0.5.0.

### Compatibility

- This early-development release intentionally ignores state written by 0.4.0 and older builds. Insert a fresh 0.5.0 instance.

## 0.4.0 — Clean Pre-Release Foundation — 17 October 2026

### Removed

- Removed the complete 0.2.x `character` migration path.
- Removed the temporary `modeId` 0–127 placeholder parameter.
- Removed the legacy `tapeDrive`, `tapeAge`, `tapeWow` and `tapeFlutter` parameter IDs.
- Removed all pre-1.0 saved-state compatibility behaviour. Older prototype state is deliberately ignored.

### Changed

- Replaced the temporary identifiers with `mode`, `drive`, `age`, `wow` and `flutter`.
- Replaced Mode with a true two-choice Digital/Tape host parameter.
- Rebuilt the processor around cached atomic parameter pointers and fixed-size stack data in the audio callback.
- Increased maximum delay memory from four to eight seconds for slower synchronised divisions.
- Added an explicit state schema for rejecting incompatible prototype state cleanly.
- Updated the build, interface, README and VST3 metadata to version 0.4.0.

### Improved

- Added matched mono/stereo bus-layout validation.
- Added 20 ms smoothing for Feedback, Cross, Dry/Wet, Output, Drive, Wow, Flutter and all filter coefficients.
- Added finite-value validation and bounded emergency protection for input, delay memory and output.
- Reworked Drive with stronger level-compensated soft clipping so the effect remains clearly audible at ordinary input levels.
- Added a calculated host tail length based on delay time and Feedback.
- Skipped modulation oscillator calculations at zero depth.
- Skipped inactive Digital or Tape read-head processing outside mode transitions.
- Kept Drive strictly on new delay input so feedback repeats are never re-driven.

### Compatibility

- This release intentionally breaks all earlier prototype state. Delete old Afterglow instances and insert a fresh 0.4.0 instance.
- Saved-state compatibility becomes mandatory only from version 1.0.0 onward.

## 0.3.2 — Audible Drive Repair — 16 October 2026

### Fixed

- Removed the excessive pre-gain division introduced in 0.3.1, which made Drive nearly inaudible at normal delayed-signal levels.
- Reworked Drive as a real delay-input overdrive: up to roughly 24 dB of input gain feeds a soft clipper, followed by pre-gain removal and modest makeup gain.

### Retained

- Drive is applied once as new audio enters the delay; recirculating repeats are never passed through Drive again, so it cannot increase internal regeneration or destabilise the feedback loop.
- Preserved the existing `tapeDrive` parameter ID and saved values for project compatibility.

### Changed

- Updated the build, interface, README and VST3 metadata to version 0.3.2.

## 0.3.1 — Drive Routing Fix — 16 October 2026

### Fixed

- Removed Drive from the feedback write path so it can no longer increase regeneration or extend the repeat decay.
- Reworked the Drive waveshaper to retain unity gain for quiet signals instead of amplifying them inside the delay loop.

### Changed

- Drive now acts as output overdrive distortion on the audible wet signal in every mode.
- Updated the build, interface, README and VST3 metadata to version 0.3.1.

## 0.3.0 — Mode Foundation and Tape Engine — 16 October 2026

### Added

- Distinct Digital and Tape delay-time engines.
- Clean dual-read-head crossfades for Digital delay-time changes.
- Tape-style varispeed pitch glides for Tape delay-time changes.
- Short engine crossfade when switching modes.
- Stable integer `modeId` parameter with reserved IDs for later modes.
- Automatic migration of the 0.2.x Digital/Tape `character` state.

### Changed

- Made Drive, Wow and Flutter available in every mode.
- Restricted Age to the Tape engine, where its bandwidth loss accumulates through the feedback path.
- Reset the fresh-instance defaults for Drive, Wow and Flutter to zero so Digital starts clean.
- Renamed the interface selector from Character to Mode.
- Updated the build, interface, README and VST3 metadata to version 0.3.0.

### Compatibility

- Preserved every 0.1.0 parameter ID.
- Preserved the 0.2.x `tapeDrive`, `tapeAge`, `tapeWow` and `tapeFlutter` IDs and saved values.
- Retained the 0.2.1 editor-startup bounds guard.

## 0.2.1 — Editor Startup Fix — 16 October 2026

### Fixed

- Prevented the editor's initial `resized()` callback from indexing dial controls before they have been created.
- Restored reliable PHI loading after the layout regression introduced in 0.2.0.

### Changed

- Updated the build, interface, README and VST3 metadata to version 0.2.1.

## 0.2.0 — Tape Character — 16 October 2026

### Added

- Digital and Tape Character selector.
- Tape Drive saturation inside the feedback path.
- Tape Age bandwidth reduction that accumulates across repeats.
- Tape Wow and Flutter delay-time movement.
- Stable host parameters for Character, Drive, Age, Wow and Flutter.

### Changed

- Reorganised the controls into compact rows aligned from the top-left of the main panel.
- Moved Tempo Sync and both division selectors fully inside the main panel.
- Reduced the editor height and limited the contextual information area to two text lines.
- Darkened the main panel with a muted plum tint.
- Adopted SV-Drummer-style compact knobs, value displays and label fonts.
- Updated the build, interface and VST3 metadata to version 0.2.0.

### Fixed

- Centred every knob label on its corresponding rotary control.
- Removed the obsolete large border around the contextual information area.
- Removed two harmless double-to-float build warnings in initial delay setup.

### Compatibility

- Preserved all existing 0.1.0 parameter IDs and project-state values.
- Existing projects open with the new Character parameter set to Digital.

## 0.1.0 — Initial Digital Delay — 16 October 2026

### Added

- Initial JUCE 9.0.4 Windows x64 VST3 project.
- Stereo digital delay with independent Left and Right times.
- Tempo synchronisation and musical divisions.
- Feedback, Cross Feedback, Dry/Wet and Output controls.
- Low Cut and High Cut processing for every returned echo.
- Host parameter automation and project-state restoration.
- Fixed two-line contextual help text.
- Portable `dist\Afterglow.vst3` build workflow.

### Fixed during 0.1.0 development

- Corrected the CMake executable path in `- Build.bat`.
- Restored automatic closure of PolyHostInterface before building.
- Corrected the initial invisible-knob layout.
- Moved filtering onto the delayed signal so the first echo is processed.
- Confirmed Low Cut, High Cut, Cross Feedback and PHI state restoration by listening tests.

## Versioning policy

- Adjustments and fixes advance the patch number, for example `0.3.1`.
- New feature stages advance the middle number, for example `0.4.0`.
- The build metadata, interface text, README and changelog advance together.
