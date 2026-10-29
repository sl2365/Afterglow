#!/usr/bin/env python3
"""One-time external delay preset to Afterglow converter."""

from __future__ import annotations

import argparse
import math
import shutil
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path, PurePosixPath


TEST_PRESETS = (
    "Richard Devine/BBDModder.vpreset",
    "Richard Devine/BoxTiles.vpreset",
    "Richard Devine/BrokenData.vpreset",
    "Richard Devine/Cascader.vpreset",
    "Richard Devine/Decimate.vpreset",
)

NEXT_TEST_PRESETS = (
    "Richard Devine/Double Down.vpreset",
    "Richard Devine/DownSpirals.vpreset",
    "Richard Devine/ElectroFeed.vpreset",
    "Richard Devine/Elektrik.vpreset",
    "Richard Devine/Feedback Harmor.vpreset",
)

CALIBRATION_PRESETS = TEST_PRESETS + NEXT_TEST_PRESETS

# Valhalla exposes 24 reserved algorithm slots; 17 are populated in v3.0.5.
# Composite source modes map to Afterglow's equivalent base algorithm plus
# its independent Duck, Pitch and character controls rather than duplicating
# the same processing as additional mode entries.
MODE_TARGETS = (
    1,   # Tape -> Tape I
    3,   # HiFi -> HiFi
    2,   # BBD -> Brigade
    0,   # Digital -> Digital
    4,   # Ghost -> Ghost
    5,   # Pitch -> Pitch
    6,   # RevPitch -> Pitch Rev
    12,  # BBDuck -> Analogue + Duck
    0,   # Clarity -> Digital + Duck
    11,  # DuckTape -> Chromatic + Duck
    5,   # PitchDuck -> Pitch + Duck
    7,   # LoFi -> LoFi + Duck
    8,   # Quartz -> Crystal
    9,   # PhaserDDL -> Phaser
    10,  # RichPitch -> Pitch Lush
    11,  # ChromeTape -> Chromatic
    12,  # Analog -> Analogue
)
MODE_NAMES = (
    "Tape", "HiFi", "BBD", "Digital", "Ghost", "Pitch", "RevPitch",
    "BBDuck", "Clarity", "DuckTape", "PitchDuck", "LoFi", "Quartz",
    "PhaserDDL", "RichPitch", "ChromeTape", "Analog",
)
STYLE_NAMES = ("Single", "Dual", "Ratio", "PingPong", "Quad")
ERA_NAMES = ("Past", "Present", "Future")
DISTORTION_NAMES = (
    "Soft Clip", "Saturation", "Tube", "Rectify", "Crackle",
    "Lightning", "Amp", "Bumpy", "Asym", "Razor", "Pixelated",
    "Comb", "Stressed", "Samurai", "Fresh", "Hard Clip",
)

# Deelay stores its mode as a zero-based choice.  PE appends Frequency and
# Pitch to the original five-mode order.
DEELAY_MODE_TARGETS = (0, 16, 14, 13, 15, 4, 5)
DEELAY_MODE_NAMES = (
    "Normal", "Reverse Forward", "Pure Reverse", "Chaos",
    "Reverse Chaos", "Frequency", "Pitch",
)
DEELAY_DISTORTION_NAMES = (
    "Saturation", "Asym", "Clip", "Tube", "Rectify", "Crackly",
    "Lightning", "Amp", "Bumpy", "Razor", "Pixelated", "Comb",
    "Samurai", "Stressed", "Fresh",
)
# Deelay's displayed order differs from Afterglow's DIST list.  Each entry is
# the corresponding zero-based Afterglow distortion choice.
DEELAY_DISTORTION_TARGETS = (
    1,   # Saturation -> Saturation
    8,   # Asym -> Asym
    0,   # Clip -> Soft Clip
    2,   # Tube -> Tube
    3,   # Rectify -> Rectify
    4,   # Crackly -> Crackle
    5,   # Lightning -> Lightning
    6,   # Amp -> Amp
    7,   # Bumpy -> Bumpy
    9,   # Razor -> Razor
    10,  # Pixelated -> Pixelated
    11,  # Comb -> Comb
    13,  # Samurai -> Samurai
    12,  # Stressed -> Stressed
    14,  # Fresh -> Fresh
)
DEELAY_STRAIGHT_BEATS = (4.0, 2.0, 1.0, 0.5, 0.25, 0.125, 0.0625)
DEELAY_TRIPLET_BEATS = (
    8.0 / 3.0, 4.0 / 3.0, 2.0 / 3.0, 1.0 / 3.0,
    1.0 / 6.0, 1.0 / 12.0, 1.0 / 24.0,
)
DEELAY_DOTTED_BEATS = (6.0, 3.0, 1.5, 0.75, 0.375, 0.1875, 0.09375)

# ColourCopy stores the displayed TIME BASE choice as a zero-based index.
# The first seventeen choices are host-synchronised beat lengths; the final
# four are absolute delay-line durations.  RATE changes the virtual bucket
# clock from quarter speed at -100 to four-times speed at +100, so audible
# duration is divided by 4 ** (RATE / 100).
COLOURCOPY_SYNC_BEATS = (
    0.125, 1.0 / 6.0, 0.1875, 0.25, 1.0 / 3.0, 0.375,
    0.5, 2.0 / 3.0, 0.75, 1.0, 4.0 / 3.0, 1.5,
    2.0, 8.0 / 3.0, 3.0, 4.0, 8.0,
)
COLOURCOPY_ABSOLUTE_MS = (1000.0, 100.0, 10.0, 1.0)
COLOURCOPY_TIME_BASE_NAMES = (
    "1/32", "1/16 trip", "1/32 dot", "1/16", "1/8 trip",
    "1/16 dot", "1/8", "1/4 trip", "1/8 dot", "1/4",
    "1/2 trip", "1/4 dot", "1/2", "1/1 trip", "1/2 dot",
    "1/1", "2/1", "1s", ".1s", ".01s", ".001s",
)
COLOURCOPY_LFO_TARGET_NAMES = ("Rate", "Rate+", "Tap Position", "Amp")
COLOURCOPY_DUCK_NAMES = ("Off", "Fast", "Medium", "Slow")
COLOURCOPY_FEEDBACK_ROUTING_NAMES = ("Stereo", "Cross", "Mix")

