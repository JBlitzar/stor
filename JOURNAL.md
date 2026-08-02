---
title: "USB-C MicroSD Flash Drive"
author: "JBlitzar"
description: "High-speed USB-C microSD flash drive"
created_at: "2026-07-28"
---

# July 28: Developed the flash drive concept

Basically came up with the idea and assessed feasability. After thinking about my previous hardware projects, I thought a flash drive would be super cool. Why?

 - I love USB. Doing HS would still be USB but would be slightly levelled-up. usb3.0 in general like requires like NDA or FPGA or custom ASIC, so I thought I would go one step up instead of two for now.
 - I realized that eMMC is super expensive these days! shoutout to the [2025–present global memory supply shortage](https://en.wikipedia.org/wiki/2025%E2%80%93present_global_memory_supply_shortage)
, I guess. Sort of affects all memory-related ICs. It's not as bad as $80 for 2GB of LPDDR4 chip ram though. Still, decently expensive. I decided for this project I wanted to actually store files of a meaningful size.
 - The solution? [MicroSD cards](https://xkcd.com/691/)!! They have great data density (the highest-end ones can store up to 2TB these days), and are pretty cheap, especially if you source them off of the OEM supply chain like LCSC rather than consumer marketplaces like Amazon. I found a deal for 128GB for just $10 (??! probably trash data durability or a scam or both, but anyways I found plenty of other cheap listings that aren't too-good-to-be-true cheap). Plus, hotswappable storage is very hacker aesthetic. 
 - The good news is that USB3.0 isn't even worth chasing anymore, since we'll be able to saturate the SD card link speed itself. The link speeds actually don't bottleneck each other too much either way. With UHS-I enabled microSDs we're looking at 50 megabytes per second, and maybe 30-40 without. Pretty good.

 ![](doc/sdlisting.png)

 yeah uh otherwise not much more to show, just research and concept

Time spent: 0.5 hours

# July 29: Selected the microcontroller and USB PHY

Did some research, locked in some more precise specs. 

![](doc/r2.png)

Time spent: 0.5 hours

# July 29: Researched and completed the schematic

STM chip is the chip for the job. So unfortuantely I will have to depart from my goat espressif for this project. There do exist STM chips that have built in USB2.0 HS PHY, but they are more expensive (the lcsc listing I found was low stock AND $28 per, which is crazy considering the rk3566 costs just $14).

In the end, STM32H723VGT6 plus usb3300 usb phy is the way to go. The STM *does* have built-in sd card phy which is nice, and the clock speed is more than enough to saturate USB2.0 link speed. Later, I could do something cool, like on-the-fly encryption or something

Time elapsed taken from gh commit timestamps, I was working pretty much continuously.

Had to find actual resources. The STM datasheet wasn't too helpful, and the [DISCO schematic](st.com/resource/en/schematic_pack/mb1248-h747i-d03-schematic.pdf
) was... pretty intimidating, because it has examples for *every* peripheral. I went off of a [reddit post](https://www.reddit.com/r/PCB/comments/1n8w67u/stm32h723vet6_breakout_board_feedback_advice_for/
) for hardware/strapping, and hopefully I can just expose some GPIOs, flash over SWD, and pray.

![](doc/reddit_schem.png)

Then I just researched which pins to connect up for the sd card and found a proper example schematic for the USB PHY (although they put ferrite beads everywhere which I omitted).



![](doc/usb_phy_schem.png)

TPS62177DQCR had a perfectly acceptable example schematic

![](doc/tps_schem_2.png)

Then I put it all together! Had to use an A3 sheet because so big lol 

![](doc/schem1.png)

The STM pins were unfortunately not... charitably labelled. I used a lot of cmd+f.

Also some confusion between VDD/VDDA and GND/GNDA. Since I'm not doing any analog stuff, I merged them. Basically, the reddit PSU situation was way overengineered. That's why I went with the TPS.

So I guess research and schematic both took a surprising amount of time here.

Time spent: 4 hours

# July 29: Placed and routed the PCB

Placement and routing !!

You know what they say, good placement is 90% of good routing. What's cool is that I was pretty much able to fit everything in a standard thumb-drive footprint. Well, a slightly chunky thumb drive. Thumb drives are usually 18x60mm ish with casing, mine stands at 20mm wide without casing. Still acceptable. 

this is my first four-layer PCB, although honestly it feels like a two-layer pcb because I'm trying to keep the gnd and power planes solid, so there are really only two signal planes, just like normal. For the two most common traces though, gnd and vcc, all you have to do is via down and you tap the plane. GND plane important for usb2.0 diff pair, and four-layer esp important because it means the layers are thinner so impedance matching works out.

I'm not here to talk about impedance matching though, I just did placement. Here are some progress pics:

![](doc/place1.png)

![](doc/place2.png)

![](doc/route1.png)

Then I realized I had been putting off the differential pair when I really should have done it first. And really, my whole placement in that upper section was no good because of the rotation. I ended up completely refactoring the routing by changing the placement of the USB PHY ic. Which mean that the XO/XI caps plus the crystal had to all be re-placed.

![](doc/ref1.png)

Then I just did the routing for the USB PHY and the sd card. Which is like sixteen bus lines, all interwoven.

![](doc/crazy1.png)

![](doc/crazy2.png)

![](doc/crazy3.png)

Time spent: 2.5 hours

# July 30: Refined the routing and USB-C connector

Fussed with the routing a little bit. Managed to get some of the SD bus traces slightly further from the edge, which is probably good for case considerations even if solder mask exists. push and shove routing really is my goat. EasyEDA could never compete. (speaking of which, the newest kicad nightly update releases constraint-driven geometry! so cool! But I digress.)

Yeah, the routing looks quite... intricate ATP

![](doc/intricate.png)

I also swapped out my THT USBC male for a different one that's less sketch and not weird and double-sided. This one will be more secure. It also had the escape routing I needed for the D+ and D- diff pairs to line up on the right side. 

Time spent: 1.0 hours

# July 31: Documented the design and added mounting holes

I realized that I hadn't been writing journals, so I wrote journals.

oh also I realized I haven't shown a 3d render, here it is!

![](doc/rend.png)

The front is pretty packed with components. I think it looks cool.

I also uh drew my own SD logo in jspaint because I thought it'd be funny. I didn't pay the $1500 SD™ licensing fee. Front was pretty busy though so I guess it could only go on the back. It's a bit weird for that to be the only thing on the back, so I guess I have a few options:

- remove it
- add other (serious) silkscreen art
- add other spoof regulatory markings lol (FCC, CE, ROHS)

Anyways this was mostly just a whimsical thing I did.

I also added mounting holes!! had to shift the microsd holder a bit to the right and redo those traces but otherwise yeah. They are m3 holes. 

![](doc/holes.png)

TODOs:

- double check the premise of the schematic more closely (You shouldn't always trust reddit with these things. Last time I compared a hackaday tutorial to the datasheet on other stuff, it deviated in some really meaningful ways)
- Write firmware (non-trivial!)
- Get a quote + BOM optimize
- Docs, good README, flashing instructions
- potentially a beauty render

Time spent: 1.0 hours


# August 1: Routing hell

I posted my routing images around because I thought they looked cool, but I actually got free feedback!

Eliminated via-in-pad, re-matched my crystal caps, added some decoupling/bypass, and connected some pins that I needed to that I forgot 💀. Also used a different esd diode. 

I tried to be extra generous because apparently easyeda2kicad scams you and gives you bad courtyards. In eliminating via-in-pad, I finally bit the bullet and put some drunk components. 

![](doc/drunk.png)

Time spent: 2.0 hours


# August 1: Firmware

Used TinyUSB. I'll be honest, I used a bit of AI. Mostly firmware is uninteresting here, some config knobs to change but mostly this is piping vendored tinyusb to vendored HAL_SD.

Also added a red LED and a white LED! Fun stuff. 

![](doc/leds.png)

The PR was comically large because of vendored TinyUSB being included (had to edit library.json), it's actually about 500 SLOC, and, like I said, it's mostly config / plumbing.

![](doc/pr.png)

Also dropped in my FPGA flashing docs. 


Time spent: 1.0 hours
