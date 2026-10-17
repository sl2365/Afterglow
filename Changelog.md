# Afterglow — Changelog

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
