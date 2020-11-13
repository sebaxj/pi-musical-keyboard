<<<<<<< HEAD
# slemhadri-sebaxj-project

TODO:

1) Figure out how to play different notes using the audio jack on the Pi
2) Adjust keyboard module to correlate scancode to specific note to be played
3) Create graphical interface of a 1-octave piano

How to play different notes: 
- Figure out the frequency for each note. 
- Solve for x : Frequency = 9,600,000/ (256 * 256) 
- Convert the x in a binary fraction 
- Left shift it the right amount so that the integer part end at the 8 MSB bits of PHASE. 

We should set a function which does that the conversion for a given frequency : 
I'm trying to understand how to deal with fractions in c (floats) and convert it to a binary fraction. 

Note	Frequency 	Coefficient
A	220		1.50186667
A#	233.082		1.59117312
B	246.942		1.68579072
C	261.626		1.78603349
C#	277.183		1.89223595
D	293.665		2.00475307
D#	311.127		2.12396032
E	329,628		2,25026048
F	349,228		2,384063147
F#	369,994		2,525825707
G	391,995		2,6760192
G#	415,305		2,8351488

Key-to-Note Mapping:

Keyboard:   Scancode:   Frequency:

Q           0x10        A
W           0x11        A#
E           0x12        B
R           0x13        C
T           0x14        C#
Y           0x15        D
U           0x16        D#
I           0x17        E
O           0x18        F
P           0x19        F#
[{          0x1a        G
]}          0x1b        G#

    
=======
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
>>>>>>> cbdb4bc0c677c8e07002829adbd282344007b524
