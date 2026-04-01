# Stepper motor driver using a pic18f16q20
Currently a work in progress.

This project implements a DMA-driven stepper motor controller that writes directly to the IO port to manage step and direction signals. A hardware timer triggers the DMA transfer, generating interrupt triggers for the DMA that push the next port mask into the PORTx register with minimal cpu intervention.
When the DMA completes a buffer, it raises an interrupt and swaps the DMA descriptors, containing the next buffer. The previous buffer gets set as fill, and a event is posted to handle the refill of the buffer.
The system uses an event-driven architecture to offload the step pulse generation from the CPU. While one buffer is active, the other is being prepared by the motion control task. This frees up time for the CPU to handle motion planning and other required tasks without being forced into a ISR on every tick.


## ruff overview of system
<img width="1646" height="1153" alt="Untitled Diagram(9) drawio" src="https://github.com/user-attachments/assets/d8f4d72b-6cba-43cd-be2c-5540c169cdc8" />
