# NtLua
Do you have a burning desire to run coroutines in kernel?

Do you hate the fact that Microsoft does not consider Admin-Kernel boundary real when they do not feel like paying security researchers and yet refuse to give you Ring-0 access?

Did your parents explicitly tell you to stay away from kernel?

Do you get a smile on your face when you imagine device driver developers seeing this and crying over how stupid it is for hours (although half of the hardware manufacturers ship drivers with arbitrary physical memory read/write somehow)?

Well you've come to the right place to run a scripting language at `DPC_LEVEL`!

# How to use
1) Build everything using the solution file.
2) Create the NtLua service via `sc create NtLua binpath= <Path-to-driver> type= kernel`
3) Start the NtLua service `sc start NtLua`
4) Run the console and enjoy!

# Horrible samples for horrible people
![](https://i.can.ac/vq7g1.png)
![](https://i.can.ac/OKncG.png)
![](https://i.can.ac/PmiNI.png)
![](https://i.can.ac/K6Da1.png)
![](https://i.can.ac/lvDuN.png)
![](https://i.can.ac/lD1bF.png)

## Supports:
- Structured exception handling
- Garbage collected temporaries
- UNICODE_STRING/ANSI_STRING via `unicode_string(str) and ansi_string(str)`
- Automatic importing of entire kernel images

## Under development:
- File importing via UM console
- Symbol parsing for internal functions and structure declarations
- Lua to C callback wrapping
- Multi-thread support
- HIGH_LEVEL IRQL support
- Other fun stuff you are considering to contribute.

----------
## If you have any useful scripts, feel free to send a PR to include it in the Repo under /scripts!
