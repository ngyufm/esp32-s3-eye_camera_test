#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "bsp\esp-bsp.h"
#include "freertos\FreeRTOS.h"
#include "freertos\task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_camera.h"

static char *TAG = "app_main";

button_handle_t btns[BSP_BUTTON_NUM];
camera_fb_t *pic;

volatile bool captureNow = false;

void setCaptureFlag(){
    ESP_LOGI(TAG, "Capturing...");
    captureNow = true;
}

int getImgCount(){
    FILE *ICFPtr = fopen(BSP_SD_MOUNT_POINT"/imgcnt", "r");
    char text[9];
    fread(text, 1, 9, ICFPtr);
    fclose(ICFPtr);
    return strtoimax(text, NULL, 10);
}

int currentImgCount;
void updateImgCount(){
    FILE *ICFPtr = fopen(BSP_SD_MOUNT_POINT"/imgcnt", "w");
    fprintf(ICFPtr, "%d", currentImgCount);
    fclose(ICFPtr);
}


void app_main(void)
{
    ESP_LOGI(TAG, "HOI I'M TEMMIE!");

    bsp_iot_button_create(btns, NULL, BSP_BUTTON_NUM);
    bsp_i2c_init();
    bsp_leds_init();

    esp_err_t sdRetCode = bsp_sdcard_mount();

    char* error_code = esp_err_to_name( (sdRetCode) );
    ESP_LOGI(TAG, "Recieved feedback: %s", error_code);

    if(sdRetCode != ESP_OK){
        ESP_LOGE(TAG, "SD Card Mount Failed");
        return;
    }

    currentImgCount = getImgCount();
    ESP_LOGI(TAG, "Current Img count: %d", currentImgCount);

    bsp_display_start();
    bsp_display_backlight_on();

    const camera_config_t camera_config = BSP_CAMERA_DEFAULT_CONFIG;        // << sets frame size to 240x240, matching the display res
    if(esp_camera_init(&camera_config) != ESP_OK){
        ESP_LOGE(TAG, "Camera Init Failed");
        return;
    }
    //sensor_t *s = esp_camera_sensor_get();
    
    uint32_t cam_buff_size = BSP_LCD_H_RES * BSP_LCD_V_RES * 2;
    uint8_t *cam_buff = heap_caps_malloc(cam_buff_size, MALLOC_CAP_SPIRAM);
    assert(cam_buff);

    bsp_display_lock(0);
    lv_obj_t *camera_canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(camera_canvas, cam_buff, BSP_LCD_H_RES, BSP_LCD_V_RES, LV_COLOR_FORMAT_RGB565);
    assert(camera_canvas);
    lv_obj_center(camera_canvas);
    bsp_display_unlock();

    ESP_LOGI(TAG, "registering 'captureImage' function to button...");
    iot_button_register_cb(btns[BSP_BUTTON_MENU], BUTTON_SINGLE_CLICK, setCaptureFlag, NULL);



    while(1){
        pic = esp_camera_fb_get();
        if(pic){
            esp_camera_fb_return(pic);
            bsp_display_lock(0);
            memcpy(cam_buff, pic->buf, cam_buff_size);
            if(BSP_LCD_BIGENDIAN){
                lv_draw_sw_rgb565_swap(cam_buff, cam_buff_size);
            }
            lv_obj_invalidate(camera_canvas);
            bsp_display_unlock();
        }else{
            ESP_LOGE(TAG, "Failed to get frame");
        }


        if(captureNow){
            ESP_LOGI(TAG, "Doing thing...");
            char fileName[30];
            sprintf(fileName, BSP_SD_MOUNT_POINT"/img%d.txt", currentImgCount);
            FILE *fptr = fopen( fileName, "w");
            if (fptr == NULL){
                ESP_LOGE(TAG, "FAILED to open file");
            }else{
                ESP_LOGI(TAG, "Opened file");
                fwrite(pic->buf, 1, pic->len, fptr);
                vTaskDelay(4);  // 40ms
                fclose(fptr);
                ESP_LOGI(TAG, "sucess, saved into %s as %d", fileName, pic->format);
                
                currentImgCount++;
                updateImgCount();
                ESP_LOGI(TAG, "Current Img count: %d", currentImgCount);

            }
            captureNow = false;
        }

        vTaskDelay(1);  // 10ms
    }
}