# Era is not one global distortion selector in ValhallaDelay.  It changes
# hardwired details inside each algorithm.  These choices materialise the
# closest available Afterglow DIST family without changing the directly
# mapped source Drive amount.  Future deliberately selects the clean Fresh
# path; older Digital and Brigade models use more period-appropriate colour.
ERA_DISTORTION_BY_MODE = (
    (2, 1, 14),   # Tape: Tube / Saturation / Fresh
    (2, 1, 14),   # HiFi: Tube / Saturation / Fresh
    (7, 1, 14),   # Brigade: Bumpy / Saturation / Fresh
    (10, 0, 14),  # Digital: Pixelated / Soft Clip / Fresh
    (2, 1, 14),   # Ghost: closest colour only; diffusion topology is distinct
    (2, 1, 14),   # Pitch
    (2, 1, 14),   # Pitch Rev
    (7, 1, 14),   # BBDuck -> Analogue
    (10, 0, 14),  # Clarity -> Digital
    (2, 1, 14),   # DuckTape -> Chromatic
    (2, 1, 14),   # PitchDuck -> Pitch
    (12, 1, 14),  # LoFi: Stressed / Saturation / Fresh
    (0, 1, 14),   # Quartz -> Crystal
    (2, 1, 14),   # PhaserDDL -> Phaser
    (2, 1, 14),   # RichPitch -> Pitch Lush
    (2, 1, 14),   # ChromeTape -> Chromatic
    (7, 1, 14),   # Analog -> Analogue
)

DIVISION_NAMES = (
    "1/128", "1/64 T", "1/64", "1/32 T", "1/64 D", "1/32", "1/16 T",
    "1/32 D", "1/16", "1/8 T", "1/16 D", "1/8", "1/4 T",
    "1/8 D", "1/4", "1/4 D", "1/2", "1/1", "2/1", "3/1", "4/1",
)
DIVISION_BEATS = (
    0.03125, 1.0 / 24.0, 0.0625, 1.0 / 12.0, 0.09375, 0.125,
    1.0 / 6.0, 0.1875, 0.25, 1.0 / 3.0, 0.375,
    0.5, 2.0 / 3.0, 0.75, 1.0, 1.5, 2.0, 4.0, 8.0, 12.0, 16.0,
)
STRAIGHT_DIVISIONS = (2, 5, 8, 11, 14, 16, 17, 18, 20)
DOTTED_DIVISIONS = (4, 7, 10, 13, 15, 15, 15, 15, 15)
TRIPLET_DIVISIONS = (1, 3, 6, 9, 12, 12, 12, 12, 12)


def defaults() -> list[tuple[str, float]]:
    values: list[tuple[str, float]] = [
        ("timeL", 375.0), ("timeR", 500.0),
        ("echoOffset", 0.0),
        ("link", 0.0), ("syncL", 0.0), ("syncR", 0.0),
        ("pingPong", 0.0), ("delayStyle", 1.0),
        ("divisionL", 14.0), ("divisionR", 11.0),
        ("multiTap", 0.0), ("multiTapSync", 0.0),
    ]
    tap_defaults = (
        (250.0, 11.0, -75.0, 100.0),
        (375.0, 13.0, -25.0, 85.0),
        (500.0, 14.0, 25.0, 70.0),
        (750.0, 15.0, 75.0, 55.0),
    )
    for number, (time_ms, division, pan, level) in enumerate(tap_defaults, 1):
        values.extend((
            (f"tap{number}Time", time_ms),
            (f"tap{number}Division", division),
            (f"tap{number}Pan", pan),
            (f"tap{number}Level", level),
            (f"tap{number}Enabled", 1.0),
        ))
    values.extend((
        ("feedback", 35.0), ("cross", 0.0), ("spread", 100.0),
        ("duck", 0.0), ("duckAttack", 20.0), ("duckRelease", 400.0),
        ("diffusionAmount", 0.0), ("diffusionSize", 50.0),
        ("modulationAmount", 0.0), ("modulationRate", 0.30),
        ("pitchShift", 7.0), ("pitchDetune", 0.0),
        ("ghostShift", 0.0), ("ghostDetune", 14.0),
        ("phaserResonance", 30.0), ("phaserStages", 12.0),
        ("pitchMix", 50.0), ("mix", 35.0),
        ("lowCut", 10.0), ("highCut", 20000.0),
        ("output", 0.0), ("bypass", 0.0),
        ("freeze", 0.0), ("freezeToggle", 0.0),
        ("mode", 0.0), ("distortionType", 0.0),
        ("drive", 0.0), ("age", 30.0),
        ("wow", 0.0), ("flutter", 0.0),
    ))
    return values


def clamp(value: float, minimum: float, maximum: float) -> float:
    return max(minimum, min(maximum, value))


def snap(value: float, interval: float, minimum: float, maximum: float) -> float:
    return clamp(round(value / interval) * interval, minimum, maximum)


def source_float(attributes: dict[str, str], name: str, fallback: float) -> float:
    try:
        value = float(attributes.get(name, fallback))
    except (TypeError, ValueError):
        return fallback
    return value if math.isfinite(value) else fallback


def parse_colourcopy(data: bytes) -> dict[str, str]:
    """Read the plain-text parameter section of a u-he .h2p preset.

    ColourCopy appends a compressed binary cache after the editable script
    parameters.  Only the documented text section is needed for conversion.
    """
    text = data.split(b"// Section for ugly compressed binary Data", 1)[0]
    decoded = text.decode("utf-8", errors="replace")
    values: dict[str, str] = {}
    for raw_line in decoded.splitlines():
        line = raw_line.strip()
        if not line or line.startswith("//") or "=" not in line:
            continue
        name, value = line.split("=", 1)
        values[name.strip()] = value.strip()

    if values.get("#AM") != "ColourCopy":
        raise ValueError(".h2p preset is not a ColourCopy preset")
    return values


def target_from_normalised(position: float, start: float, end: float,
                           skew: float, interval: float) -> float:
    position = clamp(position, 0.0, 1.0)
    proportion = position ** (1.0 / skew) if position > 0.0 else 0.0
    value = start + (end - start) * proportion
    return clamp(round((value - start) / interval) * interval + start, start, end)


def delay_ms(position: float) -> float:
    # Despite the legacy "_Ms" name, Valhalla stores this field in seconds:
    # 0.198 is the displayed 198 ms and 0.007 is the displayed 7 ms.
    return snap(max(0.0, position) * 1000.0, 0.1, 1.0, 2000.0)


