# slemhadri-sebaxj-project

Update Monday : 
- (Isma) I modified the Makefile to make install the app you created. 
- I modified the musical_ps2_keyboard.c to run the shell with the keyboard.c version with interrupts. 
- I replaced your : console.c gl.c fb.c gpio_interrupts.c and keyboard.c with mine (I had some trouble running some of your code with some of mine so I replaced it, including the interrupts version) 
- When running a file, make sure that you switch RESET to 0 on keyboard.c (I have a keyboard that needs the reset code, but it seems like you don't ?). 
- I've implemented the "music" command in shell.c, for now, draw_piano() just clears the screen, and you can play one of the 12 KEYS (qwertyuiop[]) and it outputs the corresponding note. However, it only works for the first key for now, as the audio_write functions are still looping indefinitely. You can try the new shell with "make install", then typing "music"in the shell, it's supposed to just clear the screen for now (will draw the piano when we implement the graphic part), and waits for you to press one of the appropriate keys. 
- I deleted the files in include that we don't need (they're already in libpi and libpiextra) in my local repo, however I didn't delete them on the repo so you shouldn't see any change for now. I wasn't sure if you modified some of them or not. 

- Sebastian:
- Updated audio driver to have a time parameter which places a tone in units of
  125,000 us.
- Added support between keyboard and audio in shell and test file.
- Each keypress results in a tone duration of 125,000 us.
    - Next step would be to figure out how to handle a tone when a key is held
      down.
- Updated app to start console and boot into music mode. However, no tone is
  played when a key is pressed as it should when tested in the test file. This
  might be due to the #include "sin8.h" statement in the shell.c file. Or, it
  could be due to the initialization of the audio driver?
- I also updated the Makefile so that if you have the most recent git pull, it
  should compile the test and app correctly. In the /include file, you need 5 .h
  files.
- The tests start by playing a full A major scale, followed by a testing of
  key presses. These all work correctly, but the "music_cmd" in shell.c doesn't
  work correctly.
- Also, the keyboard driver wasn't working for me? When I used the reference
  implementation it worked correctly, or when I used by implementation it worked
  correctly. I'm not sure why this is, but I left both versions of the code in
  the keyboard.c file.

=> Isma (Tuesday) 
- Advanced somehow on the problem of playing sounds when the key is pressed. 
Try it out by running either the test keyboard part, or the shell directly. It 
doesn't sound great yet. The idea is that I enter a for loop where I keep playing 
the sound for 125,000 us again and again until I get a KEY_RELEASE (see function play_note) 
. Therefore, the sound is not great since we can hear little cuts. Besides, the first one 
is played and cuts then we enter the for loop with the "continuous" sound, idk why yet. 
- The app is working fine for me now, we were initializing the audio two times, once in
the main in apps, and once is the cmd_music. Now, we're only initializing it in the main
function. music_cmd seems to be working fine now. 
- We'll try to see when we meet today why my version of the keyboard is not working for you. 
I noticed that in the test file, you weren't enabling global interrupts. Maybe that's why ? 
I'm using my version of the keyboard from assignment 7 and not assigment 5, so it might be 
due to that. I've corrected the main function of the test file to enable the interrupts, 
you can try again maybe ? Don't forget to turn RESET to 0 at the beginning of the keyboard file
if you don't need the RESET code for the keyboard, it'll just comment out automatically the parts
concerning the RESET code.

=> Isma (WEDNESDAY) 
- Tried to resolve the cutting issue. Using a timer_delay before playing the note seems to make it better, 
however this causes a lag between the keyboard press and the sound coming out. I think 400 ms is a good 
trade-off here (at least the best I could find). 
- The shell has now a graphic interactive keyboard! I've modified gl.h, console.c and shell.c (I think some of the
functions in gl.h should be in console.h but i haven't really checked that yet). If you want any clarification, we can 
meet quickly. I'm afraid my code might be unclear in some parts. 
- Also, in the graphic keyboard, I put the A from the other octave because I think it's esthetically nicer , but it's 
not yet implemented in the shell (no key associated, and no phase yet). DONE => changed the key mapping. 

=> Sebastian (WEDNESDAY)
- See merge notes, and notes in slack.

FILE STRUCTURE: app is musical_ps2_keyboard.c
 - use audio driver to control audio output with PWN through audio jack
 - use keyboard driver to initiate notes (shell.c for now). 
 - use graphic modules to display piano graphic on console with a shell to select intruments, etc. 

TODO:

1) Adjust keyboard module to correlate scancode to specific note to be played. "music command" doesn't
work in shell, it only works in the testing implementation.
2) modify audio.c and shell.c to handle held-down keys correctly. 
3) Create graphical interface of a 1-octave piano.
4) We should set a function which does that the conversion for a given frequency : 
I'm trying to understand how to deal with fractions in c (floats) and convert it to a binary fraction. 
5) Waiting to figure out how to do that, there are online converters : https://www.exploringbinary.com/binary-converter/

Milestones: 
- Keyboard driver: => KEY-Press of the associated keys would like output the sound, and a KEY release would stop it. Sebastian 
- Command on the shell : "music" => displays the piano graphic and change the keyboard accordingly. Isma


How to play different notes: 
- Figure out the frequency for each note. 
- Solve for x : Frequency = x.(9,600,000/ (256 * 256))
- Convert the x in a binary fraction 

| Keyboard | Scancode | Note Name | Frequency (Hz) | Phase       | Phase as Binary Fraction    |
|----------|----------|-----------|----------------|-------------|-----------------------------|
| tab      | 0x0d     | A         | 220.000        | 1.50186667  | 1.100000000111101001010101  | 
| Q        | 0x15     | A#        | 233.082        | 1.59117312  | 1.100101110101011100011111  | 
| W        | 0x1d     | B         | 246.942        | 1.68579072  | 1.101011111000111111111011  | 
| E        | 0x24     | C         | 261.626        | 1.78603349  | 1.110010010011100101111101  | 
| R        | 0x2d     | C#        | 277.183        | 1.89223595  | 1.111001000110100110010011  | 
| T        | 0x2c     | D         | 293.665        | 2.00475307  | 10.000000010011011101111111 | 
| Y        | 0x35     | D#        | 311.127        | 2.12396032  | 10.000111111011101111011101 | 
| U        | 0x3c     | E         | 329.628        | 2.25026048  | 10.010000000001000100010010 | 
| I        | 0x43     | F         | 349.228        | 2.384063147 | 10.011000100101000111110110 | 
| O        | 0x44     | F#        | 369.994        | 2.525825707 | 10.100001101001110010000011 |
| P        | 0x4d     | G         | 391.995        | 2.6760192   | 10.101011010000111110011000 |
| [{       | 0x54     | G#        | 415.305        | 2.8351488   | 10.110101011100110001001111 |
| ]}       | 0x5b     | A_upper   | 440.000        | 3.0037333   | 11.000000001111010010101010 | 

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
