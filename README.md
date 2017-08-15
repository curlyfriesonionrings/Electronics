# Electronics
Showcase repository containing electronics projects

## Clapper
This project is a simple clap switch using a microphone, Arduino Nano, and relay. The Arduino Nano listens to the microphone output and tries to determine if a clap has taken place (usually indicated by a spike in reading, it responds to any loud sound) to turn a switch on or off. A relay is used so that the switch can be used to power traditional light bulb lamps.

In the folder are the source code for the Arduino Nano controller and schematic images.

## Dreamer
This project is a [lucid dream](https://en.wikipedia.org/wiki/Lucid_dream) inducer. One of the keys to lucid dreaming is being able to realize the dream while the dream is in process. This gives the dreamer the ability to control what happens in the dream. The idea behind lucid dream induction is that a sign can be sent from the physical world that will manifest itself in the dream in some way to help the dreamer come to the realization of the current dream.

To try and induce a lucid dream, this device uses IR LEDs, an Arduino Nano, and regular red LEDs. A pair of IR LEDs are used in conjunction with phototransistors to measure eye movement. If the IR LED emits light onto the human eyelid, a phototransistor can measure the reflected light. During REM sleep, when the chance of dreaming is high, the eye moves rapidly in saccades. This movement is reflected in the phototransistor readings, which, if fed into a microcontroller (Arduino Nano), can signify if a person is in REM sleep. If a person is in REM sleep, the Arduino Nano triggers a sequence of flashes using the regular LEDS, which emit light through the eyelids to act as a dream signal. Ideally, these flashes of light will propagate into the user's dream, and the dreamer can use that signal to take control.

The microcontroller set up, along with the IR and regular LEDs need to be affixed to some contraption that can go over the user's face (particularly eyes). The IR LEDs and regular LEDs need to be positioned very close to the human eye to get accurate readings, but not so close that it is uncomfortable to sleep with. For this project, I built a custom sleep mask out of fabric that had a pocket for the circuit and holes for mounting the LEDs in place. Red LEDs are used because (in my opinion) red was least obtrusive during sleeping.

The folder for this project contains the Arduino Nano code and a protoboard schematic image.

## Mind Sync
This project is essentially a [mind machine](https://en.wikipedia.org/wiki/Mind_machine), although it is also referred to as a brainwave sychronizer or a light-and-sound machine. The goal is to induce [brainwave synchronization](https://en.wikipedia.org/wiki/Brainwave_entrainment) using lights and sound. More information can be readily found around the internet, but the idea is that a person's brain wave states can be altered by external stimulation of lights and sound. The machine can control a pattern of flashing lights (to achieve a particular frequency), as well as emit a noise of a particular frequency using a [binaural beat](https://en.wikipedia.org/wiki/Binaural_beats). With extended exposure to these lights and sounds, the brain will begin to match the frequency of the stimuli and the user will start to the feel the effects of the brain wave frequency adjustment (into alpha, beta, gamma, or delta brain wave states).

The circuit was a project in an _Evil Genius_ electronics project book. It uses three 555 timers to generate the required waves for both lights and sound, and features four potentiometers to act as controls for adjusting frequency. A pair of goggles needs to be custom made to house LEDs to flash in the users eyes (while closed). I used safety goggles and drilled holes to mount the LEDs, and a custom 3.5mm jack was used to connect the goggles to the machine. I also modeled a custom enclosure for the circuit and used a 3D printer to print the enclosure to house the circuit and battery.

The folder contains the .stl models for the enclosure, as well as protoboard and schematic images.
