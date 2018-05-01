# Network Features with P4

In this assignment, you will use P4 and Mininet to design network features.

- [Introduction](#introduction)
- [Obtaining required software](#obtaining-required-software)

## Introduction
This assignment includes 2 exercises: *Source Routing*
and *Key-Value Store*. Both exercises assume that you possess basic networking
knowledge and some familiarity with the P4 language. Please take a look at the
[P4 language spec](https://p4.org/p4-spec/p4-14/v1.0.4/tex/p4.pdf) and at the example `simple_router`
target [on
p4lang](https://github.com/p4lang/p4factory/tree/master/targets/simple_router/p4src).
*Source Routing* asks you to write a P4 program to implement a
custom source routing protocol. *Key-Value Store* asks you to write a P4 program to implement a key-value store in the switch. We use P4_14 in this assignment.

## Obtaining required software

[We are providing a VM](http://www.cs.jhu.edu/~hzhu/proj4.ova) that has all the components you need to get started on the assignment. Or you can follow the instruction below to set up the environment manually.

Otherwise, you will need to clone 2 p4lang Github repositories and install their dependencies. To clonde the repositories:

- `git clone https://github.com/p4lang/behavioral-model.git bmv2`
- `git clone https://github.com/p4lang/p4c-bm.git p4c-bmv2`

The first repository ([bmv2](https://github.com/p4lang/behavioral-model)) is the
second version of the behavioral model. It is a C++ software switch that will
behave according to your P4 program. The second repository
([p4c-bmv2](https://github.com/p4lang/p4c-bm)) is the compiler for the
behavioral model: it takes P4 program and output a JSON file which can be loaded
by the behavioral model.

Each of these repositories come with dependencies. `p4c-bmv2` is a Python
repository and installing the required Python dependencies is very easy to do
using `pip`: `sudo pip install -r requirements.txt`.

`bmv2` is a C++ repository and has more external dependencies. They are listed
in the
[README](https://github.com/p4lang/behavioral-model/blob/master/README.md). If
you are running Ubuntu 14.04+, the dependencies should be easy to install (you
can use the `install_deps.sh` script that comes with `bmv2`). Do not forget to
build the code once all the dependencies have been installed:

- `./autogen.sh`
- `./configure`
- `make`

You will also need to install `mininet`, as well as the following Python
packages: `scapy`, `thrift` (>= 0.9.2) and `networkx`. On Ubuntu, it would look
like this:
- `sudo apt-get install mininet`
- `sudo pip install scapy thrift networkx`

**NOTE FOR MAC USERS**: OS X currently doesn't support native Mininet installation. You would need to do this assignment in a virtual ubuntu environment. The easiest way would probably be through running a pre-packaged Mininet/Ubuntu VM in VirtualBox, please read carefully and follow through the instructions in [here](http://mininet.org/download/). For later running `xterm` to open terminal on hosts, you probably need to install [XQuartz](https://www.xquartz.org).