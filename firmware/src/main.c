/* ESP32-S3 native USB composite: HID gamepad + CDC ACM.
   GPIO4=pot wiper, GPIO5/6=buttons to GND. */
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "mbedtls/base64.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "tusb.h"
#include "tusb_cdc_acm.h"
#include "version.h"

#define POT_GPIO GPIO_NUM_4
#define UP_GPIO GPIO_NUM_5
#define DOWN_GPIO GPIO_NUM_6
#define MIN_SPAN 300
typedef struct __attribute__((packed)) { uint16_t adc_min, adc_max; uint8_t valid, debounce; } settings_t;
typedef struct __attribute__((packed)) { uint16_t x; uint8_t buttons; } game_report_t;
static settings_t cfg = {.debounce=5}; static adc_oneshot_unit_handle_t adc; static game_report_t report;
static uint32_t up_count, down_count; static const char *TAG="setup-gas";
static char wifi_ssid[33], wifi_pass[65], ota_pass[33];

/* Gamepad: X is the sole analogue axis; buttons 1 and 2 are paddle up/down. */
static const uint8_t hid_report_descriptor[] = {
  0x05,0x01,0x09,0x05,0xA1,0x01, 0x09,0x30,0x16,0x00,0x00,0x26,0xff,0xff,0x75,0x10,0x95,0x01,0x81,0x02,
  0x05,0x09,0x19,0x01,0x29,0x02,0x15,0x00,0x25,0x01,0x75,0x01,0x95,0x02,0x81,0x02,0x75,0x06,0x95,0x01,0x81,0x03,0xC0
};

/* Espressif's stock descriptor covers CDC but not HID.  Supply the complete
   native-USB composite layout explicitly: CDC (interfaces 0/1) + gamepad (2). */
enum { ITF_CDC = 0, ITF_CDC_DATA, ITF_HID, ITF_COUNT };
enum { EP_CDC_NOTIF = 0x81, EP_CDC_OUT = 0x02, EP_CDC_IN = 0x82, EP_HID_IN = 0x83 };
#define USB_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN)
static const uint8_t usb_configuration_descriptor[] = {
  TUD_CONFIG_DESCRIPTOR(1, ITF_COUNT, 0, USB_CONFIG_TOTAL_LEN, 0, 100),
  TUD_CDC_DESCRIPTOR(ITF_CDC, 4, EP_CDC_NOTIF, 8, EP_CDC_OUT, EP_CDC_IN, 64),
  TUD_HID_DESCRIPTOR(ITF_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(hid_report_descriptor), EP_HID_IN, 16, 1),
};
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) { (void)instance; return hid_report_descriptor; }
uint16_t tud_hid_get_report_cb(uint8_t i,uint8_t id,hid_report_type_t t,uint8_t*b,uint16_t n){(void)i;(void)id;(void)t;(void)b;(void)n;return 0;}
void tud_hid_set_report_cb(uint8_t i,uint8_t id,hid_report_type_t t,uint8_t const*b,uint16_t n){(void)i;(void)id;(void)t;(void)b;(void)n;}