def deelay_delay_ms(position: float) -> float:
    """Convert Deelay PE's normalised millisecond knob to its display value.

    Unlike Valhalla's similarly named field, ``preset_timeMs`` is a skewed
    0-1 control position.  The curve below reproduces the values shown by
    Deelay PE: 0.096 -> 4 ms, 0.183207 -> 19 ms, 0.188 -> 20 ms and
    0.252 -> 40 ms.
    """
    position = clamp(position, 0.0, 1.0)
    return snap(max(1.0, 1000.0 * position ** 2.34),
                1.0, 1.0, 1000.0)


def style_index(position: float) -> int:
    return min(4, max(0, int(clamp(position, 0.0, 1.0) * 5.0)))


def mode_index(position: float) -> int:
    source_index = int(round(clamp(position, 0.0, 1.0) * 24.0)) - 1
    return min(len(MODE_TARGETS) - 1, max(0, source_index))


def era_index(position: float) -> int:
    # The three XML choice positions are 1/3, 2/3 and 1 rather than 0, 1/2, 1.
    return min(2, max(0, int(round(clamp(position, 0.0, 1.0) * 3.0)) - 1))


def sync_slot(position: float) -> int:
    # Valhalla's legacy choices are Msec, Note, Dotted and Triplet.
    return min(4, max(1, int(round(clamp(position, 0.0, 1.0) * 4.0))))


def division_index(note_position: float, slot: int) -> int:
    source_note = min(8, max(0, int(round(clamp(note_position, 0.0, 1.0) * 8.0))))
    if slot == 3:
        return DOTTED_DIVISIONS[source_note]
    if slot == 4:
        return TRIPLET_DIVISIONS[source_note]
    return STRAIGHT_DIVISIONS[source_note]


def nearest_division(beats: float) -> int:
    safe = max(beats, DIVISION_BEATS[0])
    return min(range(len(DIVISION_BEATS)),
               key=lambda index: abs(math.log(DIVISION_BEATS[index] / safe)))


def ghost_shift_hz(position: float) -> float:
    """Map the signed control with fine resolution around zero."""
    bipolar = clamp(position, 0.0, 1.0) * 2.0 - 1.0
    return snap(math.copysign(abs(bipolar) ** 3.0 * 1000.0, bipolar),
                0.1, -1000.0, 1000.0)


def quad_tap_fractions(spacing_position: float) -> tuple[float, ...]:
    """Materialise Valhalla's four heads into Afterglow tap-time ratios.

    At 0% spacing the ratios are 1/4, 1/2, 3/4 and 1.  Negative
    spacing gathers A-C toward D; positive spacing gathers A-C toward A.
    """
    spacing = clamp(spacing_position, 0.0, 1.0) * 2.0 - 1.0
    fractions = [0.25, 0.50, 0.75, 1.0]
    if spacing < 0.0:
        amount = -spacing
        fractions[:3] = [fraction + (1.0 - fraction) * amount
                         for fraction in fractions[:3]]
    elif spacing > 0.0:
        fractions[:3] = [fraction + (0.25 - fraction) * spacing
                         for fraction in fractions[:3]]
    return tuple(fractions)


def format_value(value: float) -> str:
    if abs(value - round(value)) < 1.0e-9:
        return str(int(round(value)))
    return f"{value:.9g}"


def build_target(values: dict[str, float]) -> ET.Element:
    root = ET.Element("AfterglowParameters", {"schemaVersion": "34"})
    for parameter_id, _ in defaults():
        ET.SubElement(root, "PARAM", {
            "id": parameter_id,
            "value": format_value(values[parameter_id]),
        })
    return root


def deelay_list_index(value: float, count: int) -> int:
    """Convert Deelay's one-based displayed-list ID to a zero-based index."""
    return min(count - 1, max(0, int(round(value)) - 1))


def deelay_division(attributes: dict[str, str]) -> tuple[bool, int, str]:
    if source_float(attributes, "preset_nrm", 0.0) >= 0.5:
        choices = DEELAY_STRAIGHT_BEATS
        index = int(round(source_float(attributes, "preset_timeNrm", 0.0)))
        name = "Normal"
    elif source_float(attributes, "preset_trip", 0.0) >= 0.5:
        choices = DEELAY_TRIPLET_BEATS
        index = int(round(source_float(attributes, "preset_timeTrip", 0.0)))
        name = "Triplet"
    elif source_float(attributes, "preset_dot", 0.0) >= 0.5:
        choices = DEELAY_DOTTED_BEATS
        index = int(round(source_float(attributes, "preset_timeDot", 0.0)))
        name = "Dotted"
    else:
        return False, 14, "Milliseconds"

    index = min(len(choices) - 1, max(0, index))
    return True, nearest_division(choices[index]), name


def colourcopy_lfo_rate(position: float) -> float:
    """Map ColourCopy's 0-100 RATE position to its 0.05-20 Hz display."""
    return 0.05 * math.pow(400.0, clamp(position, 0.0, 100.0) / 100.0)


def colourcopy_brightness_cut(brightness: float) -> float:
    """Approximate ColourCopy BRIGHT with Afterglow's available HI CUT.

    A perceptual curve keeps the neutral source position open while retaining
    useful travel for darker negative settings.
    """
    position = (clamp(brightness, -100.0, 100.0) + 100.0) / 200.0
    return snap(200.0 + math.pow(position, 0.35) * 19800.0,
                1.0, 200.0, 20000.0)


