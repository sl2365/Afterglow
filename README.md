# Afterglow 0.7.0

Afterglow is an early-development stereo VST3 delay by sl23 for the Dead Channel Audio plugin suite. Version 0.7.0 adds a full-bandwidth HiFi studio-tape mode, extends Tempo Sync to 1/64, preserves independent right-channel timing while linked and reorganises related controls into compact panels.

## Build and installation

Project root: `_Projects\Afterglow`, beside the shared `_Projects\_Tools` folder.

1. Save the current PolyHostInterface project because the build closes PHI automatically.
2. Double-click `- Build.bat`.
3. Wait until `BUILD SUMMARY` shows all four checks as `PASS`.
4. Use the portable Windows x64 plugin at `dist\Afterglow.vst3`.
5. If the build fails, send the newly generated `Results.log`.

The build uses JUCE 9.0.4, CMake 4.4.2 and Visual Studio Community 2026 x64 from the existing shared tool folders. It does not install the plugin automatically.

## Current delay modes

### Digital

Uses dual read-head crossfades when the delay time changes. This avoids intentional varispeed pitch sweeps and keeps ordinary timing changes clean.

### Tape

Uses a continuously moving read head when the delay time changes. Time changes therefore glide in pitch, while Age progressively narrows the repeat bandwidth.

### HiFi

Uses continuous varispeed delay movement like a studio reel-to-reel delay, but retains a bright full-bandwidth signal path. Age adds restrained asperity texture without Tape mode's narrowing filters or BBD companding.

### BBD

Models a dark bucket-brigade delay with continuous pitch-changing time movement, delay- and Age-dependent bandwidth, companding, progressively coarser clock resolution, signal-tracking noise and its own controlled feedback saturation.

## Controls

- Independent Left and Right delay times from 1 to 2000 ms.
- Link L/R makes the disabled Right Time and Right Note controls visually follow the left controls without overwriting their independent parameter values. Turning Link off restores the previous right-channel settings.
- Tempo Sync with separate Left and Right musical divisions, including straight, dotted and triplet values through 1/64.
- Every combo box supports mouse-wheel selection.
- Link L/R and Tempo Sync use clear LED status indicators rather than checkbox graphics.
- Feedback from 0% to 200% and continuously variable Cross Feedback.
- 0-100% Feedback produces progressively more repeats; 100-200% enters controlled regeneration and self-oscillation.
- Static soft limiting and Feedback-position gain compensation keep regenerative output controlled without envelope detection or compressor pumping.
- Dry/Wet balance and Output gain.
- Dry/Wet has a 50% centre detent; Output has a 0 dB centre detent. Their indicator arcs grow outwards from their centre positions.
- Double-clicking Dry/Wet returns it directly to its centred 50% value.
- Low Cut and High Cut applied to every echo, including the first.
- Drive applies an intentionally audible, level-compensated soft clip once as new audio enters the delay. Recirculating repeats are never re-driven.
- Wow and Flutter available in Digital and Tape modes.
- Age controls bandwidth wear in Tape, bandwidth/compander texture in BBD, and restrained asperity texture in HiFi. It is disabled only in Digital mode.
- Fixed two-line contextual information area.
- Top-right Power indicator is illuminated while processing is active and unlit while bypassed. Host bypass automation remains conventional, uses a smoothed dry/processed transition and leaves the delay engine running.
- Related controls are grouped into Timing, Dry/Wet–Output, Lo Cut–Hi Cut and Drive–Age–Wow–Flutter panels.
- Compact rotary styling shared with SV-Drummer.

## 0.5.0 timing and feedback changes

- Added a host-automatable Link L/R switch.
- Linked time is enforced inside the processor, not only represented by the interface.
- Right Time and Right Note visibly follow their left counterparts while linked, and the right controls are disabled.
- Reworked Feedback as a clear 0-200% control.
- Each delayed return is fed back into the delay input to create the next echo.
- Low Cut, High Cut and Tape Age remain inside the repeat path, so their effect accumulates naturally across echoes.
- Feedback above 100% enters self-oscillation instead of being blocked by the previous 95% safety ceiling.
- Added high-headroom soft limiting inside the feedback loop to prevent numerical runaway while preserving regenerative behaviour.

## 0.5.1 interface and level refinements