static void load_settings(void) { nvs_handle_t h; size_t n=sizeof cfg; if(nvs_open("gas",NVS_READONLY,&h)==ESP_OK){ nvs_get_blob(h,"settings",&cfg,&n); nvs_close(h); } if(cfg.debounce<2||cfg.debounce>30) cfg.debounce=5; if(cfg.adc_max<=cfg.adc_min+MIN_SPAN) cfg.valid=0; }
static void save_settings(void) { nvs_handle_t h; if(nvs_open("gas",NVS_READWRITE,&h)==ESP_OK){nvs_set_blob(h,"settings",&cfg,sizeof cfg);nvs_commit(h);nvs_close(h);} }
static void load_wifi(void) { nvs_handle_t h; size_t n; if(nvs_open("net",NVS_READONLY,&h)==ESP_OK){n=sizeof wifi_ssid;nvs_get_str(h,"ssid",wifi_ssid,&n);n=sizeof wifi_pass;nvs_get_str(h,"pass",wifi_pass,&n);n=sizeof ota_pass;nvs_get_str(h,"ota",ota_pass,&n);nvs_close(h);} }
static void save_wifi(void) { nvs_handle_t h; if(nvs_open("net",NVS_READWRITE,&h)==ESP_OK){nvs_set_str(h,"ssid",wifi_ssid);nvs_set_str(h,"pass",wifi_pass);nvs_set_str(h,"ota",ota_pass);nvs_commit(h);nvs_close(h);} }
static int read_adc(void) { int sum=0,v=0; for(int i=0;i<4;i++){adc_oneshot_read(adc,ADC_CHANNEL_3,&v);sum+=v;} return sum/4; }
static void reply(const char *s) { tud_cdc_write(s, strlen(s)); tud_cdc_write_flush(); }
static void command(char *s) {
  int raw=read_adc(); char o[180];
  if(!strcmp(s,"ID?")) reply("SETUP-GAS,1.0.0\r\n");
  else if(!strcmp(s,"GET")){snprintf(o,sizeof o,"OK ADC=%d GAS=%u CAL=%u MIN=%u MAX=%u DEBOUNCE=%u SAVED=%u UP=%lu DOWN=%lu\r\n",raw,report.x,cfg.valid,cfg.adc_min,cfg.adc_max,cfg.debounce,cfg.debounce,(unsigned long)up_count,(unsigned long)down_count);reply(o);}
  else if(!strcmp(s,"CAL ZERO")){cfg.adc_min=raw;reply("OK ZERO ACQUISITO\r\n");}
  else if(!strcmp(s,"CAL FULL")){cfg.adc_max=raw;reply("OK FULL ACQUISITO\r\n");}
  else if(!strcmp(s,"CAL SAVE")){cfg.valid=(abs((int)cfg.adc_max-(int)cfg.adc_min)>=MIN_SPAN);if(cfg.valid){save_settings();reply("OK CALIBRAZIONE SALVATA\r\n");}else reply("ERR INTERVALLO TROPPO PICCOLO\r\n");}
  else if(!strcmp(s,"CAL CLEAR")){cfg.valid=0;save_settings();reply("OK CALIBRAZIONE CANCELLATA\r\n");}
  else if(!strncmp(s,"DEBOUNCE ",9)){int d=atoi(s+9);if(d>=2&&d<=30){cfg.debounce=d;reply("OK FILTRO ATTIVO\r\n");}else reply("ERR DEBOUNCE 2..30\r\n");}
  else if(!strcmp(s,"DEBOUNCE SAVE")){save_settings();reply("OK FILTRO SALVATO\r\n");}
  else if(!strcmp(s,"COUNTERS CLEAR")){up_count=down_count=0;reply("OK CONTATORI AZZERATI\r\n");}
  else if(!strncmp(s,"WIFI SSID ",10)){strlcpy(wifi_ssid,s+10,sizeof wifi_ssid);reply("OK SSID IN RAM\r\n");}
  else if(!strncmp(s,"WIFI PASS ",10)){strlcpy(wifi_pass,s+10,sizeof wifi_pass);reply("OK PASSWORD IN RAM\r\n");}
  else if(!strncmp(s,"OTA PASS ",9)){strlcpy(ota_pass,s+9,sizeof ota_pass);reply("OK PASSWORD OTA IN RAM\r\n");}
  else if(!strcmp(s,"WIFI SAVE")){save_wifi();reply("OK RETE SALVATA; RIAVVIA\r\n");}
  else reply("ERR COMANDO: ID? GET CAL ZERO|FULL|SAVE|CLEAR DEBOUNCE n|SAVE COUNTERS CLEAR\r\n");
}
static esp_err_t ota_handler(httpd_req_t *r) { char auth[128]={0}, expected[100], decoded[80]={0}; size_t n=httpd_req_get_hdr_value_len(r,"Authorization"), out=0; if(n>=sizeof auth || httpd_req_get_hdr_value_str(r,"Authorization",auth,sizeof auth)!=ESP_OK) return httpd_resp_send_err(r,HTTPD_401_UNAUTHORIZED,"auth richiesta"); snprintf(expected,sizeof expected,"gas:%s",ota_pass); if(!ota_pass[0] || strncmp(auth,"Basic ",6)||mbedtls_base64_decode((unsigned char*)decoded,sizeof decoded,&out,(const unsigned char*)auth+6,strlen(auth+6))||out!=strlen(expected)||memcmp(decoded,expected,out)) return httpd_resp_send_err(r,HTTPD_401_UNAUTHORIZED,"auth errata");
  const esp_partition_t *next=esp_ota_get_next_update_partition(NULL); esp_ota_handle_t h; if(esp_ota_begin(next,OTA_SIZE_UNKNOWN,&h)!=ESP_OK)return httpd_resp_send_err(r,HTTPD_500_INTERNAL_SERVER_ERROR,"slot OTA"); char buf[1024]; int left=r->content_len; while(left){int got=httpd_req_recv(r,buf,left>sizeof buf?sizeof buf:left);if(got<=0){esp_ota_abort(h);return httpd_resp_send_err(r,HTTPD_500_INTERNAL_SERVER_ERROR,"upload interrotto");}if(esp_ota_write(h,buf,got)!=ESP_OK){esp_ota_abort(h);return httpd_resp_send_err(r,HTTPD_500_INTERNAL_SERVER_ERROR,"scrittura");}left-=got;} if(esp_ota_end(h)!=ESP_OK||esp_ota_set_boot_partition(next)!=ESP_OK)return httpd_resp_send_err(r,HTTPD_500_INTERNAL_SERVER_ERROR,"immagine non valida"); httpd_resp_sendstr(r,"OK; riavvio nello slot inattivo\n");vTaskDelay(pdMS_TO_TICKS(300));esp_restart();return ESP_OK; }