def convert_colourcopy(attributes: dict[str, str]) -> tuple[ET.Element, list[str]]:
    values = dict(defaults())
    values["mode"] = 18.0  # Colour

    time_base = min(20, max(0, int(round(
        source_float(attributes, "TimeB", 18.0)))))
    delay_rate = clamp(source_float(attributes, "DlyRate", 0.0),
                       -100.0, 100.0)
    clock_speed = math.pow(4.0, delay_rate / 100.0)
    tap_left = clamp(source_float(attributes, "TapL", 100.0), 0.0, 100.0)
    tap_right = clamp(source_float(attributes, "TapR", 100.0), 0.0, 100.0)
    left_fraction = tap_left * 0.01
    right_fraction = tap_right * 0.01

    if time_base < len(COLOURCOPY_SYNC_BEATS):
        base_beats = COLOURCOPY_SYNC_BEATS[time_base] / clock_speed
        left_division = nearest_division(base_beats * max(left_fraction, 0.0001))
        right_division = nearest_division(base_beats * max(right_fraction, 0.0001))
        values["syncL"] = 1.0
        values["syncR"] = 1.0
        values["divisionL"] = float(left_division)
        values["divisionR"] = float(right_division)
        displayed_left = DIVISION_NAMES[left_division]
        displayed_right = DIVISION_NAMES[right_division]
    else:
        base_ms = COLOURCOPY_ABSOLUTE_MS[
            time_base - len(COLOURCOPY_SYNC_BEATS)] / clock_speed
        left_ms = snap(base_ms * max(left_fraction, 0.0001),
                       0.1, 1.0, 2000.0)
        right_ms = snap(base_ms * max(right_fraction, 0.0001),
                        0.1, 1.0, 2000.0)
        values["timeL"] = left_ms
        values["timeR"] = right_ms
        displayed_left = f"{format_value(left_ms)} ms"
        displayed_right = f"{format_value(right_ms)} ms"

    values["link"] = float(abs(tap_left - tap_right) < 0.0001)
    values["feedback"] = snap(
        source_float(attributes, "Regen", 35.0), 0.1, 0.0, 200.0)

    feedback_routing = min(2, max(0, int(round(
        source_float(attributes, "RoutFB", 0.0)))))
    values["cross"] = (0.0, 100.0, 30.0)[feedback_routing]
    values["spread"] = snap(
        source_float(attributes, "Width", 100.0), 0.1, 0.0, 100.0)

    stereo_phase = clamp(source_float(attributes, "Stereo", 90.0),
                         0.0, 180.0)
    if stereo_phase < 45.0:
        values["delayStyle"] = 0.0
        mapped_phase = 0
    elif stereo_phase < 135.0:
        values["delayStyle"] = 1.0
        mapped_phase = 90
    else:
        # Ping-Pong style supplies a 180-degree modulation phase without
        # enabling Afterglow's separate Ping-Pong audio-routing switch.
        values["delayStyle"] = 3.0
        mapped_phase = 180

    lfo_target = min(3, max(0, int(round(
        source_float(attributes, "LFOTrg", 0.0)))))
    lfo_depth = clamp(source_float(attributes, "LFODpt", 0.0), 0.0, 100.0)
    source_lfo_rate = colourcopy_lfo_rate(
        source_float(attributes, "Rate", 50.0))
    if lfo_target < 3:
        values["modulationAmount"] = snap(lfo_depth, 0.1, 0.0, 100.0)
        values["modulationRate"] = snap(
            source_lfo_rate, 0.01, 0.02, 10.0)

    # Colour is a continuous visible 1-5 macro (stored as floating point),
    # while Saturation controls its prominence.  AGE is the closest continuous
    # character-position control; DIST Saturation retains the source intensity.
    colour_position = clamp(
        source_float(attributes, "Colour", 3.0), 1.0, 5.0)
    saturation = clamp(source_float(attributes, "Sat", 0.0), 0.0, 100.0)
    values["age"] = snap(
        (colour_position - 1.0) * 25.0, 0.1, 0.0, 100.0)
    values["distortionType"] = 1.0  # Saturation
    values["drive"] = snap(saturation, 0.1, 0.0, 100.0)
    brightness = source_float(attributes, "Bright", 0.0)
    values["highCut"] = colourcopy_brightness_cut(brightness)

    duck_mode = min(3, max(0, int(round(
        source_float(attributes, "Duck", 0.0)))))
    duck_target = min(1, max(0, int(round(
        source_float(attributes, "DuckTrg", 0.0)))))
    duck_threshold = clamp(
        source_float(attributes, "DuckThr", -30.0), -48.0, 24.0)
    if duck_mode > 0 and duck_target == 1:
        values["duck"] = snap(
            (24.0 - duck_threshold) / 72.0 * 100.0,
            0.1, 0.0, 100.0)
        duck_times = ((20.0, 400.0), (1.0, 120.0),
                      (8.0, 350.0), (35.0, 900.0))
        values["duckAttack"], values["duckRelease"] = duck_times[duck_mode]

    values["mix"] = snap(
        source_float(attributes, "Mix", 35.0), 0.1, 0.0, 100.0)
    values["output"] = snap(
        source_float(attributes, "OutG", 0.0), 0.1, -24.0, 12.0)

    notes = [
        "Source: u-he ColourCopy (.h2p)",
        f"Mode: Colour",
        f"Time Base: {COLOURCOPY_TIME_BASE_NAMES[time_base]}, "
            f"Rate {format_value(delay_rate)}%",
        f"Time L: {displayed_left} (tap {format_value(tap_left)}%)",
        f"Time R: {displayed_right} (tap {format_value(tap_right)}%)",
        f"Feedback: {format_value(values['feedback'])}%",
        f"Feedback routing: {COLOURCOPY_FEEDBACK_ROUTING_NAMES[feedback_routing]} "
            f"-> Cross {format_value(values['cross'])}%",
        f"Width: {format_value(values['spread'])}% -> Spread",
        f"Colour: {format_value(colour_position)} -> AGE "
            f"{format_value(values['age'])}%",
        f"Saturation: {format_value(saturation)}% -> DIST Saturation "
            f"{format_value(values['drive'])}%",
        f"Brightness: {format_value(brightness)} -> HI CUT "
            f"{format_value(values['highCut'])} Hz (closest available control)",
        f"Mix: {format_value(values['mix'])}%",
        f"Output: {format_value(values['output'])} dB",
    ]

    if lfo_depth > 0.0001:
        if lfo_target < 3:
            notes.append(
                f"LFO: {COLOURCOPY_LFO_TARGET_NAMES[lfo_target]}, "
                f"{format_value(source_lfo_rate)} Hz, "
                f"Depth {format_value(lfo_depth)}% -> MOD "
                f"{format_value(values['modulationRate'])} Hz / "
                f"{format_value(values['modulationAmount'])}%.")
            if source_lfo_rate > 10.0:
                notes.append("LFO Rate was clamped to Afterglow's 10 Hz maximum.")
        else:
            notes.append(
                "LFO AMP target omitted: Afterglow MOD controls delay time, "
                "not delay amplitude.")
        if int(round(source_float(attributes, "Wave", 0.0))) != 0:
            notes.append(
                "Triangle LFO waveform approximated by Afterglow's fixed sine LFO.")
        if abs(stereo_phase - mapped_phase) > 0.0001:
            notes.append(
                f"LFO stereo phase {format_value(stereo_phase)} degrees -> "
                f"nearest available {mapped_phase} degrees.")

    if abs(tap_left - 100.0) > 0.0001 or abs(tap_right - 100.0) > 0.0001:
        notes.append(
            "Tap positions are materialised as Afterglow L/R Time. ColourCopy's "
            "separate full feedback-loop period has no direct Afterglow control.")
    if duck_mode > 0:
        if duck_target == 1:
            notes.append(
                f"Ducking: {COLOURCOPY_DUCK_NAMES[duck_mode]} AMP at "
                f"{format_value(duck_threshold)} dB -> Duck "
                f"{format_value(values['duck'])}%.")
        else:
            notes.append(
                "Feedback-only ducking omitted: Afterglow DUCK controls the "
                "audible delay output rather than only the feedback return.")
    if abs(source_float(attributes, "LFODyn", 0.0)) > 0.0001:
        notes.append("Dynamic LFO depth/rate omitted: Afterglow has no equivalent control.")
    if int(round(source_float(attributes, "Invert", 0.0))) != 0:
        notes.append("Feedback phase inversion omitted: Afterglow has no equivalent switch.")
    if int(round(source_float(attributes, "RoutIN", 0.0))) != 0:
        notes.append("Input routing omitted: Afterglow has no separate input-routing selector.")
    if int(round(source_float(attributes, "Floor", 0.0))) != 0:
        notes.append("Vintage noise floor omitted: Afterglow has no equivalent control.")
    input_gain = source_float(attributes, "InG", 0.0)
    if abs(input_gain) > 0.0001:
        notes.append(
            f"Input gain {format_value(input_gain)} dB omitted: Afterglow has no "
            "separate delay-circuit input gain.")
    if int(round(source_float(attributes, "NoteCtr", 0.0))) != 0:
        notes.append("MIDI Note Control omitted: Afterglow has no equivalent switch.")

    return build_target(values), notes


