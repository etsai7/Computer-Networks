# Server and Client Data Transmission

## Overview
`iPerf` is a common tool used to measure network bandwidth. `iPerfer` is a version of this tool in C/C++ using sockets. This tool is used to measure the performance of virtual networks in Mininet and explain how link characteristics and multiplexing impact performance.

* [Part 1](#part1): Using `iPerfer`
* [Part 2](#part2): Using Mininet
* [Part 3](#part3): Custom Topologies

<a name="part1"></a>
## Part 1: Using iPerfer
`iPerfer` will require a client and a server connected to the host IP and Port to begin transmitting data

### Server Mode

To operate iPerfer in server mode, it should be invoked as follows:

`./iPerfer -s -p <listen_port>`

* `-s` indicates this is the `iPerfer` server which should consume data
* `listen_port` is the port on which the host is waiting to consume data; the port should be in the range 1024 ≤ listen port ≤ 65535

You can use the presence of the `-s` option to determine `iPerfer` should operate in server mode.

### Client Mode

To operate `iPerfer` in client mode, it should be invoked as follows:

`./iPerfer -c -h <server_hostname> -p <server_port> -t <time>`

* `-c` indicates this is the `iPerfer` client which should generate data
* `server_hostname` is the hostname or IP address of the `iPerfer` server which will consume data
* `server_port` is the port on which the remote host is waiting to consume data; the port should be in the range 1024 ≤ `server_port` ≤ 65535
* `time` is the duration in seconds for which data should be generated

You can use the presence of the `-c` option to determine `iPerfer` should operate in client mode.

**NOTE:** The client is set up by default to send chunks of 1000 bytes of data.

<a name="part2"></a>
## Part 2: Using Mininet

To test `iPerfer`, I used Mininet to create virtual networks and run simple experiments. According to the [Mininet website](http://mininet.org/), *Mininet creates a realistic virtual network, running real kernel, switch and application code, on a single machine (VM or native), in seconds, with a single command.* We will use Mininet in programming assignments throughout the semester.

### Running Mininet

We will be using 64-bit [Mininet 2.2.1](https://github.com/mininet/mininet/wiki/Mininet-VM-Images). To run Mininet, you will need a virtual machine (VM). We will be using [VirtualBox](https://www.virtualbox.org/), which is a free and open-source hypervisor.

When you boot up the VM, you'll have to log in using `mininet` and `mininet` as username and password.

Alternatively, if you have trouble with the method above, you can also set up a local VM with a recent version of Ubuntu (I didn't run into any problems using Ubuntu 14.04) and follow options 2 or 3 listed [here](http://mininet.org/download/#option-2-native-installation-from-source).

#### GUI in MininetVM

Details can be found at <https://github.com/mininet/mininet/wiki/FAQ#vm-console-gui>, but you can follow this basic set of instructions to setup a working GUI:

* Update: `sudo apt-get update`
* Install the `lxde` desktop environment: `sudo apt-get install xinit lxde`
* Install VirtualBox guest additions: `sudo apt-get install virtualbox-guest-dkms`
* Restart the VM: `sudo reboot`
* Start the GUI using `startx`

To avoid having to `startx` every time you boot, you can add the following snippet to the end of your `~/.bashrc`:

```
if [ -z "$DISPLAY" ] && [ -n "$XDG_VTNR" ] && [ "$XDG_VTNR" -eq 1 ]; then
  exec startx
fi
```

To launch hosts: `xterm <hostname>`

This will launch a separate terminal to run `iPerfer` from that host and its IP address and Port

To transfer files to/from your VM you should use the `scp` (secure copy) command. See the `scp` man page, or find a tutorial online, for instructions on how to use `scp`.

<a name="part3"></a>

## Part 3: Custom Topologies

Some topologies are provided [here](https://github.com/etsai7/Computer-Networks/tree/master/Hw1/Topologies)

To run Mininet with the provided topology, run the Python script `Original_Topology.py` using sudo:

`sudo python Original_Topology.py`

This will create a network with the following topology:

<img src="./Topologies/Original_Topology.png" title="Assignment 1's topology" alt="Should be showing the topology described in assignment1_topology.py" width="350" height="220"/>

If you have trouble launching the script, a common fix is to first try running `sudo mn -c`, and then try launching the script again.

Hosts (`h1` through `h10`) are represented by squares and switches (`s1` to `s6`) are represented by circles; the names in the diagram match the names of hosts and switches in Mininet. The hosts are assigned IP addresses 10.0.0.1 through 10.0.0.10; the last number in the IP address matches the host number.

**NOTE:** When running ping and `iPerfer` in Mininet, you must use IP addresses, not hostnames.