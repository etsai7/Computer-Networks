# Video Streaming via CDN

## Overview

Video traffic dominates the Internet. In this project, we will explore how video content distribution networks (CDNs) work. In particular, this project will implement adaptive bitrate selection, DNS load balancing, and an HTTP proxy server to stream video at high bit rates from the closest server to a given client.

### Video CDNs in the Real World
Clients trying to stream a video first issue a DNS query to resolve the service's domain name to an IP address for one of the CDN's content servers. The CDN's authoritative DNS server selects the “best” content server for each particular client based on
(1) the client's IP address (from which it learns the client's geographic location) and
(2) current load on the content servers (which the servers periodically report to the DNS server).

### Video CDN in this Project
This implementation is a simplified version of a CDN. First, the entire system runs on one host and relies on mininet to run several processes with arbitrary IP addresses on one machine. Mininet will also allow you to assign arbitrary link characteristics (bandwidth and latency) to each pair of “end hosts” (processes).

<img src="./Images/our-CDN.png" title="Video CDN in assignment 2" alt="" width="330" height="111"/>

The gray-shaded components in the figure above are implemented

**Browser.** An off-the-shelf web browser (Firefox) will be used to play videos served by the CDN (via your proxy).

**Proxy.** Rather than modify the video player itself, the HTTP proxy implements an adaptive bitrate selection. The player requests chunks with standard HTTP GET requests; your proxy will intercept these and modify them to retrieve whichever bitrate your algorithm deems appropriate. To simulate multiple clients, launch multiple instances of the proxy.

**Web Server.** Video content will be served from an off-the-shelf web server (Apache). As with the proxy, multiple instances of Apache will be run on different IP addresses to simulate a CDN with several content servers.

**DNS Server.** A simple DNS that supports only a small portion of actual DNS's functionality. The server will respond to each request with the “best” server for that particular client.

To summarize, this project has the following components:

* [Part 1](#part1): Bitrate Adaptation in HTTP Proxy
* [Part 2](#part2): DNS Load Balancing