def convert_deelay(source: ET.Element) -> tuple[ET.Element, list[str]]:
    attributes = source.attrib
    values = dict(defaults())

    source_mode = deelay_list_index(
        source_float(attributes, "preset_mode", 1.0),
        len(DEELAY_MODE_TARGETS))
    target_mode = DEELAY_MODE_TARGETS[source_mode]
    tape = clamp(source_float(attributes, "preset_tape", 0.0), 0.0, 1.0)

    base_ms = deelay_delay_ms(
        source_float(attributes, "preset_timeMs", 0.375))
    echo_offset = clamp(
        source_float(attributes, "preset_echoOffset", 0.0), 0.0, 1.0)
    sync, division, time_mode = deelay_division(attributes)
    ping_pong = source_float(attributes, "preset_pipo", 0.0) >= 0.5
    mono = source_float(attributes, "preset_mono", 0.0) >= 0.5

    values["mode"] = float(target_mode)
    values["timeL"] = base_ms
    values["timeR"] = base_ms
    values["echoOffset"] = snap(echo_offset * 100.0,
                                  0.1, 0.0, 100.0)
    values["syncL"] = float(sync)
    values["syncR"] = float(sync)
    values["divisionL"] = float(division)
    values["divisionR"] = float(division)
    values["link"] = float(mono)
    values["pingPong"] = float(ping_pong)
    values["delayStyle"] = float(3 if ping_pong else (0 if mono else 1))

    values["feedback"] = snap(
        source_float(attributes, "preset_feedback", 0.35) * 100.0,
        0.1, 0.0, 200.0)
    values["cross"] = 100.0 if ping_pong else 0.0
    # Deelay PE displays Spread as a delay-time offset in milliseconds,
    # whereas Afterglow's SPREAD controls the stereo field.  Preserve the
    # source knob position as the nearest useful spatial equivalent.  Do not
    # discard it when Deelay's Mono switch is set: the source Spread control
    # remains active in that state.
    values["spread"] = snap(
        source_float(attributes, "preset_spread", 0.0) * 100.0,
        0.1, -100.0, 100.0)
    values["duck"] = snap(
        source_float(attributes, "preset_duck", 0.0) * 100.0,
        0.1, 0.0, 100.0)
    values["duckAttack"] = target_from_normalised(
        source_float(attributes, "preset_duckAttack", 0.5),
        1.0, 500.0, 0.45, 1.0)
    # Deelay PE's Release display is logarithmic from 10 to 1000 ms
    # (0.5 therefore displays as 100 ms).  Convert its displayed value first,
    # then clamp it to Afterglow's 20–3000 ms range.
    duck_release_position = clamp(
        source_float(attributes, "preset_duckRelease", 0.5), 0.0, 1.0)
    values["duckRelease"] = snap(
        10.0 * math.pow(100.0, duck_release_position),
        1.0, 20.0, 3000.0)
    values["diffusionAmount"] = snap(
        source_float(attributes, "preset_diffAmt", 0.0) * 100.0,
        0.1, 0.0, 100.0)
    values["diffusionSize"] = snap(
        source_float(attributes, "preset_diffSize", 0.5) * 100.0,
        0.1, 0.0, 100.0)
    values["modulationAmount"] = snap(
        source_float(attributes, "preset_modAmt", 0.0) * 100.0,
        0.1, 0.0, 100.0)
    # Deelay PE's stored Mod Rate is a linear normalised position across its
    # 0.02–10 Hz display range.  Applying Afterglow's own skew to the source
    # position incorrectly turned 3.0 Hz into roughly 0.34 Hz.
    mod_rate_position = clamp(
        source_float(attributes, "preset_modRate", 0.0), 0.0, 1.0)
    values["modulationRate"] = snap(
        0.02 + mod_rate_position * (10.0 - 0.02),
        0.01, 0.02, 10.0)
    # Deelay PE's single Tape control combines slower wow and faster flutter.
    # Afterglow exposes those motions separately and is stronger at matched
    # percentages, so reproduce the source amount on both at two-thirds gain.
    tape_motion = snap(tape * (2.0 / 3.0) * 100.0,
                       0.1, 0.0, 100.0)
    values["wow"] = tape_motion
    values["flutter"] = tape_motion

    source_distortion = deelay_list_index(
        source_float(attributes, "preset_distFunction", 1.0),
        len(DEELAY_DISTORTION_TARGETS))
    distortion_choice = DEELAY_DISTORTION_TARGETS[source_distortion]
    values["distortionType"] = float(distortion_choice)
    values["drive"] = snap(
        source_float(attributes, "preset_distortion", 0.0) * 100.0,
        0.1, 0.0, 100.0)
    values["lowCut"] = snap(
        source_float(attributes, "preset_lowCut", 10.0),
        1.0, 10.0, 2000.0)
    values["highCut"] = snap(
        source_float(attributes, "preset_highCut", 20000.0),
        1.0, 200.0, 20000.0)

    dry = clamp(source_float(attributes, "preset_dry", 1.0), 0.0, 2.0)
    wet = clamp(source_float(attributes, "preset_wet", 0.35), 0.0, 2.0)
    gain_sum = dry + wet
    values["mix"] = snap(
        (wet / gain_sum if gain_sum > 0.000001 else 0.0) * 100.0,
        0.1, 0.0, 100.0)
    values["output"] = snap(
        20.0 * math.log10(max(gain_sum, 10.0 ** (-24.0 / 20.0))),
        0.1, -24.0, 12.0)

    notes = [
        "Source: Sixth Sample Deelay PE (.sspreset)",
        f"Mode: {DEELAY_MODE_NAMES[source_mode]}",
        f"Time mode: {time_mode}",
        f"Time L: {DIVISION_NAMES[division] if sync else format_value(base_ms) + ' ms'}",
        f"Time R: {DIVISION_NAMES[int(values['divisionR'])] if sync else format_value(values['timeR']) + ' ms'}",
        f"Feedback: {format_value(values['feedback'])}%",
        f"Dry/Wet: {format_value(dry)} / {format_value(wet)} -> Mix {format_value(values['mix'])}%, Output {format_value(values['output'])} dB",
        f"DIST: {DEELAY_DISTORTION_NAMES[source_distortion]} -> "
            f"{DISTORTION_NAMES[distortion_choice]} at "
            f"{format_value(values['drive'])}%",
    ]
    if echo_offset > 0.0:
        notes.append(
            f"Echo Offset: {format_value(values['echoOffset'])}% -> "
            "Afterglow Echo Offset.")
    source_spread = source_float(attributes, "preset_spread", 0.0)
    if abs(source_spread) > 0.0001:
        notes.append(
            f"Spread: source delay-time offset position "
            f"{format_value(source_spread * 100.0)}% -> Afterglow spatial "
            f"Spread {format_value(values['spread'])}% (closest available "
            "control; the units are not equivalent).")
    if tape > 0.0001:
        notes.append(
            f"Tape: {format_value(tape * 100.0)}% -> Wow and Flutter "
            f"{format_value(tape_motion)}% each.")
    if source_float(attributes, "preset_postDist", 0.0) >= 0.5:
        notes.append("Post-DIST routing omitted: Afterglow DIST is an input stage.")
    if source_float(attributes, "preset_fdbFil", 0.0) >= 0.5:
        notes.append("Feedback-filter routing omitted: Afterglow filters use their fixed delay path.")
    notes.append("Diffusion Quality has no direct Afterglow parameter and is omitted.")
    return build_target(values), notes