- Converted Cross, Dry/Wet, Drive, Age, Wow and Flutter from 0-1 displays to 0-100% displays.
- Added a smooth, memoryless ceiling to the audible wet return and static compensation above 100% Feedback.
- Avoided envelope followers and automatic compressors, so the protection cannot pump.
- Added centre detents and centre-origin indicator arcs to Dry/Wet and Output.
- Made the disabled Right Time knob itself visibly grey when Link L/R is enabled.
- Grouped Left Time, Right Time, both Note selectors, Link L/R and Tempo Sync inside a dedicated Timing panel.

## 0.5.2 compact controls and bypass

- Reduced both Timing note-selector widths and narrowed the complete Timing panel.
- Replaced checkbox presentation for Link L/R and Tempo Sync with compact LED switches.
- Added a power-style Bypass control at the top-right of the interface without a text box.
- Made Bypass host-automatable and saved as part of the current prototype state.
- Added a 20 ms bypass crossfade so switching cannot click; internal delay processing and tails continue while the dry signal is passing through.
- Made double-click reset Dry/Wet to its 50% centre.

## 0.5.3 timing-panel polish

- Replaced the Bypass graphic with a centred, conventional power symbol while preserving the subtle active glow.
- Removed the redundant `TIMING`, `LEFT TIME`, `RIGHT TIME`, `LEFT NOTE` and `RIGHT NOTE` labels.
- Added compact line-and-dot `TIME` and `NOTE` group dividers.
- Moved the timing controls upward, reduced the panel height and made its left and right margins symmetrical.
- Enabled mouse-wheel changes on Mode and both Note selectors.
- Added dotted and triplet 1/32 note divisions.

## 0.6.0 BBD mode

- Added BBD as a genuine third delay algorithm rather than a renamed Tape setting.
- Added continuous BBD delay-time movement, so live time changes produce pitch movement instead of Digital mode's read-head crossfade.
- Added delay- and Age-dependent BBD bandwidth, companding, clock quantisation and low-level signal-tracking noise.
- Added independent BBD filter memory and feedback saturation, with a 20 ms click-resistant crossfade between all three modes.
- Enabled Age in both Tape and BBD modes while retaining Digital mode's clean path.
- Moved all Timing controls down four pixels, then moved the Note heading and selectors down four additional pixels.
- Replaced the dot-ended headings with `L — TIME — R` and `L — NOTE — R`; each channel letter is centred over its corresponding control.
- Increased the Timing panel height enough to preserve comfortable lower padding.

## 0.7.0 HiFi mode and grouped interface

- Added straight, dotted and triplet 1/64 Tempo Sync values.
- Reworked Link L/R so the processor uses the left settings and the disabled right controls mirror them visually without changing the stored Right Time or Right Note parameters.
- Reduced the gap between the Note selectors from 24 to 12 pixels, moved the right timing column left by 12 pixels and narrowed the Timing panel by the same amount.
- Added separate vertical panels for Dry/Wet–Output and Lo Cut–Hi Cut, plus a two-by-two panel for Drive, Age, Wow and Flutter.
- Shortened the visible labels to `FDBK`, `LO CUT`, `HI CUT` and `OUT`.
- Inverted the top-right indicator so its glow means Power is on rather than Bypass is on, without reversing the host bypass parameter.
- Added HiFi as a fourth delay mode with bright, continuous studio-tape movement, restrained Age-controlled asperity texture and gentler feedback saturation.
- Extended the 20 ms click-resistant mode transition across Digital, Tape, BBD and HiFi.

## 0.4.0 foundation changes

- Removed all compatibility and migration code from earlier prototypes.
- Replaced temporary parameter IDs with clean current IDs: `mode`, `drive`, `age`, `wow` and `flutter`.
- Replaced the oversized integer Mode placeholder with an actual Digital/Tape choice parameter.
- Added explicit matched mono and stereo bus validation.
- Cached parameter pointers instead of repeatedly looking them up during processing.
- Added click-resistant smoothing for Feedback, Cross, Dry/Wet, Output, Drive, modulation depth and filter coefficients.
- Added finite-value checks around input, feedback memory and output.
- Reworked Drive so its saturation is unmistakable across ordinary input levels without placing it in the feedback path.
- Increased delay memory to eight seconds for slow tempo-synchronised divisions.
- Added a calculated host tail length instead of the old fixed eight-second report.
- Avoided running modulation oscillators and inactive delay-mode processing when they are not needed.
- Kept all memory allocation outside the audio-processing callback.

## Pre-1.0 state policy

