# QUANTAS
A Quantitative User-friendly Adaptive Networked Things Abstract Simulator.

This project is a simulator that enables quantitative performance analysis of distributed algorithms. QUANTAS is an abstract simulator, therefore, the obtained results are not affected by the specifics of a particular network or operating system architecture. QUANTAS allows distributed algorithms researchers to quickly investigate a potential solution  and collect data about its performance. QUANTAS programming is relatively straightforward and is accessible to theoretical researchers. 

<img src="Documentation/abstract%20sim%20draw.pptx.jpg" alt="System Diagram" style="zoom: 33%;" />

#### Dependencies:

By default, QUANTAS is set up for Linux and uses the `g++` compiler. Use this to install `g++`:

```sh
sudo apt install g++
```

If you would rather use `clang`, use `make clang`.

#### Basic Usage
To use the simulator, first clone the repository. Once cloned, you need to configure the simulated algorithm for QUANTAS to run and an input file for the algorithm to use. QUANTAS comes with several example algorithms and input files. They are listed in the `makefile` in the root directory. Uncomment the required algorithm and input file, for example:
 
    INPUTFILE := $(PROJECT_DIR)/ExampleInput.json
    ALG := EXAMPLE_PEER
    ALGFILE := ExamplePeer

selects `EXAMPLE_PEER` algorithm and `ExampleInput` input file.

Once configured, 
```sh
make release 
```
compiles the simulator with the algorihtm, and
``` sh
make run
```
runs the compiled simulation with the speicifed input file.
       
```sh
make debug
````
compiles the simulator for debugging.


#### MacOS
```sh
make clang
```

#### Visual Studio

To use our simulator with the Visual Studio editor takes additional steps.
First, you'll need to create an empty solution.
Next, right click on the solution explorer, in the dropdown menu select add from existing.
You'll need to add all the .cpp and .hpp files in the quantas folder and Common subfolder.

The final step is selecting the input file for the debugger. 
To do this select Project->Solution Properties->Debugging  in the Command Arguments field you can select your input file likely having to resolve the filepath. For example: C:\Users\User\Documents\QUANTAS\quantas\ExampleInput.json

#### Further Info

QUANTAS is further described here:
[QUANTAS: Quantitative User-friendly Adaptable Networked Things Abstract Simulator](https://arxiv.org/abs/2205.04930)
```
@article{oglio2022quantitative,
  title={QUANTAS: Quantitative User-friendly Adaptable Networked Things Abstract Simulator},
  author={Oglio, Joseph and Hood, Kendric and Nesterenko, Mikhail and Tixeuil, Sebastien},
  journal={arXiv preprint arXiv:2205.04930},
  year={2022}
}
```