def convert(source: ET.Element) -> tuple[ET.Element, list[str]]:
    attributes = source.attrib
    values = dict(defaults())

    source_mode = mode_index(source_float(attributes, "Mode", 1.0 / 24.0))
    target_mode = MODE_TARGETS[source_mode]
    style = style_index(source_float(attributes, "DelayStyle", 0.0))
    source_era = era_index(source_float(attributes, "Era", 2.0 / 3.0))

    left_slot = sync_slot(source_float(attributes, "DelayLSync", 0.25))
    right_slot = sync_slot(source_float(attributes, "DelayRSync", 0.25))
    left_sync = left_slot != 1
    right_sync = right_slot != 1
    left_division = division_index(source_float(attributes, "DelayLNote", 0.0), left_slot)
    right_division = division_index(source_float(attributes, "DelayRNote", 0.0), right_slot)
    left_ms = delay_ms(source_float(attributes, "DelayL_Ms", 0.5))
    right_ms = delay_ms(source_float(attributes, "DelayR_Ms", 0.5))

    values["mode"] = float(target_mode)
    values["delayStyle"] = float(style)
    values["distortionType"] = float(
        ERA_DISTORTION_BY_MODE[source_mode][source_era])
    values["mix"] = snap(source_float(attributes, "Mix", 0.35) * 100.0,
                         0.1, 0.0, 100.0)
    values["feedback"] = snap(source_float(attributes, "Feedback", 0.175) * 200.0,
                              0.1, 0.0, 200.0)
    if source_mode in (7, 8, 9, 10, 11):
        values["duck"] = snap(
            source_float(attributes, "Ducking", 0.0) * 100.0,
            0.1, 0.0, 100.0)
    else:
        values["duck"] = 0.0
    values["spread"] = snap((source_float(attributes, "Width", 1.0) * 200.0) - 100.0,
                            0.1, -100.0, 100.0)
    values["drive"] = snap(source_float(attributes, "DriveIn", 0.0) * 100.0,
                           0.1, 0.0, 100.0)
    values["age"] = snap(source_float(attributes, "Age", 0.30) * 100.0,
                         0.1, 0.0, 100.0)
    if source_mode == 14:  # RichPitch reuses the character slot as PitchMix.
        values["pitchMix"] = values["age"]
    values["diffusionAmount"] = snap(source_float(attributes, "Diffusion", 0.0) * 100.0,
                                     0.1, 0.0, 100.0)
    values["diffusionSize"] = snap(source_float(attributes, "DiffSize", 0.5) * 100.0,
                                   0.1, 0.0, 100.0)
    # Valhalla retains hidden values when a mode does not expose a control.
    # Digital, Brigade, HiFi and Pitch retain the source delay modulation.
    # Pitch presets rely on this motion independently of semitone shifting;
    # discarding it turns moving low textures into static resonant loops.
    if source_mode in (1, 2, 3, 7, 8, 11, 12, 13, 16):
        values["modulationAmount"] = snap(
            source_float(attributes, "ModDepth", 0.0) * 100.0,
            0.1, 0.0, 100.0)
        values["modulationRate"] = target_from_normalised(
            source_float(attributes, "ModRate", 0.0),
            0.02, 10.0, 0.35, 0.01)
    else:
        values["modulationAmount"] = 0.0
        values["modulationRate"] = 0.30
    values["pitchShift"] = snap((source_float(attributes, "PitchShift", 0.5) - 0.5) * 48.0,
                                 0.01, -24.0, 24.0)
    values["pitchDetune"] = snap((source_float(attributes, "PitchDetune", 0.5) - 0.5) * 2.0,
                                  0.001, -1.0, 1.0)
    if target_mode == 4:
        values["ghostShift"] = ghost_shift_hz(
            source_float(attributes, "FreqShift", 0.5))
        values["ghostDetune"] = target_from_normalised(
            source_float(attributes, "FreqDetune", 0.0),
            0.0, 100.0, 0.35, 0.1)
    values["lowCut"] = target_from_normalised(
        source_float(attributes, "LowCut", 0.0), 10.0, 2000.0, 0.35, 1.0)
    values["highCut"] = target_from_normalised(
        source_float(attributes, "HighCut", 1.0), 200.0, 20000.0, 0.4, 1.0)
    # In these seven Valhalla modes Wow/Flutter are active only for Tape.
    if source_mode in (0, 9, 15):
        values["wow"] = snap(source_float(attributes, "Wow", 0.0) * 100.0,
                             0.1, 0.0, 100.0)
        values["flutter"] = snap(source_float(attributes, "Flutter", 0.0) * 100.0,
                                 0.1, 0.0, 100.0)
    else:
        values["wow"] = 0.0
        values["flutter"] = 0.0

    if style == 0:  # Single: Afterglow's linked stereo timing is the direct equivalent.
        values["link"] = 1.0
        values["syncL"] = float(left_sync)
        values["syncR"] = float(left_sync)
        values["divisionL"] = float(left_division)
        values["divisionR"] = float(left_division)
        values["timeL"] = left_ms
        values["timeR"] = left_ms
    elif style == 2:  # Ratio: materialise the audible right time into Afterglow's R control.
        ratio = clamp(source_float(attributes, "DelayRatio", 1.0), 0.01, 2.0)
        values["syncL"] = float(left_sync)
        values["syncR"] = float(left_sync)
        values["divisionL"] = float(left_division)
        values["divisionR"] = float(nearest_division(DIVISION_BEATS[left_division] * ratio))
        values["timeL"] = left_ms
        values["timeR"] = snap(left_ms * ratio, 0.1, 1.0, 2000.0)
    else:
        values["syncL"] = float(left_sync)
        values["syncR"] = float(right_sync)
        values["divisionL"] = float(left_division)
        values["divisionR"] = float(right_division)
        values["timeL"] = left_ms
        values["timeR"] = right_ms

    values["pingPong"] = float(style == 3)
    values["multiTap"] = float(style == 4)
    if style == 4:
        values["multiTapSync"] = float(left_sync)
        tap_fractions = quad_tap_fractions(
            source_float(attributes, "DelaySpacing", 0.5))
        for tap, fraction in enumerate(tap_fractions, 1):
            values[f"tap{tap}Time"] = snap(
                left_ms * fraction, 0.1, 1.0, 2000.0)
            values[f"tap{tap}Division"] = float(nearest_division(
                DIVISION_BEATS[left_division] * fraction))
            values[f"tap{tap}Level"] = 100.0
            values[f"tap{tap}Enabled"] = float(
                source_float(attributes, f"Tap{'ABCD'[tap - 1]}", 1.0)
                >= 0.5)

    root = build_target(values)

    notes = [
        f"Mode: {MODE_NAMES[source_mode]}",
        f"Era: {ERA_NAMES[source_era]} -> {DISTORTION_NAMES[int(values['distortionType'])]} DIST family",
        f"Style: {STYLE_NAMES[style]}",
        f"Time L: {DIVISION_NAMES[int(values['divisionL'])] if values['syncL'] else format_value(values['timeL']) + ' ms'}",
        f"Time R: {DIVISION_NAMES[int(values['divisionR'])] if values['syncR'] else format_value(values['timeR']) + ' ms'}",
        f"Feedback: {format_value(values['feedback'])}%",
        f"Ducking: {format_value(values['duck'])}%",
        f"Mix: {format_value(values['mix'])}%",
    ]
    if target_mode == 4:
        notes.extend((
            f"Ghost Shift: {format_value(values['ghostShift'])} Hz",
            f"Ghost Detune: {format_value(values['ghostDetune'])} Hz",
        ))
    if style == 4:
        notes.append("Quad heads: " + ", ".join(
            f"{tap}={'on' if values[f'tap{tap}Enabled'] else 'off'} "
            f"at {DIVISION_NAMES[int(values[f'tap{tap}Division'])] if values['multiTapSync'] else format_value(values[f'tap{tap}Time']) + ' ms'}"
            for tap in range(1, 5)))
    return root, notes


