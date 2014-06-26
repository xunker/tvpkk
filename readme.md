## TVKPP (Trinket Volume Knob Plus Plus)

#### Build a Griffin Powermate clone using $10 in parts.

This is a modification of the Adafruit Trinket Volume Knob tutorial, changed
so the knob can do more than just change the volume. It uses the same wiring
as the TrinketVolumeKnobPlus example.

### IMPORTANT NOTE:

As of June 26 2014, `TrinketHidCombo.h` from Adafruit does not include
support for mouse wheel events. I have forked the code and added support:
https://github.com/xunker/Adafruit-Trinket-USB

Hopefully my pull request will be accepted soon and the changes will merge
in to the Adafruit master. :)

#### Description

This version allows you to change the commands being sent by "long pressing"
the knob for 1 second or more. A press of less than one second will execute
the normal button-press command.

To change the commands, edit `struct control_set control_sets[] = { ... }`.

By default, the command sets are:

| ROTATE LEFT           | ROTATE RIGHT          | PRESS BUTTON             |
|-----------------------|-----------------------|--------------------------|
| mouse wheel scroll up | mouse wheel scroll dn | home key (scroll to top) |
| volume down key       | volume down key       | mute key                 |
| previous track key    | next track key        | play/pause key           |
| page up key           | page down key         | home key                 |
| up arrow key          | down arrow key        | home key                 |

Original tutorial found at http://learn.adafruit.com/trinket-usb-volume-knob
