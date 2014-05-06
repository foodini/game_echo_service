This project provides you with a way to push local UDP traffic out to a remote
echo server, then accept the echoed data back and forward it to its final
destination. This is most useful to developers who are testing internet-
enabled games that they wish to run between nodes on their local network, but
want to be able to see how the game will behave under real network load and
loss.

I wrote this a while back in order to provide a tangible answer to my question
on stackoverflow:

http://stackoverflow.com/questions/2329059/network-simulator/2362274#2362274

(There, I'm "sniggerfardimungus".)

Because the code I describe there was owned by the company that employed me at
the time, I wrote it all from scratch a year or so ago. This repository is the
result of that effort.

This version does not encrypt your echoed traffic, as described on 
stackoverflow. That will be added later, if there is sufficient interest. It
does set up a Unity policy server, since I have a current interest in using
Unity for something else I'm doing.

It should build on Windows, using Visual Studio, cygwin, mac, and linux. I
will formalize that support better as I find time to mature the project.


proxy.cpp is the proxy server itself. It accepts TCP connections and routes
each through the echo server before forwarding the data along to their
destination. See the help text in game.cpp for a description of how to provide
the game server (the destination) and the echo server info.

terminator.cpp is the echo server. It takes a single argument, the port number
to listen on.