def indent(element: ET.Element, level: int = 0) -> None:
    space = "\n" + "  " * level
    if len(element):
        if not element.text or not element.text.strip():
            element.text = space + "  "
        for child in element:
            indent(child, level + 1)
        if not element[-1].tail or not element[-1].tail.strip():
            element[-1].tail = space
    if level and (not element.tail or not element.tail.strip()):
        element.tail = space


def choose_preset_names(available_names: tuple[str, ...], args) -> tuple[str, ...]:
    if args.test_five:
        return TEST_PRESETS
    if args.next_five:
        return NEXT_TEST_PRESETS
    if args.calibration_ten:
        return CALIBRATION_PRESETS
    if args.richard_devine:
        return tuple(
            name for name in available_names
            if name.startswith("Richard Devine/")
            and name.lower().endswith(".vpreset"))
    return tuple(name for name in available_names
                 if name.lower().endswith((".vpreset", ".sspreset", ".h2p")))


def safe_windows_component(name: str) -> str:
    invalid = '<>:"/\\|?*'
    cleaned = "".join("_" if character in invalid else character
                      for character in name).rstrip(" .")
    return cleaned or "_"


def convert_selected(names: tuple[str, ...], read_preset,
                     output_directory: Path, reports: list[str]) -> None:
    for name in names:
        source_data = read_preset(name)
        if name.lower().endswith(".h2p"):
            source_attributes = parse_colourcopy(source_data)
            target, notes = convert_colourcopy(source_attributes)
            preset_name = PurePosixPath(name).stem
        else:
            source = ET.fromstring(source_data)
        if name.lower().endswith(".sspreset"):
            target, notes = convert_deelay(source)
            preset_name = (source.attrib.get(
                "preset_name", PurePosixPath(name).stem).strip()
                or PurePosixPath(name).stem)
        elif name.lower().endswith(".vpreset"):
            target, notes = convert(source)
            preset_name = (source.attrib.get(
                "presetName", PurePosixPath(name).stem).strip()
                or PurePosixPath(name).stem)
        indent(target)

        source_path = PurePosixPath(name)
        if any(part in ("", ".", "..") for part in source_path.parts):
            raise ValueError(f"unsafe preset path: {name}")

        safe_name = safe_windows_component(preset_name)
        safe_folders = tuple(safe_windows_component(part)
                             for part in source_path.parts[:-1])
        target_path = (output_directory / Path(*safe_folders)
                       / f"{safe_name}.agpreset")
        target_path.parent.mkdir(parents=True, exist_ok=True)
        ET.ElementTree(target).write(
            target_path, encoding="utf-8", xml_declaration=True)

        reports.append(
            f"{source_path} -> {target_path.relative_to(output_directory)}")
        reports.extend(f"  {note}" for note in notes)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source_presets", type=Path,
                        help="supported preset, ZIP or unzipped folder")
    parser.add_argument("output_directory", type=Path)
    parser.add_argument("--test-five", action="store_true")
    parser.add_argument("--next-five", action="store_true")
    parser.add_argument("--calibration-ten", action="store_true")
    parser.add_argument("--richard-devine", action="store_true")
    args = parser.parse_args()

    if sum((args.test_five, args.next_five, args.calibration_ten,
            args.richard_devine)) > 1:
        parser.error("choose only one preset-selection option")

    if args.output_directory.exists():
        shutil.rmtree(args.output_directory)
    args.output_directory.mkdir(parents=True)

    source_presets = args.source_presets.resolve()
    if not source_presets.exists():
        parser.error(f"source does not exist: {source_presets}")

    reports: list[str] = []
    if source_presets.is_dir():
        preset_files = tuple(sorted(
            (path for path in source_presets.rglob("*")
             if path.is_file()
             and path.suffix.lower() in (".vpreset", ".sspreset", ".h2p")),
            key=lambda path: path.as_posix().lower()))
        available_names = tuple(
            path.relative_to(source_presets).as_posix()
            for path in preset_files)
        names = choose_preset_names(available_names, args)

        def read_folder_preset(name: str) -> bytes:
            relative = PurePosixPath(name)
            return (source_presets / Path(*relative.parts)).read_bytes()

        if not names:
            parser.error(
                f"no supported preset files found in: {source_presets}")
        convert_selected(names, read_folder_preset,
                         args.output_directory, reports)
    elif (source_presets.is_file()
          and source_presets.suffix.lower() in (".vpreset", ".sspreset", ".h2p")):
        names = (source_presets.name,)

        def read_single_preset(name: str) -> bytes:
            if name != source_presets.name:
                raise ValueError(f"unexpected preset path: {name}")
            return source_presets.read_bytes()

        convert_selected(names, read_single_preset,
                         args.output_directory, reports)
    elif zipfile.is_zipfile(source_presets):
        with zipfile.ZipFile(source_presets) as archive:
            available_names = tuple(archive.namelist())
            names = choose_preset_names(available_names, args)
            if not names:
                parser.error(
                    f"no supported preset files found in: {source_presets}")
            convert_selected(names, archive.read,
                             args.output_directory, reports)
    else:
        parser.error("source must be a supported preset, ZIP or unzipped folder")

    report = "\n".join((
        "External delay presets to Afterglow conversion bank",
        "====================================================",
        "",
        "Supported sources: ValhallaDelay .vpreset, Sixth Sample Deelay PE .sspreset",
        "and u-he ColourCopy .h2p.",
        "",
        "ValhallaDelay direct mappings: Mix, audible timing, timing sync type, Mode,",
        "Feedback, Width/Spread, Drive/DIST amount, mode-aware Era-to-DIST family, Age,",
        "Diffusion Amount/Size, active mode-specific modulation, Pitch Shift/Detune,",
        "Ghost Frequency Shift/Detune, filters, active Tape Wow/Flutter and Delay Style.",
        "",
        "Deelay PE direct mappings: delay mode family, timing mode/value, Feedback,",
        "Ping-Pong/Mono, Duck/Attack/Release, Diffusion, Modulation, DIST type/amount,",
        "filters and independent Dry/Wet gains. Dry and Wet are represented by Afterglow's",
        "Mix plus Output so their relative balance and combined level are retained.",
        "Deelay Echo Offset maps to Afterglow Echo Offset. Deelay's millisecond time Spread",
        "has no exact counterpart, so its bipolar knob position maps to Afterglow's spatial",
        "Spread as the closest available control. Independent Tape,",
        "Post-DIST, feedback-filter routing and Diffusion Quality have no direct Afterglow",
        "controls and are reported per preset when applicable.",
        "",
        "ColourCopy direct/closest mappings: Colour mode, TIME BASE/RATE/tap timing,",
        "Regen, feedback routing/Cross, Width/Spread, LFO time modulation, continuous",
        "Colour/AGE position, Saturation/DIST, Brightness/HI CUT, AMP ducking, Mix and",
        "Output. Feedback-only ducking, AMP LFO, input routing/gain, feedback inversion,",
        "dynamic LFO, noise floor and MIDI Note Control have no direct Afterglow controls",
        "and are reported per preset instead of inventing hidden compatibility behaviour.",
        "",
        "Quad converts to Multi-Tap: Tap A-D enable states and four audible head times are",
        "materialised. Delay Spacing changes those times; Valhalla's inter-channel head offset",
        "and Repeat/Swell feedback topology have no direct Afterglow controls and are omitted.",
        "",
        "Era conversion is an available-colour approximation: Valhalla also changes internal",
        "filtering, saturation smoothing and (for Ghost) diffusion topology, for which Afterglow",
        "has no direct parameter. Source Drive remains authoritative, so Era never invents DIST.",
        "",
        "Delay Style is retained as an internal Afterglow parameter. Ping-Pong now uses its",
        "dedicated one-gain-per-round-trip topology. Ratio remains materialised as independent",
        "L/R timing, and Afterglow's unitary feedback matrix supplies its gradual density build.",
        "",
        "Deliberately omitted: Valhalla Single-style time-offset Spread, Repeat/Swell, reserved",
        "values and Quad's paired left/right head topology.",
        "",
        "This is an approximate parameter translation, not an attempt to reproduce Valhalla DSP.",
        "",
        *reports,
        "",
    ))
    (args.output_directory / "Conversion Notes.txt").write_text(report, encoding="utf-8")


if __name__ == "__main__":
    main()
