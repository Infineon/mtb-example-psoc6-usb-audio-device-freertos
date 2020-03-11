# PSoC 6 MCU: USB Audio Device with FreeRTOS

This example demonstrates how to use PSoC 6 MCU to implement a USB Audio Device and HID Audio Playback Control that connects to the PC via the USB interface. The example also uses FreeRTOS.

## Overview

This project shows how to use the PSoC 6 MCU to enumerate a Standard USB Audio Device and HID Consumer Control over the USB interface. The PC host sees the device as:
- Audio Playback device
- Microphone/Recording device
- Remote Control handling volume, play/pause/stop

The PSoC 6 MCU device works as a bridge between the audio data streamed from the USB host and the I2S block, which connects to an audio codec. The audio codec outputs the audio data to a speaker or headphone. The same audio codec can input microphone data and stream to the PSoC 6 MCU device, which bridges to the USB interface.

CapSense buttons are used to play/pause/stop and change the volume of the data streamed to the PSoC 6 MCU device. Any presses on CapSense buttons are reported back to the host over USB HID Consumer Control.

[Figure 1](#figure-1-block-diagram) shows the high-level block diagram of this application.

##### Figure 1. Block Diagram

![BlockDiagram](images/BlockDiagram.png)

## Requirements

- [ModusToolbox™ IDE](https://www.cypress.com/products/modustoolbox-software-environment) v2.0
- Programming Language: C
- Associated Parts: All [PSoC® 6 MCU](http://www.cypress.com/PSoC6) parts with USB

## Supported Kits

- [PSoC 6 Wi-Fi BT Prototyping Kit](https://www.cypress.com/CY8CPROTO-062-4343W) (CY8CPROTO-062-4343W)
- [PSoC 6 WiFi-BT Pioneer Kit](https://www.cypress.com/CY8CKIT-062-WiFi-BT) (CY8CKIT-062-WiFi-BT) - Default target
- [PSoC 62S2 WiFi-BT Pioneer Kit](https://www.cypress.com/CY8CKIT-062S2-43012) (CY8CKIT-062S2-43012)

## Hardware Setup

If using CY8CKIT-062-WiFi-BT or CY8CKIT-062S2-43012, it requires the CY8CKIT-028-TFT to be connected to the board. If using CY8CPROTO-062-4343W, it requires a third-party module - [Pmod I2S2: Stereo Audio Input and Output](https://store.digilentinc.com/pmod-i2s2-stereo-audio-input-and-output/). This module should be soldered to the PMOD header J15 on CY8CPROTO-062-4343W.

**Note**: The PSoC 6 BLE Pioneer Kit and the PSoC 6 WiFi-BT Pioneer Kit ship with KitProg2 installed. ModusToolbox software requires KitProg3. Before using this code example, make sure that the board is upgraded to KitProg3. The tool and instructions are available in the [Firmware Loader](https://github.com/cypresssemiconductorco/Firmware-loader) GitHub repository. If you do not upgrade, you will see an error like "unable to find CMSIS-DAP device" or "KitProg firmware is out of date".

## Software Setup

This example uses the [Audacity](https://www.audacityteam.org/) tool to record and play sound. You can also use any software tool that plays music.

## Using the Code Example

### In ModusToolbox IDE:

1. Click the **New Application** link in the Quick Panel (or, use **File > New > ModusToolbox IDE Application**).

2. Pick a kit supported by the code example from the list shown in the **IDE Application** dialog.

   When you select a supported kit, the example is reconfigured automatically to work with the kit. To work with a different supported kit later, use the **Library Manager** to choose the BSP for the supported kit. You can use the Library Manager to select or update the BSP and firmware libraries used in this application. To access the Library Manager, right-click the application name from the Project Workspace window in the IDE, and select **ModusToolbox > Library Manager**. For more details, see the IDE User Guide: *{ModusToolbox install directory}/ide_2.0/docs/mt_ide_user_guide.pdf*.

   You can also just start the application creation process again and select a different kit.

   If you want to use the application for a kit not listed here, you may need to update the source files. If the kit does not have the required resources, the application may not work.

3. In the **Starter Application** window, choose the example.

4. Click **Next** and complete the application creation process.

See [Importing Code Example into ModusToolbox IDE - KBA225201](https://community.cypress.com/docs/DOC-15968) for details.

### In Command-line Interface (CLI):

1. Download and unzip this repository onto your local machine, or clone the repository.

2. Open a CLI terminal and navigate to the application folder.

3. Import required libraries by executing the `make getlibs` command.

## Operation

1. Connect the CY8CKIT-028-TFT shield to the CY8CKIT-062-WiFi-BT kit, or connect Pmod I2S2 to the CY8CPROTO-062-4343W kit.

2. Connect the board to your PC using the provided USB cable through the KitProg USB connector.

3. Program the board.

   ### Using ModusToolbox IDE:

   1. Select the application project in the Project Explorer.

   2. In the **Quick Panel**, scroll down, and click **\<Application Name> Program (KitProg3)**.

   ### Using CLI:

   1. From the terminal, execute the `make program` command to build and program the application using the default toolchain to the default target. You can specify a target and toolchain manually:
        ```
        make program TARGET=<BSP> TOOLCHAIN=<toolchain>
        ```
        Example:

        ```
        make program TARGET=CY8CKIT-062-WIFI-BT TOOLCHAIN=GCC_ARM
        ```
        **Note**:  Before building the application, ensure that the *libs* folder contains the BSP file (*TARGET_xxx.lib*) corresponding to the TARGET. Execute the `make getlibs` command to fetch the BSP contents before building the application.

   After programming, the application starts automatically. 

4. Connect a headphone or earphone to the audio jack of the CY8CKIT-028-TFT shield, or to the Pmod I2S2 audio jack.

5. Connect another USB cable from your PC (or reuse the same cable used to program the kit) to the USB device connector [J28 for CY8CKIT-062-WiFi-BT or J10 for CY8CPROTO-062-4343W]. 

6. On the computer, verify that a new USB device was enumerated as a Speaker/Microphone and named as "PSoC 6 USB Audio Device".

7. Use your audio software to specify the microphone or speaker. In the Audacity software tool, select the microphone and speaker name as "PSoC 6 USB Audio Device".

![Audacity](images/Audacity.png)

8. Record some sound. In Audacity, press the Record button ![Record](images/Record.png). Stop at any time by pressing the Stop button ![Stop](images/Stop.png).

9. Play the record by pressing the Play button ![Play](images/Play.png) and confirm that the sound was recorded correctly.

10. Open a music player in the PC and start playing a song. Use the CapSense slider to change the volume (you don't need to slide the finger, simply touch the slider on the left or on the right).

    **Note**: You might need to set the Speaker "PSoC 6 USB Audio Device" as the default speaker device in your OS.

11. Press the Left CapSense button (BTN0) in the kit to Play/Pause a sound track.

12. Press the Right CapSense button (BTN1) in the kit to Stop a sound track.

## Debugging

You can debug the example to step through the code. In the ModusToolbox IDE, use the **\<Application Name> Debug (KitProg3)** configuration in the **Quick Panel**. See [Debugging a PSoC 6 MCU ModusToolbox Project - KBA224621](https://community.cypress.com/docs/DOC-15763) for details.

## Design and Implementation

The [CY8CKIT-028-TFT](https://www.cypress.com/documentation/development-kitsboards/tft-display-shield-board-cy8ckit-028-tft) shield contains the audio codec [AK4954A](https://static6.arrow.com/aropdfconversion/47cd3adddec5b3d93811e75b58ad024fefd8e164/27149850412266ak4954aen.pdf), which is a 32-bit Stereo codec with microphone. The [PMod I2S2](https://store.digilentinc.com/pmod-i2s2-stereo-audio-input-and-output/) module contains the [Cirrus CS5343](https://www.cirrus.com/products/cs5343-44/?_ga=2.191300067.810289828.1576048380-104852753.1571286442) and [Cirrus CS4344](https://www.cirrus.com/products/cs4344-45-48/?_ga=2.191300067.810289828.1576048380-104852753.1571286442) converters. If using AK4594A, the PSoC 6 MCU device configures the audio codec with the I2C Master (SCB) and streams audio data with I2S interface, which operates as Master (TX) and Slave (RX). If using Pmod I2S2, you don't need to configure it over I2C and the I2S interface operates as a Master only (TX and RX). The codecs also require a Master clock (MCLK), which is generated by the PSoC 6 MCU device with a PWM (TCPWM). This clock requires to be at least 256x the frame rate. In this application, the frame rate can be set to 48 ksps and 44.1 ksps, requiring MCLK of 12.288 MHz and 11.2896 MHz, respectively.

### Firmware Details

The firmware implements a bridge between the USB and I2S blocks. The USB descriptor implements the Audio Device Class with four endpoints and the HID Device Class with one endpoint:

- **Audio Control Endpoint:** controls the access to the audio streams
- **Audio IN Endpoint:** sends audio data to the USB host
- **Audio OUT Endpoint:** receives audio data from the USB host
- **Audio Feedback Endpoint:** controls the sample rate in the OUT endpoint
- **HID Audio/Playback Control Endpoint:** controls the volume and audio stream

To manage the flow of data between USB and I2S with minimum CPU impact, six DMA channels transfer the data between the I2S FIFOs and the USB buffers. The application handles four of them, while the USB block handles the remaining two channels.

The USB buffers store interleaved 24-bit audio stereo data. However, the I2S FIFO requires 32-bit accesses, even if you configured the FIFO word length to be 24-bit. To handle this disparity, an intermediary sample variable temporarily stores the sample with 32 bits. For the OUT endpoint flow, CYBSP_DMA_USB_OUT transfers three bytes from the Audio OUT endpoint buffer to the intermediary sample variable; then CYBSP_DMA_I2S_TX transfers four bytes (one sample) to the I2S TX FIFO. The same methodology applies for the IN endpoint flow: CYBSP_DMA_USB_IN transfers four bytes (one sample) from the I2S RX FIFO to an intermediary buffer, then the CYBSP_DMA_I2S_RX transfers 3 bytes to the Audio IN endpoint. [Figure 2](#figure-2-audio-out-dma-tranfers) and [Figure 3](#figure-3-audio-in-dma-transfers) show DMA transfers.

##### Figure 2. Audio OUT DMA Transfers

![AudioOUT](images/AudioOUT.png)

##### Figure 3. Audio IN DMA Transfers

![AudioIN](images/AudioIN.png)

The example project firmware uses FreeRTOS on the CM4 CPU. The following tasks are created in *main.c*:
- **Audio App Task:** implements high-level functions related to the audio. For example, requests to change the sample rate and volume.
- **Audio IN Task:** implements the functions related to the audio IN endpoint.
- **Audio OUT Task:** implements the functions related to the audio OUT endpoint.
- **Touch Task:** implements the user interface related to CapSense and issues the HID control commands.
- **Idle Task:** Goes to sleep.

The example also uses the FreeRTOS Event Group, which notifies tasks when USB events occur.

[Figure 4](#figure-4-audio-out-and-feedaback-endpoints-flow) shows the firmware flowchart for the Audio OUT and Feedback Endpoint. 

##### Figure 4. Audio OUT and Feedback Endpoints Flow

![AudioOutFeedFlow](images/AudioOutFeedFlow.png)

[Figure 5](#figure-5-audio-in-endpoint-flow) shows the firmware flowchart for the Audio IN Endpoint.

##### Figure 5. Audio IN Endpoint Flow

![AudioInFlow](images/AudioInFlow.png)

The Audio IN task waits for a recording request from the USB host. When it receives the request, it prepares I2S RX to sample a new frame. It initially writes a null frame to the Audio IN Endpoint buffer to initiate the recording. The frame size depends on the sample rate (48 ksps or 44.1 ksps), the number of channels (2x), and the duration of USB transfer (1 ms). The overall equation is:

```
Frame size = Sample Rate x Number of Channels x Transfer Time
```

In this example, the frame size is equal to 48000 x 2 x 0.001 = 96 samples. Note that the Audio IN Endpoint Callback also implements a method to change the frame size depending on the FIFO level in the I2S RX FIFO. This is important because there are clocking differences between the USB host and the PSoC 6 MCU audio subsystem.

DMA transfers shown in [Figure 3](#figure-3-audio-in-dma-transfers) are configured to transfer an entire audio frame per trigger. It triggers when there is enough data in the I2S RX FIFO. Once it completes a transfer, it disables itself; it is enabled again only when the Audio IN Endpoint Callback executes. In this callback, it checks if the USB host still records. If yes, it keeps writing to the Audio IN Endpoint Buffer; otherwise, it stops sampling the microphone. 

In a similar way, the Audio OUT Task waits for a playing request from the USB host. When it receives the request, it prepares the I2S TX to stream a frame. It enables the OUT Endpoint, so the USB host can start transmitting the data. Once the USB host completes sending a frame, the OUT Endpoint Callback executes. It reads the data received and prepares the DMA descriptors to transfer the right amount of data. Note that the number of bytes received in the OUT endpoint might not contain all samples of a frame. For example, if the number of bytes received is not a multiple of 3 bytes (each sample is 24 bits), the remaining bytes should be transferred only on the next frame.

The DMA flow shown in [Figure 2](#figure-2-audio-out-dma-tranfers) is configured to transfer an entire audio frame per trigger. It triggers on the Audio OUT Endpoint Callback. Once it completes a transfer, it disables itself; it is enabled again only when the Audio OUT Endpoint Callback executes again. The USB OUT Endpoint is also enabled on every callback execution, so that the USB host can keep streaming data.

There is also a mechanism to synchronize the clocks between USB host and the PSoC 6 MCU audio subsystem in the OUT endpoint flow. It uses the Feedback Endpoint callback to report back to the USB host how fast I2S TX streams the data, so that the host can increase or decrease the sample rate.

In the Touch task, the firmware uses the CapSense resource to scan finger touches on the kit's buttons and slider. The left button (BTN0) plays or pauses a sound track, and the right button (BTN1) stops a sound track. This is achieved by sending a command over the HID Audio/Playback Control endpoint. The CapSense slider controls the volume. It also sends a command over the HID and configures the volume played in the audio codec.

The project consists of the following files:

- *main.c (CM4)* contains the main function, which is the entry point for executing the application. The main function sets up user tasks and starts the RTOS scheduler. It also implements the Idle task.
- *audio_app.c/h* files contain the task that handle the audio-related blocks, such as the USB Audio events and I2S control.
- *audio_in.c/h* files contain the task that handles recording requests to the Audio IN endpoint. These files also implement the Audio IN Data Endpoint callback.
- *audio_out.c/h* files contain the task that handles playing requests to the Audio OUT endpoint. These files also implement the Audio OUT Data Endpoint callback.
- *audio.h* contains macros related to the USBFS descriptor.
- *usb_comm.c/h* files contain macros and functions related to the USBFS block and USB Audio Device class.
- *audio_feed.c/h* files implement the Audio Feedback Endpoint callback.
- *touch.c/h* files handle the CapSense calls and write to the Audio/Playback Control endpoint.
- *ak4954a.c/h* files implement the driver for the AK4954A audio codec.
- *rtos.h* contains macros and handles for the FreeRTOS components in the application.
- *FreeRTOSConfig.h* contains the FreeRTOS settings and configuration. Non-default setting are marked with inline comments. For details of FreeRTOS configuration options, see the [FreeRTOS customization](https://www.freertos.org/a00110.html) webpage.

### Resources and Settings

[Table 1](#table-1-application-resources) lists the ModusToolbox resources used in this example, and how they are used in the design.

##### Table 1. Application Resources
| Resource  |  Alias/Object     |    Purpose     |
| :------- | :------------    | :------------ |
| SCB (I2C) (HAL) | mi2c          | I2C Master Driver to configure the audio codec |
| TCPWM (PWM) (HAL)| mclk_pwm | PWM to clock the external audio codec |
| USBDEV (PDL)   | CYBSP_USBDEV         | USB Device configured with Audio and HID Descriptors  |
| DMA (PDL)    | CYBSP_DMA_USB_IN    | Transfers data from the SRAM to USB buffer |
| DMA (PDL)    | CYBSP_DMA_I2S_RX | Transfers data from the I2S RX FIFO to SRAM |
| DMA (PDL)    | CYBSP_DMA_USB_OUT | Transfers data from the USB buffer to SRAM |
| DMA (PDL)    | CYBSP_DMA_I2S_TX | Transfers data from the SRAM to the I2S TX FIFO |
| I2S (PDL)    | CYBSP_I2S | Interface with the audio codec |
| CapSense (PDL) | CYBSP_CSD | Scan for button and slider touches |

## Related Resources

| Application Notes                                            |                                                              |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| [AN228571](https://www.cypress.com/AN228571) – Getting Started with PSoC 6 MCU on ModusToolbox | Describes PSoC 6 MCU devices and how to build your first application with ModusToolbox |
| [AN221774](https://www.cypress.com/AN221774) – Getting Started with PSoC 6 MCU on PSoC Creator | Describes PSoC 6 MCU devices and how to build your first application with PSoC Creator |
| [AN215656](https://www.cypress.com/AN215656) – PSoC 6 MCU: Dual-CPU System Design | Describes the dual-CPU architecture in PSoC 6 MCU, and shows how to build a simple dual-CPU design |
| **Code Examples**                                            |                                                              |
| [Using ModusToolbox IDE](https://github.com/cypresssemiconductorco/Code-Examples-for-ModusToolbox-Software) | [Using PSoC Creator](https://www.cypress.com/documentation/code-examples/psoc-6-mcu-code-examples) |
| **Device Documentation**                                     |                                                              |
| [PSoC 6 MCU Datasheets](https://www.cypress.com/search/all?f[0]=meta_type%3Atechnical_documents&f[1]=resource_meta_type%3A575&f[2]=field_related_products%3A114026) | [PSoC 6 Technical Reference Manuals](https://www.cypress.com/search/all/PSoC%206%20Technical%20Reference%20Manual?f[0]=meta_type%3Atechnical_documents&f[1]=resource_meta_type%3A583) |
| **Development Kits**                                         | Buy at www.cypress.com                                       |
| [CY8CKIT-062-WiFi-BT](https://www.cypress.com/CY8CKIT-062-WiFi-BT) PSoC 6 WiFi-BT Pioneer Kit | [CY8CPROTO-062-4343W](https://www.cypress.com/cy8cproto-062-4343w) PSoC 6 Wi-Fi BT Prototyping Kit |
| [CY8CKIT-062S2-43012](https://www.cypress.com/CY8CKIT-062S2-43012) PSoC 62S2 Wi-Fi BT Pioneer Kit |                                                              |
| **Libraries**                                                |                                                              |
| PSoC 6 Peripheral Driver Library and docs                    | [psoc6pdl](https://github.com/cypresssemiconductorco/psoc6pdl) on GitHub |
| Cypress Hardware Abstraction Layer Library and docs          | [psoc6hal](https://github.com/cypresssemiconductorco/psoc6hal) on GitHub |
| RetargetIO - A utility library to retarget the standard input/output (STDIO) messages to a UART port | [retarget-io](https://github.com/cypresssemiconductorco/retarget-io) on GitHub |
| **Middleware**                                               |                                                              |
| CapSense library and docs                                    | [capsense](https://github.com/cypresssemiconductorco/capsense) on GitHub |
| USB Device library and docs                                    | [usbdev](https://github.com/cypresssemiconductorco/usbdev) on GitHub |
| Links to all PSoC 6 MCU Middleware                           | [psoc6-middleware](https://github.com/cypresssemiconductorco/psoc6-middleware) on GitHub |
| **Tools**                                                    |                                                              |
| [ModusToolbox IDE](https://www.cypress.com/modustoolbox)     | The Cypress IDE for PSoC 6 MCU and IoT designers             |
| [PSoC Creator](https://www.cypress.com/products/psoc-creator-integrated-design-environment-ide) | The Cypress IDE for PSoC and FM0+ MCU development            |

## Other Resources

Cypress provides a wealth of data at www.cypress.com to help you select the right device, and quickly and effectively integrate it into your design.

For PSoC 6 MCU devices, see [How to Design with PSoC 6 MCU - KBA223067](https://community.cypress.com/docs/DOC-14644) in the Cypress community.

## Document History

Document Title: *CE224997* - *PSoC 6 MCU: USB Audio Device with FreeRTOS*

| Version | Description of Change |
| ------- | --------------------- |
| 1.0.0   | New code example      |

------

All other trademarks or registered trademarks referenced herein are the property of their respective owners.

![Banner](images/Banner.png)

-------------------------------------------------------------------------------

© Cypress Semiconductor Corporation, 2020. This document is the property of Cypress Semiconductor Corporation and its subsidiaries ("Cypress"). This document, including any software or firmware included or referenced in this document ("Software"), is owned by Cypress under the intellectual property laws and treaties of the United States and other countries worldwide. Cypress reserves all rights under such laws and treaties and does not, except as specifically stated in this paragraph, grant any license under its patents, copyrights, trademarks, or other intellectual property rights. If the Software is not accompanied by a license agreement and you do not otherwise have a written agreement with Cypress governing the use of the Software, then Cypress hereby grants you a personal, non-exclusive, nontransferable license (without the right to sublicense) (1) under its copyright rights in the Software (a) for Software provided in source code form, to modify and reproduce the Software solely for use with Cypress hardware products, only internally within your organization, and (b) to distribute the Software in binary code form externally to end users (either directly or indirectly through resellers and distributors), solely for use on Cypress hardware product units, and (2) under those claims of Cypress's patents that are infringed by the Software (as provided by Cypress, unmodified) to make, use, distribute, and import the Software solely for use with Cypress hardware products. Any other use, reproduction, modification, translation, or compilation of the Software is prohibited.  
TO THE EXTENT PERMITTED BY APPLICABLE LAW, CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD TO THIS DOCUMENT OR ANY SOFTWARE OR ACCOMPANYING HARDWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. No computing device can be absolutely secure. Therefore, despite security measures implemented in Cypress hardware or software products, Cypress shall have no liability arising out of any security breach, such as unauthorized access to or use of a Cypress product. CYPRESS DOES NOT REPRESENT, WARRANT, OR GUARANTEE THAT CYPRESS PRODUCTS, OR SYSTEMS CREATED USING CYPRESS PRODUCTS, WILL BE FREE FROM CORRUPTION, ATTACK, VIRUSES, INTERFERENCE, HACKING, DATA LOSS OR THEFT, OR OTHER SECURITY INTRUSION (collectively, "Security Breach"). Cypress disclaims any liability relating to any Security Breach, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any Security Breach. In addition, the products described in these materials may contain design defects or errors known as errata which may cause the product to deviate from published specifications. To the extent permitted by applicable law, Cypress reserves the right to make changes to this document without further notice. Cypress does not assume any liability arising out of the application or use of any product or circuit described in this document. Any information provided in this document, including any sample design information or programming code, is provided only for reference purposes. It is the responsibility of the user of this document to properly design, program, and test the functionality and safety of any application made of this information and any resulting product. "High-Risk Device" means any device or system whose failure could cause personal injury, death, or property damage. Examples of High-Risk Devices are weapons, nuclear installations, surgical implants, and other medical devices. "Critical Component" means any component of a High-Risk Device whose failure to perform can be reasonably expected to cause, directly or indirectly, the failure of the High-Risk Device, or to affect its safety or effectiveness. Cypress is not liable, in whole or in part, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any use of a Cypress product as a Critical Component in a High-Risk Device. You shall indemnify and hold Cypress, its directors, officers, employees, agents, affiliates, distributors, and assigns harmless from and against all claims, costs, damages, and expenses, arising out of any claim, including claims for product liability, personal injury or death, or property damage arising from any use of a Cypress product as a Critical Component in a High-Risk Device. Cypress products are not intended or authorized for use as a Critical Component in any High-Risk Device except to the limited extent that (i) Cypress's published data sheet for the product explicitly states Cypress has qualified the product for use in a specific High-Risk Device, or (ii) Cypress has given you advance written authorization to use the product as a Critical Component in the specific High-Risk Device and you have signed a separate indemnification agreement.  
Cypress, the Cypress logo, Spansion, the Spansion logo, and combinations thereof, WICED, PSoC, CapSense, EZ-USB, F-RAM, and Traveo are trademarks or registered trademarks of Cypress in the United States and other countries. For a more complete list of Cypress trademarks, visit cypress.com. Other names and brands may be claimed as property of their respective owners.
