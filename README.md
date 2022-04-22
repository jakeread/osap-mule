## OSAP Mule Projects

This is my stash of demo and test projects for [OSAP](http://osap.tools). 

To build and run a JavaScript example, see [/js]. Embedded examples are in [/firmware]. 

This repo contains many instances of the following [submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules):

[osapjs](https://github.com/jakeread/osapjs): javascript codes  
[osape](https://github.com/jakeread/osape/): embedded codes  
[osape_arduino](https://github.com/jakeread/osape_arduino): arduino-specific link layers  

They should all populate when you do a `git pull` on this repository. I'm working up to a proper `NPM` package and `Arduino` library, but I am so often editing submodule source code that package managers are too heavy handed at the moment. Also, who doesn't love learning new git tricks? Submodules are great *so long* as you don't nest them. 