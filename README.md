# Juice

![Terrible Screenshot](https://github.com/natewaddoups/juice/raw/main/media/TinyGrainyScreenshot.jpg)

# What is it?

Juice is an editor and simulator for jointed and motorized contraptions.

* Create simple geometric objects
* Drag them around with a mouse
* Resize them with your keyboard
* Connect them with hinges or sliding joints
* Motorize the joints with simple wave functions
* Create complex control systems with a visual programming language
* Use multilevel undo/redo to erase and repeat your mistakes

# Why is it?

In late 2001, I set out to build a small walking robot, and figured I would start
by simulating it, using Russ Smith's Open Dynamics Engine (see http://ode.org/). 
Specifying robot geometry in C++ was hard, so I started building a GUI to
define and "motorize" jointed rigid bodies.

One thing led to another, and I ended up with a GUI that I was quite proud of, complete
with multilevel undo/redo and a built-in visual programming language to specify robot 
motion. I even had virtual quadrupeds, hexapods, and bipeds walking around under joystick control.  Snakes, too!

The name Juice was an homage to SodaPlay, a (defunct) browser-based contraption construction kit that provided significant inspiration for this project.

# When is it?

It started around the turn of the century. Then life continued, other hobbies took over, and I forgot about the whole thing. 

Around 2010, I couldn't get it to compile with then-current compilers.

In 2021, I figured I should get it compiled again, and put it on GitHub. 

# How is it?

Grab the [latest zip file](https://github.com/natewaddoups/juice/releases/latest) and load some of the included models. Maximum enjoyment requires a joystick, at least for now.

It mostly works, but the terrain subsystem and MPEG recorder are both disabled. They were based on third-party libraries that I still need to track down and re-integrate.
