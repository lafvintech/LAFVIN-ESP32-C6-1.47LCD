.. _install_arduino_ide:

Install Arduino IDE
=======================

Download the Arduino IDE 2.x.x
-------------------------------

#. Visit `Download Arduino IDE <https://www.arduino.cc/en/software>`_ page.

#. Download the IDE for your OS version.

   .. image:: img/Install_Arduino_IDE_1.png

.. note:: Uploading code to the Arduino UNO R4 requires Arduino IDE version 2.2 
   or higher. If your version is older, please upgrade to the latest version.

Installation
--------------

Windows
^^^^^^^^

#. Double click the ``arduino-ide_xxxx.exe`` file to run the downloaded file.

#. Read the License Agreement and agree it.

   .. image:: img/Install_Arduino_IDE_2.png

#. Choose installation options.

   .. image:: img/Install_Arduino_IDE_3.png

#. Choose install location. It is recommended that the software be installed on a drive other than the system drive.

   .. image:: img/Install_Arduino_IDE_4.png

#. Then Finish. 

   .. image:: img/Install_Arduino_IDE_5.png

MacOS
^^^^^^^^

Double click on the downloaded ``arduino_ide_xxxx.dmg`` file and follow the 
instructions to copy the **Arduino IDE.app** to the **Applications** folder, you will see the Arduino IDE installed successfully after a few seconds.

.. image:: img/Install_Arduino_IDE_6.png
    :width: 800

Linux
"""""""

For the tutorial on installing the Arduino IDE 2.0 on a Linux system, please 
refer `Linux-Install Arduino IDE <https://docs.arduino.cc/software/ide-v2/tutori
als/getting-started/ide-v2-downloading-and-installing#linux>`_

Open the IDE
^^^^^^^^^^^^^

#. When you first open Arduino IDE 2.0, it automatically installs the Arduino AVR Boards, built-in libraries, and other required files.

   .. image:: img/Install_Arduino_IDE_7.png

#. In addition, your firewall or security center may pop up a few times asking you if you want to install some device driver. Please install all of them.

   .. image:: img/Install_Arduino_IDE_8.png

#. Now your Arduino IDE is ready!

.. note::
   In the event that some installations didn't work due to network issues or other 
   reasons, you can reopen the Arduino IDE and it will finish the rest of the 
   installation. The Output window will not automatically open after all installations 
   are complete unless you click Verify or Upload.


Environment Configuration
------------------------------------
First, open the software platform arduino, and then click File in Menus and select Preferences.

.. image:: img/evn_1.png

Second, click on the symbol behind "Additional Boards Manager URLs"

.. image:: img/evn_2.png

Third, fill in **https://raw.githubusercontent.com/espressif/arduino-esp32/ghpages/package_esp32_index.json**
in the new window, click OK, and click OK on the Preferences window again.

.. image:: img/evn_3.png

Fourth, click "Boards Manager". Enter “esp32” in Boards manager and select 3.0.4，
Then click “INSTALL”.

.. image:: img/evn_4.png

Arduinowill download these files automaticly. Wait for the installation to complete.

.. image:: img/evn_5.png

When finishing installation, click Tools in the Menus again and select Board: 
"Arduino Uno", and then you can see information of ESP32. click "ESP32-S3 Dev Module" 
so that the ESP32-S3 programming development environment is configured.

.. image:: img/evn_6.png