#pragma once

// bad interface, int for ipv4 address => and ipv6 ?
// input port to listen,
// input max number of incoming conenciton to serve
int loopbackserv(int address, int port, int max_connections);
