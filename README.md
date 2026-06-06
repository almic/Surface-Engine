# Surface

Real. Free.

Please read `LICENSE` before proceeding any further!


## Setup

Get rust


## Structure

The Surface Engine is located in the `surface/` directory, and contains all the major functionality of the editor. The terms 'surface', 'engine', and 'editor' all mean the same thing.

The core libraries of the engine are located in the `core/` directory. These are structured as independent libraries, meaning you can depend on them individually in your own projects. Seriously, they are all **standalone** Rust crates. They have **few to zero dependencies**. If you want one, add the prefix `surface-` to the folder name to obtain the public crate name. For example, the name of the crate at `core/json/` would be `surface-json`.


## Goals

### Develop the best open-source application/ game engine and editor that competes with existing industry software

[Relevant XKCD](https://xkcd.com/927/). Open source is the inevitable future of game development software. Competition is prerequisite to innovation. With that in mind, best to start early.


### Provide well-maintained, high-quality, lean and standalone libraries to the world

A foundational goal of Surface is to provide high-quality, **standalone** libraries to the world. You can be certain that when you add a new Surface dependency, you will get a small download with only the code you needed. Real effort is made to avoid [anti-patterns](https://en.wikipedia.org/wiki/Anti-pattern) that plague other open-source library suites.


### Positively impact global software development by setting an example of what good fundamentals should be, and what can be achieved

People say that maturity is a prerequisite for good software, that only decades of work by hundreds of people create stability. I say this is a pessimistic and unfounded view of the world when there are many examples of solo developers and small teams creating incredible work in just months or weeks. The Surface Engine will continue to be proof that good fundamentals and good thinking are sufficient to meet, *and exceed*, the quality and stability of existing software.


## Contributions

You will notice that Issues are currently disabled, and only invited collaborators can make Pull Requests. This was done at the start of the project to get ahead of spam and low-quality contributions, particularly from LLM agents.

Regardless of my concerns about spam, the project is currently too underdeveloped anyway. Because there's very little source code for anybody to follow, and all those "fundamentals" I mentioned haven't been expressed yet, I would almost certainly reject any contribution.

Your contributions will have to wait until this project has a bit more code, and I've made the grand design a bit more clear.

<!-- The following portion is deliberately commented out
I will eventually provide ways to directly contribute to Surface, but if you are really eager to help now, add me on Discord or send an email. It shouldn't be hard for you to figure it out. If you do contact me, *be serious about it*: tell me right away why you are messaging me and what your contribution is.

If you think you're ready for the big leagues, fork the repository and create a branch with your contributions. I check them regularly and will contact you directly if I think your code is wonderful.
-->
