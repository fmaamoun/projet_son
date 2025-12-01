# Teensy Polyphonic Synthesizer

## Overview

A compact audio synthesizer running on the **Teensy 4.0** microcontroller with the **SGTL5000** audio codec, demonstrating a complete DSP audio system on embedded hardware. It features:
- **4 independent voices** with per-voice oscillators and envelopes
- **5 waveform modes**: sine, square, sawtooth, triangle, pulse
- **Real-time control**: volume, reverb, waveform mode selection and stereo Freeverb reverb (adjustable room size)
- **Responsive I/O**: debounced keys, smoothed potentiometers

![3D Render](3D_render.png)



## Repository Structure

```
.
├── code.ino          # Main sketch: audio graph, UI loop, voice allocation
├── MyDsp.h           # Voice class definition (AudioStream subclass)
├── MyDsp.cpp         # Oscillator implementations & per-voice processing
└── README.md         # This file
```

## How It Works

### Signal Flow Overview
The synthesizer processes audio in real-time through a fixed audio graph:
1. **4 voice oscillators** generate waveforms independently
2. **Mixer** combines all voice outputs with adaptive gain
3. **Dry/Wet mixer** blends direct signal with reverb effect
4. **Stereo output** sends final mix to headphones or speaker

### Key Press → Sound Generation
When you press a key:
1. The debouncer detects a valid key press
2. `findFreeVoice()` searches for an unused voice slot (0–3)
3. If found, that voice is configured with the key's frequency and starts playing
4. The voice state is marked as "playing" in `voicePlaying[]` and frequency stored in `voiceFreq[]`
5. `updateMixerGain()` recalculates mixer levels to balance the new voice

### Waveform Generation
Each voice can produce one of five waveforms, switchable in real-time via the mode button:
- **SINE**: Smooth, pure tone (uses hardware-optimized `Sine` oscillator)
- **SQUARE**: Rich, hollow tone (phase-accumulator; high/low based on 50% threshold)
- **SAWTOOTH**: Bright, buzzy tone (linear ramp from –1 to +1 each cycle)
- **TRIANGLE**: Warm tone between sine and square (symmetric ramp)
- **PULSE**: Thin, nasal tone (narrow pulse; high for 25% of cycle)

### Release & Fade-Out
When you release a key:
1. `findVoiceWithFreq()` locates the voice playing that frequency
2. The voice enters "release" mode and begins fading out
3. `currentAmplitude` decrements linearly over 150 ms (configurable)
4. This smooth fade prevents clicking artifacts
5. Once amplitude reaches zero, the voice is marked as free for reallocation

### Real-Time Control
- **Volume pot** (A0): Adjusts master output level (0–100%)
- **Reverb pot** (A8): Adjusts reverb room size (0–100%), creating space and depth
- **Mode button** (pin 17): Cycles through waveforms, affecting all active voices instantly
- All pot readings are smoothed via `ResponsiveAnalogRead` to eliminate noise

### Mixer Gain Balancing
To prevent clipping when multiple voices play:
- `updateMixerGain()` counts active voices
- Distributes gain equally: `gain = 1.0 / activeVoices`
- If all voices are idle, gain defaults to 1.0
- This ensures consistent output level regardless of polyphony count

### Audio Graph
```
voice1 ──┐
voice2 ──├─→ mixer ──→ dryWetMixer ──┐
voice3 ──┤              ↓            ├─→ out (stereo)
voice4 ──┘           reverb ────────┘
```


## Hardware Setup

### Components
| Component | Qty | Notes |
|-----------|-----|-------|
| Teensy 4.0 | 1 | Microcontroller (primary target) |
| Teensy Audio Shield | 1 | SGTL5000 codec, includes stereo I/O |
| Potentiometers | 2 | Volume (A0) & Reverb room size (A8) with pull-up resistors |
| Momentary switches | 8 | Note keys (C D E F G A B) & Waveform selector (with pull-up resistors) |
| Pull-up resistors | 10 | 10kΩ typical for buttons and pot inputs |
| Headphones / Speaker | 1 | Audio output |

### Pin Mapping

| Function | Pin |
|----------|-----|
| Volume pot | `A0` |
| Reverb pot | `A8` |
| Mode button | `17` |
| Note keys | `{0,1,2,3,4,5,9}` |

### Wiring Diagram (Text)
```
Teensy 4.0
┌─────────────────────────┐
│                         │
│  USB (Power & Serial)   │ → PC for programming / serial monitor
│                         │
│ GND ────────┬───────────┤ → Common ground (all switches, pots)
│             │           │
│ A0  ←───[POT1]──────────┤ → Volume (0–1023 ADC)
│ A8  ←───[POT2]──────────┤ → Reverb room size (0–1023 ADC)
│                         │
│ Pin 0  ←─[SW1]──────────┤ → Note C
│ Pin 1  ←─[SW2]──────────┤ → Note D
│ Pin 2  ←─[SW3]──────────┤ → Note E
│ Pin 3  ←─[SW4]──────────┤ → Note F
│ Pin 4  ←─[SW5]──────────┤ → Note G
│ Pin 5  ←─[SW6]──────────┤ → Note A
│ Pin 9  ←─[SW7]──────────┤ → Note B
│                         │
│ Pin 17 ←─[MODE_BTN]─────┤ → Mode button (cycle waveforms)
│                         │
│ Audio Shield (SGTL5000) │
│ ┌──────────────────────┐│
│ │ Teensy Audio Shield  ││
│ │ Pin 23 (DAC out) ────┤┤ → Stereo headphones / speaker
│ │ Pin 22 (DAC out) ────┤┤
│ │ GND ─────────────────┤┤
│ │ 3.3V ────────────────┤┤
│ └──────────────────────┘│
└─────────────────────────┘
```

## Getting Started

### Prerequisites
1. **Arduino IDE**: Download from https://www.arduino.cc/
2. **Teensyduino**: Follow https://www.pjrc.com/teensy/teensyduino.html
3. **Libraries** (install via Arduino IDE → Sketch → Include Library → Manage Libraries):
   - `Bounce2` (debouncing)
   - `ResponsiveAnalogRead` (smoothing analog inputs)

### Build & Upload
1. Connect Teensy to PC via USB cable
2. Open `code.ino` in the Arduino IDE
3. Select **Tools → Board → Teensy 4.0**
4. Select **Tools → Port** (COM port where Teensy appears)
5. Click **Upload** (or Ctrl+U)
6. Open **Tools → Serial Monitor** (9600 baud, optional for debug output)

### First Use
1. Plug in the Teensy via USB → powers the audio system
2. Connect headphones or powered speaker to the audio jack
3. Adjust the **volume pot** to set output level
4. **Press note keys** (pins 0–9, mapped to C D E F G A B) to play sounds
5. **Press mode button** (pin 17) to cycle waveforms: SINE → SQUARE → SAW → TRIANGLE → PULSE
6. **Adjust reverb pot** (A8) to add/remove reverb (room size 0–100%)

> In the serial monitor output, you should see debug messages such as:
>   ```
>  Mode: SINE
>  Volume: 0.50
>  Reverb: 0.40
>  Mode changed to: SQUARE
>  Note ON: C
>  Note OFF: C
>  Mode changed to: SQUARE
>  Note ON: C
>  Note OFF: C
>  ```
