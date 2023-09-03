# savemid
c++ library for creating your own midi files

# Example Usage
Let's say we wanna create a file that has 120 BPM, 96 ticks per quarter note and a name "c4note.mid", and then call a C4 Note that has 100 velocity and gonna last a quarter note which will start in the begining of the pattern.
```cpp
#include "MidiFile.h"

int main()
{
    using enum Notes;
    // Creating a file by using this constructor.
    MidiFile midi("c4note.mid", 120, 96);
    midi.NoteOn(0, C4, 100);
    midi.NoteOff(96, C4);
    // Always call this when you are done with the file.
    midi.EndFile();
}
```
This is gonna create the note indeed, but this is gonna get so messy, calling NoteOn and NoteOff everytime we create a one note, but i got ya with this:
```cpp
#include "MidiFile.h"

int main()
{
    using enum Notes;
    MidiFile midi("c4note.mid", 120, 96);
    midi.WriteNotes(Note(0, 96, C4, 100));
    midi.EndFile();
}
```
Same result as the previous one, and only one line!
As a matter of a fact, this is a variadic template, you can input as many notes as you would like to write and the result will be exactly the same!
```cpp
midi.WriteNotes(
  Note(0, 96, C4, 100),
  Note(0, 96, D4, 100),
  Note(0, 96, E4, 100),
  Note(0, 96, F4, 100),
  Note(0, 96, G4, 100),
  Note(0, 96, A4, 100),
  Note(0, 96, B4, 100),
  Note(0, 96, C5, 100)
);
```
Well that's nice and all, but how about creating a chord??
```cpp
#include "MidiFile.h"

int main()
{
    using enum Notes;
    MidiFile midi("nicechord.mid", 120, 96);
    midi.WriteChord(0, 96, C4, E4, G4, B4);
    midi.EndFile();
}
```
Once again, you can write as many notes as you want to create a chord with.

# maybe i will update this with more info soon