static esp_err_t version_handler(httpd_req_t *r) { return httpd_resp_sendstr(r,GAS_FIRMWARE_VERSION); }
static void wifi_ota_start(void) { if(!wifi_ssid[0]) return; esp_netif_init();esp_event_loop_create_default();esp_netif_create_default_wifi_sta();wifi_init_config_t wi=WIFI_INIT_CONFIG_DEFAULT();esp_wifi_init(&wi);wifi_config_t c={0};strlcpy((char*)c.sta.ssid,wifi_ssid,sizeof c.sta.ssid);strlcpy((char*)c.sta.password,wifi_pass,sizeof c.sta.password);esp_wifi_set_mode(WIFI_MODE_STA);esp_wifi_set_config(WIFI_IF_STA,&c);esp_wifi_start();esp_wifi_connect();httpd_config_t hc=HTTPD_DEFAULT_CONFIG();httpd_handle_t server; if(httpd_start(&server,&hc)==ESP_OK){httpd_uri_t u={.uri="/update",.method=HTTP_POST,.handler=ota_handler};httpd_uri_t v={.uri="/version",.method=HTTP_GET,.handler=version_handler};httpd_register_uri_handler(server,&u);httpd_register_uri_handler(server,&v);} }
/* Poll TinyUSB directly.  The ESP wrapper callback can be missed on a
   composite device before the host toggles modem-control lines. */
static void cdc_poll(void){static char line[80];static size_t p;while(tud_cdc_available()){uint8_t b;tud_cdc_read(&b,1);if(b=='\r'||b=='\n'){if(p){line[p]=0;command(line);p=0;}}else if(p<sizeof line-1)line[p++]=b;}}
static void cdc_task(void *arg) { (void)arg; bool announced=false; while(1){ if(tud_mounted()&&!announced){reply("SETUP-GAS CDC PRONTA\r\n");announced=true;} cdc_poll(); vTaskDelay(pdMS_TO_TICKS(2)); } }
static void input_task(void *arg) { (void)arg; uint8_t stable[2]={1,1}, last[2]={1,1}, ticks[2]={0,0}; int filtered=0;
 while(1){int raw=read_adc(); filtered=(filtered*7+raw)/8; if(cfg.valid){int32_t d=(int32_t)cfg.adc_max-cfg.adc_min;int32_t x=((int32_t)filtered-cfg.adc_min)*65535/d;if(d<0)x=-x; if(x<0)x=0;if(x>65535)x=65535;report.x=x;}else report.x=0;
 uint8_t pins[2]={gpio_get_level(UP_GPIO),gpio_get_level(DOWN_GPIO)};for(int i=0;i<2;i++){if(pins[i]!=last[i]){last[i]=pins[i];ticks[i]=0;}else if(ticks[i]<cfg.debounce)ticks[i]++;if(ticks[i]==cfg.debounce&&stable[i]!=pins[i]){stable[i]=pins[i];if(!stable[i]){if(i)down_count++;else up_count++;}}} report.buttons=(!stable[0])|((!stable[1])<<1);
 if(tud_hid_ready())tud_hid_report(0,&report,sizeof report);vTaskDelay(pdMS_TO_TICKS(1));}}
void app_main(void) {
 ESP_ERROR_CHECK(nvs_flash_init()); load_settings();load_wifi(); gpio_config_t io={.pin_bit_mask=(1ULL<<UP_GPIO)|(1ULL<<DOWN_GPIO),.mode=GPIO_MODE_INPUT,.pull_up_en=1};gpio_config(&io);
 adc_oneshot_unit_init_cfg_t u={.unit_id=ADC_UNIT_1};adc_oneshot_new_unit(&u,&adc);adc_oneshot_chan_cfg_t ac={.atten=ADC_ATTEN_DB_12,.bitwidth=ADC_BITWIDTH_DEFAULT};adc_oneshot_config_channel(adc,ADC_CHANNEL_3,&ac);
 tinyusb_config_t usb={.configuration_descriptor=usb_configuration_descriptor};ESP_ERROR_CHECK(tinyusb_driver_install(&usb));
 ESP_LOGI(TAG,"%s ready; boot partition validated",GAS_FIRMWARE_VERSION); esp_ota_mark_app_valid_cancel_rollback(); wifi_ota_start(); xTaskCreate(cdc_task,"cdc",4096,NULL,11,NULL);xTaskCreate(input_task,"input",4096,NULL,10,NULL);
}
