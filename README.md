# ConstDisplay
A windows program to maintain a constant brightness level for your desktop

![image](https://github.com/user-attachments/assets/512f9992-a45f-40db-a27f-4e6e88f0b1cd)

### Settings:
> Target brightness: The program will attempt to bring the overall brightness of the screen to this specified value. Goes without saying but the lower it's set, the darker it'll be.

> Brightness delta tolerance: The limiting difference between the target brightness and screen's calculated brightness. Once this limit is reached, the program stops modulating the brightness.

> Brightness check interval: The amount of time to wait between consecutive brightness checks and thus modulation.

> Brightness change step size: The increment step to use when modulating brightness. It's actually changing the overlay window's opacity behind the scenes.

> Overlay colors: The color for the overlay window. Pure black (RGB of 0,0,0) recommended for most cases.
