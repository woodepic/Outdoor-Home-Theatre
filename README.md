# Outdoor-Home-Theatre

This is a re-upload of an old project, as I did not use Github at the time. This project spanned July 2018 - March 2020, and I am happy to report it is still working perfectly to this day (January 2022). 

I created an enclosure for a projector, sound equiptment, and various other A/V electronics which resides outdoors all year round. Where I live, outdoor temperatures can span -40*c to + 40*c, so creating a climate controlled system robust and powerful enough for such a climate is a challenging task. 

The enclosure utilizes a 5000BTU air conditioner, high speed fans, and a 250w heater. Using an arduino and various temperature sensors, it monitors and accounts for changes in temperature to keep the system running no matter the climate.


Here is a link to a video demo of V3.1.
<https://youtu.be/Ghoxmd-oyHQ>
I should mention that the enclosure is no longer using a PID control loop, as that is not sutible for a binary on/off cooling and heating system.
Also, the servo control is now written in non-blocking code.
Later revisions incorperate much more through testing, various bug fixes and validation logs.
