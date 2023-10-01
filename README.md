# ArmA9Galaga
Galaga game in C for the Arm A9 processor on the DE1-SoC board.
Meant to be played with a PS/2 keyboard.
Developed by Christian Canelea and Sebastian Sergnese

Using the left and right arrow keys on a PS/2 to move your spaceship at the bottom of the screen while using the spacebar to shoot. You can have up to 2 projectiles on screen at a time. With these basic controls, shoot all the enemies in a stage to clear it and move on to the next, while dodging enemy attacks and bullets. The game will display your high score, current score, current stage, and current lives on the right, and the goal is to go as far as you can and get the highest score. Press any key to transition a screen (Title screen, stage # screen, game over screen).


Each stage features the same number and types of enemies that will appear from the top at the start. These include 4 Bosses at the top worth 200 points, taking 2 hits to kill, 16 Goeis below them worth 100 points each, taking 1 hit to kill, and 20 Zakos below them worth 50 points each, taking 1 hit to kill. Each enemy will randomly shoot at you, with the odds increasing as the stage number rises. Each enemy may also dive toward the player. If either the enemy or their bullet hits you, you will lose one of your three lives, with the game ending when all the player's lives are lost. The game over screen will show player stats such as shots fired, shots hit, and the hit-miss ratio.

Some debug/cheat codes (only while playing a stage)
- KEY3 ends the game (brings you to game over screen)
- KEY2 removes a life
- KEY1 sets lives to 99 (only ones digit is displayed)
- KEY0 clears the stage (sends you to the next one)

# Gameplay Demonstration
https://youtu.be/R7KWRVVmEOE
