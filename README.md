# NAPS

NAPS (Network automatic parametrization and Simulation) is a tool that aims at finding networks&#39; parameters, particularly of metabolic networks, to obtain a stable behaviour of the desired components through their simulation in a circuit-like environment.

The main building blocks of a metabolic network are metabolites and reactions, whose description is usually contained in an SBML file. These networks can be seen as Petri nets, with each metabolite being a place and each reaction being a transition. Each one of these components is modeled as a finite-state machine and is implemented as a separate module, in SystemC language, that is connected to the others through a wiring-like system, typical of EDA.

The tool is divided in several parts and its execution follows a pipeline that starts from an SBML file containing information about a particular network that goes through a Python-based SBML parser, in order to be translated in SystemC modules. These modules are then compiled (in C++ language typical way) and  the simulation can be executed.

Easier done than said  :-)

## NAPS Installation and Usage
In order to use NAPS on your machine you should be able to compile and execute a C++ code and have Python 3 (recommended 3.6 and above) installed.
Then you have to install dependencies: LibSBML (version 5.18.0) and SystemC (version 2.3.3). 
We strongly recommend to use a Linux machine but you can (almost for sure) use also this software without particular changes both on MacOS and Windows (in this case the best way we can suggest is to use WSL, in particular to install SystemC).

### Installation of dependencies

#### SystemC
The SystemC library can be downloaded at the following link:
https://www.accellera.org/downloads/standards/systemc

**N.B.:** Please, in case of future updates, be sure to download the **correct version** (2.3.3), otherwise something might not work properly.

After downloading the archive, unzip it with this command:
```bash
tar xzfv systemc-2.3.3.tar.gz
```

Configure the installation:
```bash
cd systemc-2.3.3
mkdir build
mkdir -/pse_libraries
mkdir -/pse_libraries/systemc
cd build
../configure --prefix=<home directory>/pse_libraries/systemc
```

Compile:
```bash
make
make install
export SYSTEMC_HOME=<home directory>/pse_libraries/systemc
```

Copy and paste the last command (line 3 of last code block) as last line of ~/.bashrc file to permanently set the environment variable SYSTEMC_HOME.
Pay attention to change **&lt;home directory>** in your own user path.

#### LibSBML
LibSBML is a library for working with SBML content. It supports many programming languages and operating systems. We use the Python-based version.
LibSBML can be downloaded following the instructions at the following link:
http://sbml.org/Software/libSBML/Downloading_libSBML

Our software needs LibSBML for Python. Feel free to use whatever method described in Python section to get it. We recommend to get version 5.18.0 to be sure it works.

### Usage
After the installation of dependencies, you can run one of our given test networks and check if everything works properly.

The following commands allow you to get information from SBML file and generate SystemC modules:
```bash
python3 ./py_files/main.py ./SBML_Networks_test_examples/test1_purine.xml
```

Compile all the generated SystemC code:
```bash
cd generated_systemc/
make
```

Run stochastic simulation on the generated network:
```bash
./bin/Metabolite_Network.x
```

At the end, in the parent folder, you should have a file **&#39;met_concentrations.txt&#39;** that contains information about metabolites&#39; trends during simulation.
We also provide an R script to create a PNG image to plot data in a simple way, just to let you have an idea of the simulation.

You can take whatever network you want and simulate with our software. 
Enjoy!
