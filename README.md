This is a header only library that allows you to add a Lisp-ish script engine
to your C++ programs or S-expression serialization.

A good scenario for it is to use it as a rpc protocol. The client, depending on
its needs, can send the script containing what it wants to do server side. If
it needs to call two functions, it can do it in one shot by sending a script
that calls those two functions. If it wants to call a remote function that
returns a big deal of data but is interested only in part of it, it can filter
in the script that it sends to the server. Etc.
