# Nebula Device
This is an attempt of getting the nebula device engine to compile and run on modern systems. The engine was used for the Project Nomads (2002).  
The original, unchanged code can be found at: <https://github.com/dgiunchi/m-nebula>  
  
**This project is WIP and does not compile yet.**

## Build Instructions
Prerequisites:
* GCC 4.3 <https://aur.archlinux.org/packages/gcc43>
* Tclsh, Tcllib

Clone and Build:
```
git clone https://github.com/tdc22/m-nebula.git
cd m-nebula/code/src
tclsh updsrc.tcl

NOMADS_HOME=~/m-nebula
NEBULADIR=~/m-nebula
PATH=$PATH:$NOMADS_HOME/bin:$NOMADS_HOME/bin/linux
LD_LIBRARY_PATH=$NOMADS_HOME/bin/linux:$LD_LIBRARY_PATH
export OSTYPE NOMADS_HOME NEBULADIR PATH LD_LIBRARY_PATH

make CC=gcc-4.3 CPP=g++-4.3 CXX=g++-4.3 LD=g++-4.3
```

## Changes
* **Removed STLPort**: this library was an implementation of the C++ std library. It hasn't been updated since 2008 and it's tricky to even compile on modern systems.
