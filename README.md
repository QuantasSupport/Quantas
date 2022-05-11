# QUANTAS
A Quantitative User-friendly Adaptive Networksed Things Abstract Simulator.

This project is a simulator that enables quantitative performance analysis of distributed algorithms. It has a number of attractive features. QUANTAS is an abstract simulator, therefore, the obtained results are not affected by the specifics of a particular network or operating system architecture. QUANTAS allows distributed algorithms researchers to quickly investigate a potential solution  and collect data about its performance. QUANTAS programming is relatively straightforward and is accessible to theoretical researchers. 

<img src="Documentation/abstract%20sim%20draw.pptx.jpg" alt="System Diagram" style="zoom: 33%;" />

#### Dependencies:

Our simulator uses the g++ compiler. If you do not have this installed on the machine run the below command in the command prompt to install the compiler:

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