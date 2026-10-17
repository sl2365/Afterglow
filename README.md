# Afterglow 0.10.0

Afterglow is an early-development stereo VST3 delay by sl23 for the Dead Channel Audio plugin suite. Version 0.10.0 adds a dedicated wet-return ducker and reorganises the centre controls into compact stacked Feedback and Ducking panels.

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

Uses a continuously moving read head, delay-time-dependent drift, two-stage high-frequency head loss and soft tape saturation. Age progressively increases bandwidth wear and compression.

### HiFi

Uses continuous varispeed delay movement like a studio reel-to-reel delay, while remaining much brighter and cleaner than Tape. Gentle inherent saturation and drift distinguish it from Digital; Age adds restrained bandwidth wear and asperity texture.

### BBD

Models a deliberately dark bucket-brigade delay with two-stage, delay-dependent bandwidth loss, continuous clock movement, companding, progressively coarser resolution, signal-tracking noise and stronger controlled saturation.

### Ghost

Uses a continuously moving analogue-style delay feeding a quadrature frequency shifter. The left and right repeats shift in opposite directions, and the spectral displacement accumulates naturally through the feedback loop. Age progressively darkens and saturates the shifted repeats.

## Controls

- Independent Left and Right delay times from 1 to 2000 ms.
- Link L/R makes the disabled Right Time knob visually follow the left knob without overwriting its independent millisecond or note-division value. Turning Link off restores the previous right-channel setting for the active timing mode.
- With Tempo Sync off, the two Time knobs display and edit milliseconds.
- With Tempo Sync on, those same two knobs display and edit musical note divisions. Separate straight, dotted and triplet choices are available through 1/64 and are ordered from shortest to longest so clockwise movement remains consistent with millisecond mode.
- Switching Tempo Sync never converts or overwrites values: the stored millisecond and note settings are independent and return when their timing mode is selected again.
- The remaining Mode combo box suppresses duplicate wheel events so one physical wheel movement advances exactly one item from the first movement.
- Link L/R and Tempo Sync use clear LED status indicators rather than checkbox graphics.
- Feedback from 0% to 200% and continuously variable Cross Feedback.
- Bipolar Spread from -100% to +100% controls delayed-signal width without changing the Cross-feedback rhythm: 0% centres the repeats, +100% gives full width and negative values reverse their left/right placement. It has a centre detent and double-click reset to 0%.
- Duck from 0% to 100% reduces only the audible wet return while new input is present. The dry signal and internal feedback regeneration remain untouched, so repeats recover naturally instead of being erased from the delay loop.
- Duck Attack ranges from 1 to 500 ms and Duck Release from 20 to 3000 ms. A stereo-linked peak envelope prevents the delayed image from shifting sideways while it ducks.
- 0-100% Feedback produces progressively more repeats; 100-200% enters controlled regeneration and self-oscillation.
- Static soft limiting and Feedback-position gain compensation keep regenerative output controlled without envelope detection or compressor pumping.
- Dry/Wet balance and Output gain.
- Dry/Wet has a 50% centre detent; Output has a 0 dB centre detent. Their indicator arcs grow outwards from their centre positions.
- Double-clicking Dry/Wet returns it directly to its centred 50% value.
- Low Cut and High Cut applied to every echo, including the first.
- Drive applies an intentionally audible, level-compensated soft clip once as new audio enters the delay. Recirculating repeats are never re-driven.
- Wow and Flutter add user-controlled movement in every mode. Tape, BBD and HiFi also retain a small mode-specific movement floor so selecting the mode alone changes its character.
- Age deepens head loss and saturation in Tape, bandwidth/compander texture in BBD, and restrained wear and asperity in HiFi. It is disabled only in Digital mode.
- Fixed two-line contextual information area.
- Top-right Power indicator is illuminated while processing is active and unlit while bypassed. Host bypass automation remains conventional, uses a smoothed dry/processed transition and leaves the delay engine running.
- FDBK, CROSS and SPREAD share a shallow upper panel aligned with Hi Cut.
- DUCK, ATTACK and RELEASE share a matching lower panel aligned with Lo Cut.
- Dry/Wet–Output is the rightmost panel because it is last in the audible signal chain.
- The coloured frame has equal left and right insets around the control panels.
- The complete upper control row, including both Time knobs, sits four pixels higher while the lower row retains its previous position. Link L/R and Tempo Sync are centred beneath their corresponding Time knobs.
- The former purple panel background follows the selected mode: Digital blue, Tape green, BBD red, HiFi orange and Ghost purple.
- The Mode selector now occupies the former `STEREO DELAY / sl23` header position.
- Compact rotary styling shared with SV-Drummer.

## 0.10.0 wet-return ducking and stacked panels

- Added smoothed, host-automatable Duck, Attack and Release controls.
- Added a stereo-linked input envelope that turns down the audible wet return without changing the dry path or the audio recirculating inside the feedback loop.
- Restored FDBK, CROSS and SPREAD to a single horizontal row in a reduced-height upper panel.
- Added a matching lower Ducking panel containing DUCK, ATTACK and RELEASE.
- Aligned the upper panel with Hi Cut and the lower panel with Lo Cut.
- Moved all upper-row knobs up four pixels, left the lower row in place and centred the two Timing switches beneath their respective knobs.
- Widened the interface only as much as needed to preserve equal six-pixel coloured margins.

## 0.9.0 knob timing and Ghost mode

