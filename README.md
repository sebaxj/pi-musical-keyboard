# slemhadri-sebaxj-project

FILE STRUCTURE: app is musical_ps2_keyboard.c
 - use audio driver to control audio output with PWN through audio jack
 - use keyboard driver to initiate notes
 - use graphic modules to display piano graphic on console with a shell to select intruments, etc. 

TODO:

1) Figure out how to play different notes using the audio jack on the Pi. DONE (try out play.c in src/apps with the different phases). 
2) Adjust keyboard module to correlate scancode to specific note to be played
3) Create graphical interface of a 1-octave piano
4) We should set a function which does that the conversion for a given frequency : 
I'm trying to understand how to deal with fractions in c (floats) and convert it to a binary fraction. 
5) Waiting to figure out how to do that, there are online converters : https://www.exploringbinary.com/binary-converter/

How to play different notes: 
- Figure out the frequency for each note. 
- Solve for x : Frequency = x.(9,600,000/ (256 * 256))
- Convert the x in a binary fraction 

| Keyboard | Scancode | Note Name | Frequency (Hz) | Phase       | Phase as Binary Fraction    |
|----------|----------|-----------|----------------|-------------|-----------------------------|
| Q        | 0x10     | A         | 220.000        | 1.50186667  | 1.100000000111101001010101  |
| W        | 0x11     | A#        | 233.082        | 1.59117312  | 1.100101110101011100011111  |
| E        | 0x12     | B         | 246.942        | 1.68579072  | 1.101011111000111111111011  |
| R        | 0x13     | C         | 261.626        | 1.78603349  | 1.110010010011100101111101  |
| T        | 0x14     | C#        | 277.183        | 1.89223595  | 1.111001000110100110010011  |
| Y        | 0x15     | D         | 293.665        | 2.00475307  | 10.000000010011011101111111 |
| U        | 0x16     | D#        | 311.127        | 2.12396032  | 10.000111111011101111011101 |
| I        | 0x17     | E         | 329.628        | 2.25026048  | 10.010000000001000100010010 |
| O        | 0x18     | F         | 349.228        | 2.384063147 | 10.011000100101000111110110 |
| P        | 0x19     | F#        | 369.994        | 2.525825707 | 10.100001101001110010000011 |
| [{       | 0x1A     | G         | 391.995        | 2.6760192   | 10.101011010000111110011000 |
| ]}       | 0x1B     | G#        | 415.305        | 2.8351488   | 10.110101011100110001001111 |

*********************************

# CS107E Evaluation:

## Project title

## Team members
Sebastian Xavier James - Isma Lemhadri 

## Project description
We aim to make a musical piano keyboard, where we could play the 12 keys of one octave on specific keys of the keyboard. 

## Member contribution
A short description of the work performed by each member of the team.

## References
Cite any references/resources that inspired/influenced your project. If your submission incorporates external contributions such as adopting someone else's code or circuit into your project, be sure to clearly delineate what portion of the work was derivative and what portion is your original work.

## Self-evaluation
How well was your team able to execute on the plan in your proposal?  Any trying or heroic moments you would like to share? Of what are you particularly proud: the effort you put into it? the end product? the process you followed? what you learned along the way? Tell us about it!

## Photos
You are encouraged to submit photos/videos of your project in action. Add the files and commit to your project repository to include along with your submission.
