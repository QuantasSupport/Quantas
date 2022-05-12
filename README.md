# QUANTAS
A Quantitative User-friendly Adaptive Networksed Things Abstract Simulator.

This project is a simulator that enables quantitative performance analysis of distributed algorithms. It has a number of attractive features. QUANTAS is an abstract simulator, therefore, the obtained results are not affected by the specifics of a particular network or operating system architecture. QUANTAS allows distributed algorithms researchers to quickly investigate a potential solution  and collect data about its performance. QUANTAS programming is relatively straightforward and is accessible to theoretical researchers. 

<img src="Documentation/abstract%20sim%20draw.pptx.jpg" alt="System Diagram" style="zoom: 33%;" />

#### Dependencies:

Our simulator uses the g++ compiler. If you do not have this installed on the machine run the below command in the command prompt to install the compiler

```sh
sudo apt install g++
```

Alternitavely you can use the Clang option from the makefile to use the clang++ compiler.

#### Basic usage
To use our simulator one must first clone the repository.
Once cloned you can easily run any of our example algorithms by running the below command from the root directory

```sh
make prod
./quantas.out quantas/ExampleInput.json

```

Changing ExampleInput.json to any of the available input files.

#### MacOS usage
```sh
make Clang
./quantas.out quantas/ExampleInput.json

```

#### Windows commands
```sh
make Windows
./quantas.exe ./quantas/ExampleInput.json

```

#### Usage with Visual Studio

To use our simulator with the Visual Studio editor takes additional steps.
First, you'll need to create an empty solution.
Next, right click on the solution explorer, in the dropdown menu select add from existing.
You'll need to add all the .cpp and .hpp files in the quantas folder and Common subfolder.

The final step is selecting the input file for the debugger. 
To do this select Project->Solution Properties->Debugging  in the Command Arguments field you can select your input file likely having to resolve the filepath. For example: C:\Users\User\Documents\QUANTAS\quantas\ExampleInput.json

#### Further description of the simulator

QUANTAS is further described in the paper:
[QUANTAS: Quantitative User-friendly Adaptable Networked Things Abstract Simulator](https://arxiv.org/abs/2205.04930)
