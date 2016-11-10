![SLIP](https://raw.githubusercontent.com/Vermeille/slip/master/slip.jpg)

# Wannano watidiz?

Yo man. You're comin' here, you see my stuff, we good man. And yo ask me dude
"Daaaaannnng maaaaaaan, dat soundz awesome dat coode dat you just doooone, but
waddaheck isdis?". So I'm tellin' you bro.

This is slip man. And it means "pants" in French. It's great, it's soft, and
what's inside gonna be huge bro, it'll make Lisp great again!

This is a header only library that allows you to add a Lisp-ish script engine
to your C++ programs or S-expression serialization.

A good scenario for it is to use it as a rpc protocol. The client, depending on
its needs, can send the script containing what it wants to do server side. If
it needs to call two functions, it can do it in one shot by sending a script
that calls those two functions. If it wants to call a remote function that
returns a big deal of data but is interested only in part of it, it can filter
in the script that it sends to the server. Etc.


# How to use?

1. Copy the src/ dir to wherever ya want. Make it yours. Love it, hug it,
   cuddle without fear!

2. Include it wherever you want it.

    ```c++
    #include "slip.h"
    ```

3. If ya want, include dat line, but I'm not endorsing this. Put that slip
   on forever.

    ```c++
    using namespace slip;
    ```

4. Create a Context for "compilation" and "linking". That means adding
   functions that will be available for your slip script.

    ```c++
    slip::Context ctx;
    ctx.DeclareFun("+", [](int a, int b) -> int { return a + b; });
    ctx.DeclareFun(
        "+", [](std::string a, std::string b) -> std::string { return a + b; });
    ctx.DeclareFun("return", [](int a) -> int { return a; });
    ctx.DeclareFun("return", [](std::string a) { return a; });
    // And so on...
    ```

    You see that you can overload any function or operator. Overload resolution
    will be done.

5. Have your script coming in somewhere.

    ```c++
    std::string script = "(+ 1 (+ 2 3))";
    ```

6. Parse it.

    ```c++
    auto res = ParseSlip(script);
    if (!res) {
        throw std::runtime_error("Parsing failed");
    }
    ```

7. Type check it and resolve overloads.

    ```c++
    TypeCheck(*res->first, ctx);
    ```

8. Run it and profit. Depending on the result you expect, change the
   template parameter accordingly.

    ```c++
    auto eval = Eval<int>(*res->first, ctx);
    ```


#BUT HOW DO I...?

* "Create a function?" Damn, you don't! You want something, you fuckin pay for
  it! That's free man, do you think it would be free if it were useful man?
* "Have my types handled?" ... You dumb man?
