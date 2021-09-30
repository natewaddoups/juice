1. Run the installer for MXSML6. (It is referenced indirectly, via COM.)

2. Unzip WTL10_10320_Release.zip into a new folder in this directory, named WTL10_10320.

3. Get ODE by running "git submodule https://bitbucket.org/odedevs/ode.git"
   Run these commands:

   cd ode\build
   premake4 vs2010
   cd vs2010
   start ode.sln

   Build the solution.