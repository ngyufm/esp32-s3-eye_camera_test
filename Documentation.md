VScode's "#include" red squigglies will not disappear even thou the C intellisense config is told to search the whole directory

ESP-IDF is run on an external command prompt (cuz I have a spare screen to afford this screen real estate)

"Easiest" way to start programming on the ESP32-S3-eye is to:
1. Install ESP-IDF
2. Download the ESP-BSP (Board Support Package) zip 
3. Create a project with idf.py (use the cmd/ps shortcut created from installing the ESP-IDF) with `idf.py create-project <name>`
4. Add the `./esp32_s3_eye` folder (located under the `./bsp` folder) into the bsp folder at the project root
5. Add the `./esp_lvgl_port` folder (located under the `./components` folder) into the components folder at the project root
6. Run `idf.py reconfigure` and hope that it builds successfully
7. Run `idf.py menuconfig` and setup the chip by 

    7.1. Setting the correct flash size (QIO 8MB)

    7.2. Enabling PSRAM under: `"idf.py menuconfig" -> component config -> ESP PSRAM`

    7.3. PSRAM settings: Octal Mode PSRAM, [same RAM speed as flash]
8. Run `idf.py all` to build all the files, it should work and add the components/dependancies into `./managed_components`

To use the camera, PSRAM needs to be enabled under: `"idf.py menuconfig" -> component config -> ESP PSRAM`