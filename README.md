<h1>WiFi-Enable any Project with ESP32 and ESP-IDF</h1>
<p align="center">
<img src = "https://user-images.githubusercontent.com/9260214/28747595-19a41090-7471-11e7-826c-42c28ea7ae6e.jpeg" width = "500" height = "300"></p>
<h2>Description:</h2>
<p>
This project provides a comprehensive guide on how to enable WiFi connectivity for any project using the ESP32 microcontroller and the esp-idf (Espressif IoT Development Framework). The ESP32 is a powerful and versatile microcontroller that supports WiFi and Bluetooth connectivity, making it an excellent choice for Internet of Things (IoT) projects, home automation, sensor monitoring, and more.
</p>
<p>
By following the instructions in this repository, you can quickly integrate WiFi capabilities into your projects, allowing them to communicate with other devices, cloud services, and the internet.
</p>

<h2>Features</h2>
<ul>
<li>Easy integration of WiFi connectivity into any project.</li>
<li>Utilizes the popular ESP32 microcontroller.</li>
<li>Step-by-step guide for setting up and configuring the project.</li>
<li>Example code for common WiFi communication scenarios.</li>
<li>Robust and reliable esp-idf framework for efficient development.</li>
</ul>

<h2>Prerequisites</h2>
<p>Before getting started with this project, make sure you have the following hardware and software requirements:</p>

<h3>Hardware</h3>
<ul>
<li>ESP32 development board (e.g., ESP-WROOM-32, ESP32-DevKitC, etc.)</li>
<li>Micro-USB cable for programming and power supply</li>
<li>Breadboard and jumper wires (for prototyping, if needed)</li>
</ul>

<h3>Software</h3>
<ul>
<li>ESP-IDF installed on your development machine.</li>
<li>A compatible Integrated Development Environment (IDE) for ESP-IDF, such as Visual Studio Code with PlatformIO or Eclipse IDE with the ESP-IDF extension.</li>
</ul>

<h2>Getting Started</h2>
<p>Follow these steps to set up the project and start adding WiFi capabilities to your project:</p>
    <ol>
        <li>Clone or download this GitHub repository to your local development machine.</li>
        <pre><code>git clone https://github.com/Abhishek-2310/ESP32-WiFi-ENABLE.git</code></pre>
        <li>Connect your ESP32 development board to your computer using the Micro-USB cable.</li>
        <li>Open the project folder in your preferred IDE for ESP-IDF.</li>
        <li>Configure the project's WiFi settings in the <code>main/config.h</code> file. Update the SSID and password to
            match your WiFi network credentials.</li>
        <pre><code>#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"</code></pre>
        <li>Compile and flash the project to your ESP32 development board.</li>
        <li>Monitor the serial output to check the WiFi connection status and any debug messages.</li>
        <li>Once connected to the WiFi network, explore the example code in the <code>main</code> folder to learn how to
            send and receive data over WiFi.</li>
    </ol>
    
<h2>Debugging</h2>
<p align="center">
<img src = "https://github.com/Abhishek-2310/ESP32-WiFi-ENABLE/blob/master/JTAG_connection_ESP32.png" width = "750" height = "450"></p>
ESP PROG to target JTAG connections:
<ul>
<li>3v3          -> 3v3</li>
<li>GND          -> GND</li>
<li>Orange (TMS) -> GPIO 14</li>
<li>Yellow (TCK) -> GPIO 13</li>
<li>Blue (TDO)   -> GPIO 15</li>
<li>Green (TDI)  -> GPIO 12</li>
</ul>
    
Make sure to source the ESP32 source file made.

Run the openOCD command in one terminal:
```sh
openocd -f board/esp32-wrover-kit-3.3v.cfg     # Both target and interface are included in board
```
upload the code to target via JTAG:
```sh
openocd -f board/esp32-wrover-kit-3.3v.cfg -c "program_esp build/"project-name".bin 0x10000 verify exit reset"
```
Run the GDB command for xtensa core:
```sh
xtensa-esp32-elf-gdb -x gdbinit build/"project-name".elf
```
<h2>Contributing</h2>
<p>Contributions to this project are welcome! If you find any issues or want to add new features, please submit a pull request. For major changes, please open an issue first to discuss the proposed changes.</p>

<h2>License</h2>
<p>This project is licensed under the <a href="LICENSE">MIT License</a>.</p>
<h2>Acknowledgments</h2>
<p>Thanks to the open-source community for providing valuable resources and tools that made this project
        possible.</p>
