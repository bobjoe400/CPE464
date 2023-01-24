# Cooper Mattern 3pm lab
The main idea for this program was to use pointer arithmatic to step my way through a packet.
Using this allows for simplified byte grabbing by just either memcpy'ing the bytes that I'm 
at or just de-referencing the pointer to get the byte. This also allowed me to use postcrement
to access the value then increment the pointer in the line where I'm accessing it. I use this 
throughout the program. 

The utility file contains short functions that do not belong in the main program. 