Backward compatibility is intentionally not supported during early development. Version 0.7.0 uses a new state schema because the Note and Mode choices changed. Remove old Afterglow instances from test projects and insert a fresh 0.7.0 instance.

Once version 1.0.0 is released, parameter IDs and saved-state compatibility become release requirements.

## Suggested 0.7.0 test

1. Run `- Build.bat` and confirm the heading reports 0.7.0 and all four checks pass.
2. Remove any older Afterglow instance from PHI and load a fresh one.
3. Set distinct Left and Right Time values, enable Link L/R and turn Left Time. Right Time should visually follow it and become unavailable for separate editing.
4. Disable Link L/R and confirm the original independent Right Time value returns.
5. Repeat the same test with distinct Left and Right Note values in Tempo Sync mode; Right Note must return to its previous value when unlinked.
6. Set Dry/Wet to 100%, Feedback to 0%, and trigger a short drum hit. One delayed echo should be heard.
7. Raise Feedback through 25%, 50%, 75% and 95%. Each increase should retain more echoes and lengthen the decay.
8. Carefully test 100-200% at a low monitoring level. The delay should enter controlled self-oscillation rather than producing invalid or unbounded samples.
9. Compare Digital and Tape feedback. Tape should progressively colour and softly compress the accumulating repeats.
10. Confirm Cross, Dry/Wet, Drive, Age, Wow and Flutter display percentages rather than decimal fractions.
11. Confirm Dry/Wet snaps gently to 50%, Output snaps to 0 dB, and both indicator arcs begin at their centre positions.
12. Move Dry/Wet away from centre, double-click it and confirm it returns to 50%.
13. Toggle the top-right Power control while audio and delay tails are active. It should transition cleanly without a click, pass dry audio when off and preserve the running tail for re-entry.
14. Confirm the centred power symbol glows when processing is on and becomes unlit when processing is bypassed.
15. Confirm the narrower Timing panel has symmetrical side margins, a 12-pixel gap between the Note selectors, and correctly centred `L — TIME — R` / `L — NOTE — R` headings.
16. Scroll the mouse wheel over Mode, Left Note and Right Note, and confirm every selector changes.
17. Select `1/64`, `1/64 D` and `1/64 T` in Tempo Sync mode and confirm all three timings work.
18. Switch among Digital, Tape, BBD and HiFi while a tail is active and confirm the transition is smooth and all four modes sound distinct.
19. In BBD mode, increase Age and confirm the repeats become darker and grainier without unstable level jumps.
20. Change BBD delay time while audio is repeating and confirm the pitch moves continuously; repeat in Digital mode and confirm its clean crossfade behaviour remains intact.
21. Compare Tape and HiFi: both should move pitch continuously when delay time changes, while HiFi remains brighter and cleaner.
22. Confirm the top-right indicator glows when processing is active and turns off when bypassed; audio must still crossfade without clicking.
23. Confirm Dry/Wet–Output, Lo Cut–Hi Cut and Drive–Age–Wow–Flutter appear in their own panels while Feedback, Cross and Mode remain in their previous positions.
24. Save and reopen the PHI project created with 0.7.0 and confirm its state restores.

## Forward development plan

Development now follows this order, one stable feature stage at a time:

1. Reproduce the practical controls, settings, routing choices and audible behaviours of ValhallaDelay as closely as possible, using Afterglow's own implementation and interface. This establishes the complete primary delay foundation first.
2. After that foundation is working and tested, expand Afterglow with the additional controls, settings and workflow ideas available in Sixth Sample Deelay PE.
3. Add a flexible LFO and modulation-target system.
4. Evaluate and, if it remains useful, add a Seqwencer-style step sequencer with drag-and-drop parameter targets.

The reference products define the functional target; their proprietary code, branding and graphical assets are not copied.

## Current limitations

- This remains an early prototype, not a 1.0 release.
- Wow and Flutter currently have fixed internal rates.
- The interface is fixed-size.
- Drive is a single soft-clipping type; the planned distortion-type collection is not implemented yet.
- Ghost/frequency shifting, reverse, pitch, granular processing, diffusion, presets, A/B comparison, multitaps, LFO routing and third-party preset import remain future stages.
- Final trademark clearance for the Afterglow name has not been completed.

## Versioning

- Small fixes and adjustments increment the final number: `0.7.1`, `0.7.2`, `0.7.3`.
- New feature stages increment the middle number: `0.8.0`, `0.9.0`, `0.10.0`.
- `README.md` and `Changelog.md` are updated with every release.
