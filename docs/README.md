# capture-point

States:
- Ready
- Stand By
- Red Countdown
- Red Won
- Blue Countdown
- Blue Won

Guidelines
- After a transition is executed, all read buttons and their counters should be cleared with clearReadInputs()

To Do
- Fix initial flashing in Stand By mode.
- Fix message overlap in Stand By Mode. Sometimes "Comenzar" is still displayed.
- Fix Blue re-capturing. When all progress blocks are displayed and button is released right before the last moments of completing the recapture, the system seems frozen.
- (Seems fixed) Critical bug: Blue Capture, Red recapture, then BLUE IS NOT ABLE TO RECAPTURE
- (Seems fixed) Test extensively: some times the recapture is not working properly.
	- Red recapturing gest stuck on the 15 char. Sometimes doesn't even start recapturing.
	- Blue seems ok. Blue contact might throw off false HIGHs

Improvements
- Turn off backlight if nothing is happening (no state change, no button pressed)
- Remove all delays
- Remove all objects using 'new' as it may cause memory leaks because it needs to be removed manually
- Hardcode Mode: Stand by lasts 10 minutes. If nobody captures, everybody loses
- External LEDs for indicating: who captured the point, capturing, game over
- External siren for indicating: capture changed, game end

Future
- Game Menu that allows you to select each developed gamemode (from SD card?)
