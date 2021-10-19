# Juice

![Terrible Screenshot](https://github.com/natewaddoups/juice/raw/main/media/TinyGrainyScreenshot.jpg)

In late 2001, I set out to build a small walking robot, and figured I would start
by simulating it, using Russ Smith's Open Dynamics Engine (see http://ode.org/). 
Specifying robot geometry in C++ was hard, so I started building a GUI to
define and "motorize" jointed rigid bodies.

One thing led to another, and I ended up with a GUI that I was quite proud of, complete
with multilevel undo/redo and a built-in visual programming language to specify robot 
motion. I even had virtual quadrupeds, hexapods, and bipeds walking around under joystick control.  Snakes, too!

Life continued, other hobbies took over, and I forgot about the whole thing. 

Ten years later, I couldn't get it to compile with then-current compilers.

Twenty years later, I figured I should get it compiled again, and put it on GitHub. 

It's going to need more rehabilitation. But, to my great surprise, it mostly works.

Grab the [latest zip file](https://github.com/natewaddoups/juice/releases/latest) and load some of the included models. Maximum enjoyment requires a joystick, at least for now.

The name Juice was an homage to SodaPlay, a (defunct) browser-based contraption construction kit that provided significant inspiration for this project.
