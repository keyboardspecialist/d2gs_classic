Complete D2GS 1.09d source code that was taken from developer's web site [d2dev.dlg.cn](http://d2dev.dlg.cn)

## Modern Windows build

The supported target is 32-bit Windows. The bundled `d2server.dll` and
`d2server.lib` are x86 binaries and define the game-engine ABI, so an x64
server cannot be produced without a replacement engine library.

Requirements:

- Visual Studio 2022 with the Desktop development with C++ workload
- A Windows 10 or Windows 11 SDK

Build from a Developer PowerShell prompt:

```powershell
msbuild D2GS.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
```

Outputs are written to `build\Win32\Debug` or `build\Win32\Release`.
`d2server.dll` is copied there automatically. The output copy has its `.data`
section marked executable because the legacy DLL stores its entry point and
implementation there; modern Windows DEP otherwise terminates it before
`main`. The original binary under `include\d2gelib` remains unchanged.

## Runtime setup

D2GS must run from a directory containing the matching Diablo II 1.09d
server runtime. These copyrighted game files are not included:

- `Patch_d2.mpq`, `D2Data.mpq`, `D2Sfx.mpq`, `D2Speech.mpq`, and `D2Exp.mpq`
- `D2Win.dll`, `D2Game.dll`, `D2Client.dll`, and `D2Common.dll`
- `D2Net.dll`, `Fog.dll`, `Storm.dll`, `D2Lang.dll`, and `D2Cmp.dll`
- `D2Sound.dll`, `D2Gfx.dll`, and `d2gs.script` when required by the engine configuration

Copy the matching files into the selected output directory. Do not mix DLLs
from different Diablo II patches; the engine uses version-specific ordinals
and internal addresses.

`D2Exp.mpq` is optional for runtime validation, and expansion game creation
requests are rejected when it is absent. The bundled `d2server.dll` targets
LOD 1.09d, so the tested classic 1.09 DLL set uses a hash-gated adapter for two
known startup incompatibilities. Classic startup remains opt-in with
`D2GS_EXPERIMENTAL_CLASSIC_109=1` until client join and character persistence
have been tested; the incompatible 1.09d GE patch set is forcibly disabled in
that mode. D2CS/D2DBS registration and empty classic game creation/automatic
shutdown have been verified with the tested runtime.

See `docs\classic-1.09-adapter.md` for the current reverse-engineering map.

Import `config.example.reg` for a per-user configuration that does not require
administrator privileges, then update the D2CS/D2DBS addresses and admin
password:

```powershell
reg import config.example.reg
```

Configuration is read first from `HKCU\Software\D2Server\D2GS`, then from the
legacy `HKLM\Software\D2Server\D2GS` location. D2CS and D2DBS must be running
and reachable on their configured ports before the realm can become active.

## Version boundary

Windows/toolchain compatibility is kept separate from the 1.09 engine ABI in
`include\d2gelib\d2server.h`. Future game-version backports should provide a
version-specific engine adapter and runtime set rather than adding patch
conditionals throughout the realm/network code.


# About the Diablo2 Close Battle.net Server

As you can see, I and my partners made a Battle.net server with Diablo2 close game support last year. I will try to explain how it works and what is the problem with it. 

First, I will show why and how I made that server. 

Last year, after Diablo2 comes out, I know that game from friends and was soon interested in it. Soon, I am tired with single player game, and so changes to play multiplayer game on our local lan. in these days, I heard of a software named FSGS can help to create a battle.net server for Diablo2, that makes multiplayer game more easy and interesting. So I setup a server with FSGS on lan, and from FSGS's web page, I know that they are developing with the Diablo2 close server. it will be exciting if there are close mode support for this game, for our net limit, we can not go abord directly. And it will be very lag to play it on official battle.net through proxy. So I waited and hope FSGS can make it out soon. 

After a long time wait, no good news from FSGS, and it keeps saying -- it will be out, wait and see... In these days, I found that FSGS is spawned from BNETD, which also can make a battle.net server. And the most important, BNETD is open source. So I decided try to make a own server with Diablo2 close support by modifying current BNETD code. 

I downloaded the source and compiled it, start the server. all works fine. so I changed all current FSGS server user account to BNETD format. change server to BNETD to test. in the test, I found BNETD is stable and begin to write own code. 

I finished the battle.net part support for Diablo2 in short time, for BNETD source code is easy to understand and add new future. I should thanks all BNETD developer here, their hard work saved me lots of time. Then I began with the game server part. 

As we know, the most important thing in Diablo2 close support is game server, unlike previous games from Blizzard, in close mode, the game server is in server instead of client side. The client just send player action to server, and server should reply the result in correct format. Blizzard do not make their game server to public, so it will be hard to program a game server in your own code. In this case, you should guess out all client/server communication protocol and then make program a Diablo2 game without client GUI interface. I decided to use another way. 

You can find out easily that in tcp/ip mode games, the host just act as a game server. So the way i choose is using the tcp/ip game host as game server. The battle.net server listen at game server port, receive client request, do some necessary parse, get the imformation, do some change when needed, pass the request to a tcp/ip game host. The game host will respones to the request, the battle.net server will receive the respones, do some parse again here. Then pass it to the client, just works like a proxy or tunnel. There is a tool called bnproxy in BNETD utils, so I merged bnproxy with BNETD, bnproxy act as tunnel to game server. BNETD for battle.net server as before, and after lots of failure and try, I find that works as I image at first. The close game server use a nearly same protocol with open game host. This is the initial model of the server. 

At that time, the game server only supports one game as tcp/ip host, that sounds very bad, isn`t it? But soon, I found out some hidden game protocol that can create more than one game in one host, and with modifying some paramters in the request, the game server can act much like official game server.

At the end of last year, after some weeks debug and test, the server based on this model becomes much stable than begining, so i put it out. In these days, some friends joined the development, and also lots of friends help to test and maintain the server. I should say great thanks to them. 

But still, there are lots of problems and bugs in the server based on this model. and lots of code are from scratch. so I and my partners planed to change current server architecture. but with time limit and some other reason, it goes on slowly and seems stopped. 

And currently, I plans to combine my old project into BNETD project, that will save time and avoid repeat work, hope we can make it better together. 

Comments and suggestions are welcomed to onlyer@263.net BTW: the old 1.03 server code is a tempory solution, and nearly useless now, no need to waste time to read it.

***onlyer<br> 
2001-08-13***
