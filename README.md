# Ultimate Tester!

Pocket Sized Component Tester + Variable Power Source + Accurate Power Monitor in Arduino Uno Form Factor

Full Blog Entry & Details:
https://pk17r.wordpress.com/2023/12/18/ultimate-tester-component-tester-lipo-charge-boost-protect/

Updated Ultimate Tester Schematics: https://github.com/pk17r/Ultimate_Tester/blob/prash/ComponentTester-1.54m/UltimateTester_PCB/UltimateTester_v1_with_mods.pdf

<br>

![Alt text](Ultimate_Tester_PCBs/UltimateTester_v1_Pics/20250103_055959.jpg?raw=true "Component Tester + Power Supply and Power Monitor")

![Alt text](Ultimate_Tester_PCBs/UltimateTester_v1_Pics/2.JPG?raw=true "Component Tester")

![Alt text](Ultimate_Tester_PCBs/UltimateTester_v1_Pics/1.png?raw=true "LiPo Charge Boost Protect / USB-C Power Supply")

![Alt text](Ultimate_Tester_PCBs/UltimateTester_v1_Pics/20250103_011049.jpg?raw=true "Accurate INA226 Power Monitor with External Power Supply")

![Alt text](Ultimate_Tester_PCBs/UltimateTester_v1_Pics/20250103_005226.jpg?raw=true "Accurate INA226 Power Monitor with Internal Power Supply")

![Alt text](Ultimate_Tester_PCBs/UltimateTester_v1_Pics/20250103_011416.jpg?raw=true "Accurate INA226 Power Monitor along with Voltmeter")

Current Accuracy:<br>
Test 1: Low Current: 2.4mA<br>
<table>
  <tr>
    <th></th>
    <th>Measured</th>
    <th>Ground Truth</th>
    <th>Error</th>
  </tr>
  <tr>
    <td>Current</td>
    <td>2.4mA</td>
    <td>2.39+0.6%=2.40mA</td>
    <td>0%</td>
  </tr>
  <tr>
    <td>Voltage</td>
    <td>5.17V</td>
    <td>5.164+0.1%=5.17V</td>
    <td>0%</td>
  </tr>
</table>

![Alt text](Ultimate_Tester_PCBs/UltimateTester_v1_Pics/20250106_160324.jpg?raw=true)

Test 2: Medium Current: 209.3mA<br>
<table>
  <tr>
    <th></th>
    <th>Measured</th>
    <th>Ground Truth</th>
    <th>Error</th>
  </tr>
  <tr>
    <td>Current</td>
    <td>209.3mA</td>
    <td>207.3+0.6%=208.5mA</td>
    <td>+0.4%</td>
  </tr>
  <tr>
    <td>Voltage</td>
    <td>5.14V</td>
    <td>5.135+0.1%=5.14V</td>
    <td>0%</td>
  </tr>
</table>

![Alt text](Ultimate_Tester_PCBs/UltimateTester_v1_Pics/20250106_160627.jpg?raw=true)

Test 3: High Current: 1.07A<br>
<table>
  <tr>
    <th></th>
    <th>Measured</th>
    <th>Ground Truth</th>
    <th>Error</th>
  </tr>
  <tr><td>Current</td><td>1.07mA</td><td>1.056+0.6%=1.06A</td><td>+0.9%</td></tr>
  <tr><td>Voltage</td><td>4.95V </td><td>4.928+0.1%=4.93V</td><td>+0.4%</td></tr>
</table>

![Alt text](Ultimate_Tester_PCBs/UltimateTester_v1_Pics/20250106_161220.jpg?raw=true)

Test 4: Different Voltages<br>
<table>
  <tr>
    <th>Measured</th>
    <th>Ground Truth</th>
    <th>Error</th>
  </tr>
  <tr><td>3.93V</td><td>  3.930+0.1%=3.93V</td><td>    0%</td></tr>
  <tr><td>9.05V</td><td>  9.051+0.1%=9.02V</td><td>   +0.3%</td></tr>
  <tr><td>12.04V</td><td>12.022+0.1%=12.03V</td><td>+0.1%</td></tr>
  <tr><td>15.15V</td><td>15.136+0.1%=15.15V</td><td>0%</td></tr>
</table>

![Alt text](Ultimate_Tester_PCBs/UltimateTester_v1_Pics/20250106_180711.jpg?raw=true)

<br>

# Transistortester Warehouse

The Transistortester is a device for identifying and checking electronic components.
It also offers additional functionality like a PWM signal generator for example.
And it's OSHW.

This repo is meant to provide a convenient way to download firmware packages,
documentation files, PCB files and more. Therefore it's structured differently
from the main repo.

The project's webpage is https://www.mikrocontroller.net/articles/AVR_Transistortester
and the main repo is https://github.com/kubi48/TransistorTester-source.

BTW, all the inexpensive Transistortesters sold on your favourite shopping platform
are based on this project and can be updated with the original OSHW firmware versions.