- Removed both Note combo boxes and made the existing Left and Right Time knobs switch directly between millisecond and musical-note parameters.
- Preserved independent millisecond and note-division values for each channel, including while Link L/R is active.
- Aligned both Time knobs with the upper control row and retained the Tempo Sync LED switch.
- Reflowed FDBK and CROSS onto the upper row with SPREAD centred beneath them.
- Reduced the grouped panel widths and the overall interface width while restoring equal coloured margins at both sides.
- Added Ghost as a real fifth algorithm with continuous delay movement, stereo-opposed frequency shifting, analogue filtering and feedback saturation.
- Added a purple Ghost interface colour and updated the contextual mode description.

## 0.8.0 stereo spread and timing clarity

- Added a smoothed, host-automatable bipolar Spread parameter.
- Applied Spread to the audible wet mid/side image after the feedback path, so width changes do not alter Cross timing or regeneration.
- Grouped FDBK, CROSS and SPREAD in a new three-control panel.
- Moved Dry/Wet–Output to the far right and retained Hi Cut above Lo Cut.
- Removed the `STEREO DELAY / sl23` subtitle and moved Mode into its header position.
- Added mode-coloured panel backgrounds based on the blue, green, red and orange Valhalla mode families.
- Made the active timing source explicit: millisecond knobs for Free time, Note selectors for Tempo Sync.
- Added same-direction wheel-event suppression to prevent the remaining two-item jumps reported with some mice.

## 0.7.1 audible mode identity and wheel fix

- Corrected the weak mode separation that allowed Tape and HiFi to sound almost identical to Digital when the character controls were low.
- Kept Digital clean and stable as the reference path.
- Gave Tape inherent two-stage head loss, soft saturation and delay-dependent wow/flutter movement before Age adds further wear.
- Made BBD substantially darker, with stronger delay-dependent bandwidth loss, coarser clock resolution, compander texture and controlled saturation.
- Gave HiFi a brighter reel-to-reel response with gentle saturation, restrained filtering and much smaller drift than Tape.
- Replaced JUCE's accumulated combo-box wheel stepping with a deterministic one-item handler, preventing ignored first movements and occasional two-item jumps.
- Kept the existing 20 ms mode crossfade so switching modes remains click resistant.
- Swapped the filter layout so Hi Cut is above Lo Cut.

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

Backward compatibility is intentionally not supported during early development. Version 0.10.0 uses state schema 11. Remove old Afterglow instances from test projects and insert a fresh 0.10.0 instance.

Once version 1.0.0 is released, parameter IDs and saved-state compatibility become release requirements.

## Suggested 0.10.0 test

1. Run `- Build.bat` and confirm the heading reports 0.10.0 and all four checks pass.
2. Remove any older Afterglow instance from PHI and load a fresh one.
3. With Tempo Sync off, confirm both Time knobs display milliseconds and respond normally.
4. Set distinct Left and Right millisecond values and remember them.
5. Turn Tempo Sync on and confirm the same knobs immediately display musical note values rather than milliseconds.
6. Turn both synced knobs through straight, dotted and triplet divisions, including `1/64`, `1/64 D` and `1/64 T`, and confirm every position changes the delay time.
7. Turn Tempo Sync off and confirm the independent millisecond values from step 4 return unchanged.
8. Turn Tempo Sync on again and confirm the note values from step 6 also return unchanged.
9. In free timing, enable Link L/R and turn the left knob. The right knob should follow visually and become unavailable for separate editing.
10. Disable Link L/R and confirm the original independent right millisecond value returns. Repeat this test with different right and left note values in Tempo Sync mode.
11. Confirm `L — TIME — R` changes to `L — NOTE — R` with Tempo Sync and both timing knobs align vertically with Hi Cut.
12. Set Dry/Wet to 100%, Feedback to 0%, and trigger a short drum hit. One delayed echo should be heard.
13. Raise Feedback through 25%, 50%, 75% and 95%. Each increase should retain more echoes and lengthen the decay.
14. Carefully test 100-200% at a low monitoring level. The delay should enter controlled self-oscillation rather than producing invalid or unbounded samples.
15. Confirm Cross, Spread, Dry/Wet, Drive, Age, Wow and Flutter display percentages rather than decimal fractions.
16. Confirm Spread snaps gently to 0%, Dry/Wet to 50% and Output to 0 dB; each bipolar indicator arc must begin at its centre position.
17. Move Spread and Dry/Wet away from centre, double-click each and confirm they return to 0% and 50% respectively.
18. With different Left and Right delay times, sweep Spread from +100% through 0% to -100%. The repeats should move from full width to centred to reversed placement without changing their timing or decay.
19. Confirm FDBK, CROSS and SPREAD form one horizontal upper panel aligned with Hi Cut.
20. Confirm DUCK, ATTACK and RELEASE form one horizontal lower panel aligned with Lo Cut.
21. Confirm the upper knobs are four pixels higher than before, the lower row is unchanged, and Link L/R and Tempo Sync are centred beneath the Left and Right Time knobs respectively.
22. Confirm the control panels have equal coloured spacing at the left and right edges and Dry/Wet–Output remains the rightmost panel.
23. With Duck at 0%, confirm the delay level and character are unchanged as Attack and Release move.
24. Set Duck to 100%, play repeated dry transients and confirm the audible echoes move out of the way while each transient is present.
25. Repeat step 24 at a 50% Dry/Wet balance and confirm ducking changes the wet return only; the direct signal must not dip.
26. Raise Feedback, sustain the ducking input, then stop. Confirm the internally regenerating tail returns according to Release instead of having been removed from the feedback loop.
27. Compare short and long Attack values and confirm they change how quickly the wet return is attenuated without clicks.
28. Compare short and long Release values and confirm they change how quickly the wet return recovers without pumping, invalid samples or runaway output.
29. Scroll one notch at a time over the Mode selector. The first notch must respond and every physical 
