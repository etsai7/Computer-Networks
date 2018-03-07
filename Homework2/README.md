# Video Streaming Using Bitrate Adaptation

## Overview

Video traffic dominates the Internet. In this project, we will explore how video content distribution networks (CDNs) work. In particular, this project  implement an HTTP proxy server to stream video at high bit rates using adaptive bitrate selection process.


<img src="./Images/our-CDN.png" title="Video CDN in assignment 2" alt="" width="330" height="111"/>

The gray-shaded `Proxy` component in the figure above are implemented

**Browser.** An off-the-shelf web browser (Firefox) will be used to play videos served by the CDN (via your proxy).

**Proxy.** Rather than modify the video player itself, the HTTP proxy implements an adaptive bitrate selection. The player requests chunks with standard HTTP GET requests; your proxy will intercept these and modify them to retrieve whichever bitrate your algorithm deems appropriate. To simulate multiple clients, launch multiple instances of the proxy.

**Web Server.** Video content will be served from an off-the-shelf web server (Apache). As with the proxy, multiple instances of Apache will be run on different IP addresses to simulate a CDN with several content servers.

To summarize, this project has the following components:

* [Part 1](#part1): Bitrate Adaptation in HTTP Proxy

[This VM](http://www.cs.jhu.edu/~hzhu/proj2.ova) has all the components needed to get started on the project. This VM includes mininet, Apache, and all the files we will be streaming in this project. Both the username and password for this VM are `proj2`. To start the Apache server, simply run the python script we provide by doing the following:

`python start_server.py <host_number>`

Here `<host_number>` is a required command line argument that specifies what host you are running on Mininet. This is important as if you're running on h1 in Mininet (which is given the IP address 10.0.0.1), passing in `1` into the `<host_number>` argument will help ensure that the Apache server instance will be bound to the 10.0.0.1 IP address. The `<host_number>` argument must be between 1 and 8.

If you are testing locally, and simply wish to launch the server locally, you can run the following command:

`/usr/local/apache2/bin/apachectl start`

For this project, we will be using an off the shelf browser (in this case, it is Firefox). To launch Firefox for this project, run the following command:

`python launch_firefox.py <profile_num>`

Here `<profile_num>` is a required command line argument that specifies the instance of Firefox you are launching. We support launching profiles 1-8, however, should you feel the need to test more thoroughly, you can launch it with a different number and simply create a new profile as needed. To ensure a separate connection for each instance of Firefox, we recommend that you launch Firefox with a different profile number (otherwise you might notice that different Firefox instances will sometimes share a connection with your proxy browser).

**NOTE:** For this project, we are disabling caching in the browser. If you do choose to create a new profile, please be sure to go to the `about:config` page and set both `browser.cache.disk.enable` and `browser.cache.memory.enable` to `false`.

<a name="part1"></a>
## Part 1: Bitrate Adaptation in HTTP Proxy

Many video players monitor how quickly they receive data from the server and use this throughput value to request better or lower quality encodings of the video, aiming to stream the highest quality encoding that the connection can handle. Instead of modifying an existing video client to perform bitrate adaptation, you will implement this functionality in an HTTP proxy through which your browser will direct requests.

You are to implement a simple HTTP proxy, `miProxy`. It accepts connections from web browsers, modifies video chunk requests as described below, resolves the web server's DNS name, opens a connection with the resulting IP address, and forwards the modified request to the server. Any data (the video chunks) returned by the server should be forwarded, *unmodified*, to the browser.

`miProxy` should listen for browser connections on `INADDR_ANY` on the port specified on the command line. It should then connect to web servers either specified on the command line (see below) or issue a DNS query to find out the IP address of the server to contact (this is covered in part 2).


Path to Apache file /var/www