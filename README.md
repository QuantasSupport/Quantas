# QUANTAS
A Quantitative User-friendly Adaptive Networked Things Abstract Simulator.

This project is a simulator that enables quantitative performance analysis of distributed algorithms. QUANTAS is an abstract simulator, therefore, the obtained results are not affected by the specifics of a particular network or operating system architecture. QUANTAS allows distributed algorithms researchers to quickly investigate a potential solution  and collect data about its performance. QUANTAS programming is relatively straightforward and is accessible to theoretical researchers. 

<img src="Documentation/abstract%20sim%20draw.pptx.jpg" alt="System Diagram" style="zoom: 33%;" />

#### Dependencies:

By default, QUANTAS is set up for Linux and uses the `g++ 9` compiler. Use this to install `g++ 9`:

```sh
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-9
```

If you would rather use `clang`, use `make clang`.

We are using the JSON interpreter library available on Github [here](https://github.com/nlohmann/json).

We are using the thread pool library available on Github [here](https://github.com/bshoshany/thread-pool).

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
```
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

The next step is selecting the input file for the debugger. 
To do this select **Project** -> **Solution Properties** -> **Debugging** in the **Command Arguments** field you can select your input file likely having to resolve the filepath. For example: C:\Users\User\Documents\QUANTAS\quantas\ExampleInput.json

The next step is to define the appropriate preprocessing symbol.
1. Select **Project** -> **Properties** from the main menu (OR, right-click on the project node in **Solution Explorer** and select **Properties**).
2. Select **Configuration Properties** -> **C/C++** -> **Preprocessor**.
3. Select the drop-down menu for the **Preprocessor Definitions** field and then select **Edit**.
4. In the **Preprocessor Definitions** dialog box, add the definition of your desired alogithm (one definition per line). For example, your **Preprocessor Definition** dialog box may look like the following.
```
WIN32
_DEBUG
_CONSOLE
EXAMPLE_PEER
```
5. Finally, select **OK** to save changes.

Finally, make sure the language standard is set to C++17 or newer. This can be done by selecting **Project** -> **Properties** -> **Configuration Properties** -> **C/C++** -> **Language**. Select the drop-down menu for the **C++ Language Standard** field and then select **ISO C++17 Standard (/std:c++17)** or any newer standard, e.g., **ISO C++20 Standard (/std:c++20)**

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
