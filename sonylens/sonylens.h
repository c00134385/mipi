
#ifndef __SONY_LENS_H
#define __SONY_LENS_H

//#define SONYLENS_10X_SUPPORT


//char sony_open_power[] = {0x81,0x01,0x04,0x00,0x02,0xff}; 
#define sony_open_power_data {0x81,0x01,0x04,0x00,0x02,0xff}

typedef enum SONY_RESULT {
    SONY_RESULT_BUSYING = 1,
    SONY_RESULT_OK = 0,
    SONY_RESULT_FAIL = -1,
    SONY_RESULT_UNKNOWN = -100,
} SONY_RESULT_E;


typedef enum SONY_STATE{
    SONY_12V_ON = 0,
    SONY_VISCA_INIT,
    SONY_IDLE,
    SONY_IDLE_2,
    SONY_SELF_CHECK_DONE,
    SONY_SET_ADDRESS,
    SONY_IF_CLEAR,
    SONY_POWER_ON,
    SONY_POWER_OFF,
    SONY_WAIT_READY,
    SONY_GET_POWER_STATE,
    SONY_GET_VERSION,
    SONY_GET_MONITOR_MODE,
    SONY_SET_MONITOR_MODE,
    SONY_SET_ZOOM_RATIO,
    SONY_CLEAR_TITLES,
    SONY_INIT_CONFIG,
    SONY_SET_SPLASH,
    SONY_DISPLAY_MONITOR_MODE,
    SONY_SET_LVDS_MODE,
    SONY_INIT_LENS,
    SONY_INIT_CAMERA,
    SONY_DISPLAY_MODE_ON,
    SONY_MUTE_MODE_OFF,
    SONY_POWER_WORKING,
    SONY_STANDBY,
    SONY_ENTER_TASK_2,
    SONY_SET_CAM_AE,
    SONY_SET_CAM_ZOOM,
    SONY_INIT_CONFIG_PARAMS,
    SONY_GET_CAM_ZOOM,
    SONY_SET_CAM_FOCUS,
    SONY_SET_CAM_AE_RESP,
    SONY_SET_CAM_DZOOM,
    SONY_SET_CAM_DISPLAY_OFF,
    SONY_SET_CAM_DISPLAY_ON,
    SONY_SET_AUTO_FOCUS,
    SONY_SET_CAM_AE_SENSITIVITY,
    SONY_SET_CAM_IRCORRECTION,
    SONY_SET_CAM_FOCUS_POSITION,
    SONY_SET_CAM_WB,
    
    //SONY_INQ_POWER,
    //SONY_INQ_DISPLAY_MODE,
	
	SONY_INIT,
    SONY_POWER_ON_AND_WORKING,
    SONY_CHECK,
	SONY_RETRY,
    
	//
	SONY_SEND,
	SONY_SEND_QUERY,
	SONY_ERROR,
	SONY_FINISHED,
    //
	MODEM_HW_ERROR = 100,
} SONY_STATE_E;

typedef enum SONY_CMD_STATE {
    SONY_CMD_IDLE = 0,
    SONY_CMD_WAIT_ACK,
    SONY_CMD_ACKED,
    SONY_CMD_WAIT_COMPLETION_MSG,
    SONY_CMD_COMPLETED,
    SONY_INQ_WAIT_RESULT,
    SONY_INQ_GET_RESULT,
} SONY_CMD_STATE_E;

typedef enum SONY_CAM_PROP {
    SONY_CAM_AE = 0,
    SONY_CAM_ZOOM,
    SONY_CAM_FOCUS,
    SONY_CAM_AE_RESP,
    SONY_CAM_DZOOM,
    SONY_CAM_DISPLAY_OFF,
    SONY_CAM_DISPLAY_ON,
    SONY_CAM_AUTO_FOCUS,
    SONY_CAM_AE_SENSITIVITY,
    SONY_CAM_IRCORRECTION,
    SONY_CAM_FOCUS_POSITION,
    SONY_CAM_WB,
} SONY_CAM_PROP_E;

typedef enum SONY_EVENT {
    SONY_EVENT_NULL = 0,
    SONY_EVENT_ZOOM_DOWN = 1,
    SONY_EVENT_ZOOM_UP = 2,
    SONY_EVENT_MONITOR_MODE_DOWN = 3,
    SONY_EVENT_MONITOR_MODE_UP = 4,
    SONY_EVENT_MENU_ON,

    SONY_EVENT_NAVI_UP,
    SONY_EVENT_NAVI_DOWN,
    SONY_EVENT_NAVI_LEFT,
    SONY_EVENT_NAVI_RIGHT,
    SONY_EVENT_NAVI_CENTER,
    
    SONY_EVENT_UNKNOWN = 99
} SONY_EVENT_E;



// monitoring mode
#define SONY_MONITOR_1080i_59_94 (0x01)
#define SONY_MONITOR_1080i_60    (0x02)
#define SONY_MONITOR_1080i_50    (0x04)
#define SONY_MONITOR_1080p_29_97 (0x06)
#define SONY_MONITOR_1080p_30    (0x07)
#define SONY_MONITOR_1080p_25    (0x08)
#define SONY_MONITOR_720p_59_94  (0x09)
#define SONY_MONITOR_720p_60     (0x0A)
#define SONY_MONITOR_720p_50     (0x0C)
#define SONY_MONITOR_720p_29_97  (0x0E)
#define SONY_MONITOR_720p_30     (0x0F)
#define SONY_MONITOR_720p_25     (0x11)
#define SONY_MONITOR_1080p_59_94 (0x13)
#define SONY_MONITOR_1080p_50    (0x14)
#define SONY_MONITOR_1080p_60    (0x15)


// zoom ration and zoom position
#ifdef SONYLENS_10X_SUPPORT
#define SONY_ZOOM_RATIO_1X (0x0000)
#define SONY_ZOOM_RATIO_2X (0x1448)
#define SONY_ZOOM_RATIO_3X (0x23f2)
#define SONY_ZOOM_RATIO_4X (0x2aa0)
#define SONY_ZOOM_RATIO_5X (0x335f)
#define SONY_ZOOM_RATIO_6X (0x36c5)
#define SONY_ZOOM_RATIO_7X (0x3859)
#define SONY_ZOOM_RATIO_8X (0x3ac7)
#define SONY_ZOOM_RATIO_9X (0x3d92)
#define SONY_ZOOM_RATIO_10X (0x4000)
#else
#define SONY_ZOOM_RATIO_1X (0x0000)
#define SONY_ZOOM_RATIO_2X (0x1851)
#define SONY_ZOOM_RATIO_3X (0x22be)
#define SONY_ZOOM_RATIO_4X (0x28f6)
#define SONY_ZOOM_RATIO_5X (0x2d45)
#define SONY_ZOOM_RATIO_6X (0x3086)
#define SONY_ZOOM_RATIO_7X (0x3320)
#define SONY_ZOOM_RATIO_8X (0x3549)
#define SONY_ZOOM_RATIO_9X (0x371e)
#define SONY_ZOOM_RATIO_10X (0x38b3)
#define SONY_ZOOM_RATIO_11X (0x3a12)
#define SONY_ZOOM_RATIO_12X (0x3b42)
#define SONY_ZOOM_RATIO_13X (0x3c47)
#define SONY_ZOOM_RATIO_14X (0x3d25)
#define SONY_ZOOM_RATIO_15X (0x3ddf)
#define SONY_ZOOM_RATIO_16X (0x3e7b)
#define SONY_ZOOM_RATIO_17X (0x3efb)
#define SONY_ZOOM_RATIO_18X (0x3f64)
#define SONY_ZOOM_RATIO_19X (0x3fba)
#define SONY_ZOOM_RATIO_20X (0x4000)
#endif

// focus near limit position
#define SONY_FOCUS_NEAR_LIMIT_OVER (0x1000)
#define SONY_FOCUS_NEAR_LIMIT_2500 (0x2000)
#define SONY_FOCUS_NEAR_LIMIT_1100 (0x3000)
#define SONY_FOCUS_NEAR_LIMIT_700  (0x4000)
#define SONY_FOCUS_NEAR_LIMIT_490  (0x5000)
#define SONY_FOCUS_NEAR_LIMIT_370  (0x6000)
#define SONY_FOCUS_NEAR_LIMIT_290  (0x7000)
#define SONY_FOCUS_NEAR_LIMIT_230  (0x8000)
#define SONY_FOCUS_NEAR_LIMIT_185  (0x9000)
#define SONY_FOCUS_NEAR_LIMIT_150  (0xA000)
#define SONY_FOCUS_NEAR_LIMIT_123  (0xB000)
#define SONY_FOCUS_NEAR_LIMIT_100  (0xC000)
#define SONY_FOCUS_NEAR_LIMIT_30   (0xD000)
#define SONY_FOCUS_NEAR_LIMIT_8    (0xE000)
#define SONY_FOCUS_NEAR_LIMIT_1    (0xF000)


typedef struct _VISCA_packet_t
{
    unsigned char bytes[32];
    uint32 length;
} VISCAPacket_t;

/* TITLE STRUCTURE */
typedef struct _VISCA_title
{
  uint8 vposition;  // 00~0A
  uint8 hposition;  // 00~1F
  uint8 color;  // 00:white  01:yellow  02:violet  03:red  04:cyan  05:green  06:blue
  uint8 blink;  // 00:off  01:on
  unsigned char title[20];

} VISCA_title_t;


typedef struct _ZOOM_RATIO {
    uint32 ratio;
    char* desc;
} ZOOM_RATIO_t;

typedef struct _FOCUS_NEAR_LIMIT {
    uint32 limit;
    char* desc;
} FOCUS_NEAR_LIMIT_t;


typedef struct MONITOR_MODE {
    unsigned char mode;
    char* desc;
} MONITOR_MODE_t;

typedef struct SONY_MENU_ITEM {
    int id;
    char* name;

    int (*get_count)();
    int (*get)();
    void (*set)(int);
    char* (*right)();
    void (*action)();   

    uint8 disable;
    uint8 ineffective;
} SONY_MENU_ITEM_t;


typedef struct SONY_MENU {
    // menu desc
    int id;
    char* menu_name;
    int menu_item_count;
    int menu_item_selected;
    //const struct SONY_MENU* parent;
    void (*init)();
    // menu data
    struct SONY_MENU_ITEM menu_item_list[10];
} SONY_MENU_t;

typedef struct SONY_SPLASH {
    // menu desc
    int id;
    char* key;
    char* (*get)();
} SONY_SPLASH_t;


typedef struct CHAR_INDEX {
    unsigned char key;
    unsigned char value;
} CHAR_INDEX_t;

/*

eeprom storage design
version(32)
config_params()

*/
#define SONYLENS_VERSION_ADDR  (0)
#define SONYLENS_VERSION_SIZE  (32)

#define SONYLENS_CONFIG_PARAMS_ADDR  (SONYLENS_VERSION_ADDR + SONYLENS_VERSION_SIZE)
#define SONYLENS_CONFIG_PARAMS_SIZE  (128)

#define SONYLENS_PRIVACY_ZONE_ADDR  (SONYLENS_CONFIG_PARAMS_ADDR + SONYLENS_CONFIG_PARAMS_SIZE)
#define SONYLENS_PRIVACY_ZONE_SIZE  (24)

#define SONYLENS_PRESET_CONF_ADDR  (SONYLENS_PRIVACY_ZONE_ADDR + SONYLENS_PRIVACY_ZONE_SIZE)
#define SONYLENS_PRESET_CONF_SIZE  (100)


/* define menu & menu item id */
#define MENU_MAIN                   (0)
    #define MENU_ITEM_ZOOM_SET          ((MENU_MAIN << 4) + 0)
    #define MENU_ITEM_FOCUS_SET         ((MENU_MAIN << 4) + 1)
    #define MENU_ITEM_EXPOSURE_SET      ((MENU_MAIN << 4) + 2)
    #define MENU_ITEM_WHITE_BALANCE     ((MENU_MAIN << 4) + 3)
    #define MENU_ITEM_ADVANCE           ((MENU_MAIN << 4) + 4)
    #define MENU_ITEM_IMAGE             ((MENU_MAIN << 4) + 5)
    #define MENU_ITEM_GENERAL           ((MENU_MAIN << 4) + 6)
    #define MENU_ITEM_DEFAULT           ((MENU_MAIN << 4) + 7)
    #define MENU_ITEM_EXIT              ((MENU_MAIN << 4) + 8)

#define MENU_ZOOM_SET         (MENU_MAIN + 1)
    #define MENU_ITEM_ZOOM_SPEED          ((MENU_ZOOM_SET << 4) + 0)
    #define MENU_ITEM_ZOOM_DIG            ((MENU_ZOOM_SET << 4) + 1)
    #define MENU_ITEM_ZOOM_OSD            ((MENU_ZOOM_SET << 4) + 2)
    #define MENU_ITEM_ZOOM_DISPLAY        ((MENU_ZOOM_SET << 4) + 3)
    #define MENU_ITEM_ZOOM_BACK           ((MENU_ZOOM_SET << 4) + 4)


#define MENU_FOCUS_SET        (MENU_MAIN + 2)
    #define MENU_ITEM_FOCUS_MODE          ((MENU_FOCUS_SET << 4) + 0)
    #define MENU_ITEM_FOCUS_AF_SENS       ((MENU_FOCUS_SET << 4) + 1)
    #define MENU_ITEM_FOCUS_LIGHT         ((MENU_FOCUS_SET << 4) + 2)
    #define MENU_ITEM_FOCUS_NEAR_LIMIT    ((MENU_FOCUS_SET << 4) + 3)
    #define MENU_ITEM_FOCUS_BACK          ((MENU_FOCUS_SET << 4) + 4)

    
#define MENU_EXPOSURE_SET     (MENU_MAIN + 3)
    #define MENU_ITEM_EXPOSURE_AE_MODE          ((MENU_EXPOSURE_SET << 4) + 0)
    #define MENU_ITEM_EXPOSURE_SHUTTER          ((MENU_EXPOSURE_SET << 4) + 1)
    #define MENU_ITEM_EXPOSURE_IRIS             ((MENU_EXPOSURE_SET << 4) + 2)
    #define MENU_ITEM_EXPOSURE_GAIN             ((MENU_EXPOSURE_SET << 4) + 3)
    #define MENU_ITEM_EXPOSURE_BRI              ((MENU_EXPOSURE_SET << 4) + 4)
    #define MENU_ITEM_EXPOSURE_SLOW_AE          ((MENU_EXPOSURE_SET << 4) + 5)
    #define MENU_ITEM_EXPOSURE_SLOW_SHUTTER     ((MENU_EXPOSURE_SET << 4) + 6)
    #define MENU_ITEM_EXPOSURE_BACK             ((MENU_EXPOSURE_SET << 4) + 7)

    
#define MENU_WHITE_BALANCE    (MENU_MAIN + 4)
    #define MENU_ITEM_WB_MODE              ((MENU_WHITE_BALANCE << 4) + 0)
    #define MENU_ITEM_WB_RED_GAIN          ((MENU_WHITE_BALANCE << 4) + 1)
    #define MENU_ITEM_WB_BLUE_GAIN         ((MENU_WHITE_BALANCE << 4) + 2)
    #define MENU_ITEM_WB_ONE_PUSH          ((MENU_WHITE_BALANCE << 4) + 3)
    #define MENU_ITEM_WB_BACK              ((MENU_WHITE_BALANCE << 4) + 4)

#define MENU_ADVANCE          (MENU_MAIN + 5)
    #define MENU_ITEM_ADVANCE_PRIVACY_SET              ((MENU_ADVANCE << 4) + 0)
    #define MENU_ITEM_ADVANCE_BLC                      ((MENU_ADVANCE << 4) + 1)
    #define MENU_ITEM_ADVANCE_EXPCOMP                  ((MENU_ADVANCE << 4) + 2)
    #define MENU_ITEM_ADVANCE_WDR                      ((MENU_ADVANCE << 4) + 3)
    #define MENU_ITEM_ADVANCE_IRCUT                    ((MENU_ADVANCE << 4) + 4)
    #define MENU_ITEM_ADVANCE_NR_LEVEL                 ((MENU_ADVANCE << 4) + 5)
    #define MENU_ITEM_ADVANCE_STABILIZER               ((MENU_ADVANCE << 4) + 6)
    #define MENU_ITEM_ADVANCE_DEFOG_MODE               ((MENU_ADVANCE << 4) + 7)
    #define MENU_ITEM_ADVANCE_ALARM_OUT                ((MENU_ADVANCE << 4) + 8)
    #define MENU_ITEM_ADVANCE_BACK                     ((MENU_ADVANCE << 4) + 9)
    
#define MENU_IMAGE            (MENU_MAIN + 6)
    #define MENU_ITEM_IMAGE_APERTURE                   ((MENU_IMAGE << 4) + 0)
    #define MENU_ITEM_IMAGE_COLOR_GAIN                 ((MENU_IMAGE << 4) + 1)
    #define MENU_ITEM_IMAGE_COLOR_HUE                  ((MENU_IMAGE << 4) + 2)
    #define MENU_ITEM_IMAGE_CHROMA_SUPPRESS            ((MENU_IMAGE << 4) + 3)
    #define MENU_ITEM_IMAGE_GAMMA                      ((MENU_IMAGE << 4) + 4)
    #define MENU_ITEM_IMAGE_MIRROR                     ((MENU_IMAGE << 4) + 5)
    #define MENU_ITEM_IMAGE_FLIP                       ((MENU_IMAGE << 4) + 6)
    #define MENU_ITEM_IMAGE_FREEZE                     ((MENU_IMAGE << 4) + 7)
    #define MENU_ITEM_IMAGE_PIC_EFFECT                 ((MENU_IMAGE << 4) + 8)
    #define MENU_ITEM_IMAGE_BACK                       ((MENU_IMAGE << 4) + 9)
    
#define MENU_GENERAL          (MENU_MAIN + 7)
    #define MENU_ITEM_GENERAL_CAMERA_ID                   ((MENU_GENERAL << 4) + 0)
    #define MENU_ITEM_GENERAL_FORMAT                      ((MENU_GENERAL << 4) + 1)
    #define MENU_ITEM_GENERAL_LR_HAND                     ((MENU_GENERAL << 4) + 2)
    #define MENU_ITEM_GENERAL_PROTOCOL                    ((MENU_GENERAL << 4) + 3)
    #define MENU_ITEM_GENERAL_BAUDRATE                    ((MENU_GENERAL << 4) + 4)
    #define MENU_ITEM_GENERAL_ADDRESS                     ((MENU_GENERAL << 4) + 5)
    #define MENU_ITEM_GENERAL_PRESET_CONF                 ((MENU_GENERAL << 4) + 6)
    #define MENU_ITEM_GENERAL_AUX                         ((MENU_GENERAL << 4) + 7)
    #define MENU_ITEM_GENERAL_BACK                        ((MENU_GENERAL << 4) + 8)

    
#define MENU_EXIT             (MENU_MAIN + 8)
    #define MENU_ITEM_EXIT_YES                     ((MENU_EXIT << 4) + 0)
    #define MENU_ITEM_EXIT_NO                      ((MENU_EXIT << 4) + 1)
    #define MENU_ITEM_EXIT_BACK                    ((MENU_EXIT << 4) + 2)

#define MENU_PRIVACY_SET             (MENU_MAIN + 9)
    #define MENU_ITEM_PRIVACY_ZONE                      ((MENU_PRIVACY_SET << 4) + 0)
    #define MENU_ITEM_PRIVACY_DISPLAY                   ((MENU_PRIVACY_SET << 4) + 1)
    #define MENU_ITEM_PRIVACY_X                         ((MENU_PRIVACY_SET << 4) + 2)
    #define MENU_ITEM_PRIVACY_Y                         ((MENU_PRIVACY_SET << 4) + 3)
    #define MENU_ITEM_PRIVACY_W                         ((MENU_PRIVACY_SET << 4) + 4)
    #define MENU_ITEM_PRIVACY_H                         ((MENU_PRIVACY_SET << 4) + 5)
    #define MENU_ITEM_PRIVACY_BACK                      ((MENU_PRIVACY_SET << 4) + 6)

#define MENU_CAMERA_ID               (MENU_MAIN + 10)
    #define MENU_ITEM_CAMERA_ID                           ((MENU_CAMERA_ID << 4) + 0)
    #define MENU_ITEM_CAMERA_DISPLAY                      ((MENU_CAMERA_ID << 4) + 1)
    #define MENU_ITEM_CAMERA_BACK                         ((MENU_CAMERA_ID << 4) + 2)

#define MENU_PRESET_CONF             (MENU_MAIN + 11)
    #define MENU_ITEM_PRESET_NO                        ((MENU_PRESET_CONF << 4) + 0)
    #define MENU_ITEM_PRESET_TIME                      ((MENU_PRESET_CONF << 4) + 1)
    #define MENU_ITEM_PRESET_EXIST                     ((MENU_PRESET_CONF << 4) + 2)
    #define MENU_ITEM_PRESET_SET                       ((MENU_PRESET_CONF << 4) + 3)
    #define MENU_ITEM_PRESET_RUN                       ((MENU_PRESET_CONF << 4) + 4)
    #define MENU_ITEM_PRESET_DEL                       ((MENU_PRESET_CONF << 4) + 5)
    #define MENU_ITEM_PRESET_BACK                      ((MENU_PRESET_CONF << 4) + 6)


#pragma pack (1)
typedef struct PRIVACY_ZONE {
    uint8 no;
    uint8 display;
    uint8 x;
    uint8 y;
    uint8 w;
    uint8 h;
} PRIVACY_ZONE_t;

typedef struct PRESET_CONF{
    uint8 no;
    uint8 time;
    uint8 exist;
    uint32 zoom_ratio;
} PRESET_CONF_t;

typedef struct CONFIG_PARAMS {
    uint8 index;
    uint8 is_default;
    struct {
        uint32 zoom_ratio;
        uint8 zoom_speed;  // 5;
        uint8 dig_zoom; //off;
        uint8 osd;  //off;
        uint8 zoom_display;  //off  show the zoom ratio value
    } zoom;

    struct {
        uint8 mode;
        uint8 af_sens;
        uint8 focus_light;
        uint8 near_limit;
        uint8 focus_speed;
        uint32 focus_value;
    } focus;

    struct {
        uint8 ae_mode;
        uint8 shutter;
        uint8 iris;
        uint8 gain;
        uint8 bri;
        uint8 slow_ae;
        uint8 slow_shutter;
    } exposure;

    struct {
        uint8 wb_mode;
        uint8 red_gain;
        uint8 blue_gain;
    } wb;

    struct {
        uint8 privacy_zone_no;
        uint8 blc;
        uint8 expcomp;
        uint8 wdr;
        uint8 ircut;
        uint8 nr_level;
        uint8 stabilizer;
        uint8 defog_mode;
        uint8 alarm_out;
    } advance;

    struct {
        uint8 aperture;
        uint8 color_gain;
        uint8 color_hue;
        uint8 chroma_suppress;
        uint8 gamma;
        uint8 mirror;
        uint8 flip;
        uint8 freeze;
        uint8 pic_effect;
    } image; 

    struct {
        struct {
            int id;
            uint8 display;
        } camera;
        uint8 format;
        uint8 lr_hand;
        uint8 protocol;
        uint8 baudrate;
        uint8 address;
        uint8 preset_conf_no;
        uint8 aux;
    } general;

    PRIVACY_ZONE_t privacy_zone_list[4];
    PRESET_CONF_t preset_conf_list[10];
} CONFIG_PARAMS_t;
#pragma pack ()

void sonylens_init(void);
void sonylens_zoom_init(void);
void sonylens_focus_init(void);
void sonylens_exposure_init(void);
void sonylens_wb_init(void);
void sonylens_advance_init(void);
void sonylens_image_init(void);
void sonylens_general_init(void);
bool sonylens_set_monitor_mode(void);
bool sonylens_wait_ready(void);
bool sonylens_power_on(void);
bool sonylens_power_off(void);


void sonylens_push_event(SONY_EVENT_E event, void* params);


/* internal */
void sonylens_splash_display(void);
void sonylens_splash_clear(void);
void sonylens_menu_on1(void);
void sonylens_menu_off(void);
void sonylens_menu_move_updown1(int step);
void sonylens_menu_move_updown(int step);
void sonylens_menu_move_leftright(int step);
void sonylens_menu_update(int index);
void sonylens_menu_item_update(int index);
void sonylens_menu_action(void);
void sonylens_focus_menu_update(void);
void sonylens_exposure_menu_update(void);
void sonylens_wb_menu_update(void);
void sonylens_privacy_menu_update(void);
void sonylens_preset_conf_menu_update(void);
void sonylens_general_menu_update(void);
void sonylens_set_title(VISCA_title_t* t);
void sonylens_show_title(uint8 lines, uint8 enable);
void sonylens_clear_title(uint8 lines);
void sonylens_monitor_mode_switch(int step);
void sonylens_zoom_ratio_switch(int step);
void sonylens_set_zoom_ratio(int step);
void sonylens_set_focus_near_limit(int step);
void sonylens_focus_near_limit_switch(int step);
void sonylens_set_focus_near(void);
void sonylens_set_focus_far(void);
void sonylens_set_focus_stop(void);
bool sonylens_write_config_params(const CONFIG_PARAMS_t *param);
bool sonylens_read_config_params(CONFIG_PARAMS_t *param);
void sonylens_reset_default_config_params(void);
void sonylens_restart(void);
void sonylens_key_handle_process(void);


/* zoom set */
int sonylens_zoom_speed_get_count(void);
int sonylens_zoom_speed_get(void);
void sonylens_zoom_speed_set(int param);
char* sonylens_zoom_speed_right(void);


int sonylens_dig_zoom_get_count(void);
int sonylens_dig_zoom_get(void);
void sonylens_dig_zoom_set(int param);
char* sonylens_dig_zoom_right(void);

int sonylens_osd_get_count(void);
int sonylens_osd_get(void);
void sonylens_osd_set(int param);
char* sonylens_osd_right(void);

int sonylens_zoom_display_get_count(void);
int sonylens_zoom_display_get(void);
void sonylens_zoom_display_set(int param);
char* sonylens_zoom_display_right(void);




/* focus set */
int sonylens_focus_mode_get_count(void);
int sonylens_focus_mode_get(void);
void sonylens_focus_mode_set(int value);
char* sonylens_focus_mode_right(void);

int sonylens_focus_af_sens_get_count(void);
int sonylens_focus_af_sens_get(void);
void sonylens_focus_af_sens_set(int value);
char* sonylens_focus_af_sens_right(void);

int sonylens_focus_light_get_count(void);
int sonylens_focus_light_get(void);
void sonylens_focus_light_set(int value);
char* sonylens_focus_light_right(void);

int sonylens_focus_near_limit_get_count(void);
int sonylens_focus_near_limit_get(void);
void sonylens_focus_near_limit_set(int value);
char* sonylens_focus_near_limit_right(void);

/* exposure set */

int sonylens_exposure_ae_mode_get_count(void);
int sonylens_exposure_ae_mode_get(void);
void sonylens_exposure_ae_mode_set(int param);
char* sonylens_exposure_ae_mode_right(void);

int sonylens_exposure_shutter_get_count(void);
int sonylens_exposure_shutter_get(void);
void sonylens_exposure_shutter_set(int param);
char* sonylens_exposure_shutter_right(void);

int sonylens_exposure_iris_get_count(void);
int sonylens_exposure_iris_get(void);
void sonylens_exposure_iris_set(int param);
char* sonylens_exposure_iris_right(void);

int sonylens_exposure_gain_get_count(void);
int sonylens_exposure_gain_get(void);
void sonylens_exposure_gain_set(int param);
char* sonylens_exposure_gain_right(void);


int sonylens_exposure_bri_get_count(void);
int sonylens_exposure_bri_get(void);
void sonylens_exposure_bri_set(int param);
char* sonylens_exposure_bri_right(void);

int sonylens_exposure_slow_ae_get_count(void);
int sonylens_exposure_slow_ae_get(void);
void sonylens_exposure_slow_ae_set(int param);
char* sonylens_exposure_slow_ae_right(void);

int sonylens_exposure_slow_shutter_get_count(void);
int sonylens_exposure_slow_shutter_get(void);
void sonylens_exposure_slow_shutter_set(int param);
char* sonylens_exposure_slow_shutter_right(void);


/* white balance*/
int sonylens_wb_mode_get_count(void);
int sonylens_wb_mode_get(void);
void sonylens_wb_mode_set(int param);
char* sonylens_wb_mode_right(void);

int sonylens_wb_red_gain_get_count(void);
int sonylens_wb_red_gain_get(void);
void sonylens_wb_red_gain_set(int param);
char* sonylens_wb_red_gain_right(void);


int sonylens_wb_blue_gain_get_count(void);
int sonylens_wb_blue_gain_get(void);
void sonylens_wb_blue_gain_set(int param);
char* sonylens_wb_blue_gain_right(void);

void sonylens_wb_one_push_action(void);

/* advance */

int sonylens_advance_blc_get_count(void);
int sonylens_advance_blc_get(void);
void sonylens_advance_blc_set(int param);
char* sonylens_advance_blc_right(void);

int sonylens_advance_expcomp_get_count(void);
int sonylens_advance_expcomp_get(void);
void sonylens_advance_expcomp_set(int param);
char* sonylens_advance_expcomp_right(void);

void sonylens_advance_expcomp_on(bool enable);

int sonylens_advance_wdr_get_count(void);
int sonylens_advance_wdr_get(void);
void sonylens_advance_wdr_set(int param);
char* sonylens_advance_wdr_right(void);


int sonylens_advance_ircut_get_count(void);
int sonylens_advance_ircut_get(void);
void sonylens_advance_ircut_set(int param);
char* sonylens_advance_ircut_right(void);

int sonylens_advance_nr_level_get_count(void);
int sonylens_advance_nr_level_get(void);
void sonylens_advance_nr_level_set(int param);
char* sonylens_advance_nr_level_right(void);

int sonylens_advance_stabilizer_get_count(void);
int sonylens_advance_stabilizer_get(void);
void sonylens_advance_stabilizer_set(int param);
char* sonylens_advance_stabilizer_right(void);

int sonylens_advance_defog_mode_get_count(void);
int sonylens_advance_defog_mode_get(void);
void sonylens_advance_defog_mode_set(int param);
char* sonylens_advance_defog_mode_right(void);

int sonylens_advance_alarm_out_get_count(void);
int sonylens_advance_alarm_out_get(void);
void sonylens_advance_alarm_out_set(int param);
char* sonylens_advance_alarm_out_right(void);




/* image */
int sonylens_image_aperture_get_count(void);
int sonylens_image_aperture_get(void);
void sonylens_image_aperture_set(int param);
char* sonylens_image_aperture_right(void);

int sonylens_image_color_gain_get_count(void);
int sonylens_image_color_gain_get(void);
void sonylens_image_color_gain_set(int param);
char* sonylens_image_color_gain_right(void);

int sonylens_image_color_hue_get_count(void);
int sonylens_image_color_hue_get(void);
void sonylens_image_color_hue_set(int param);
char* sonylens_image_color_hue_right(void);


int sonylens_image_chroma_suppress_get_count(void);
int sonylens_image_chroma_suppress_get(void);
void sonylens_image_chroma_suppress_set(int param);
char* sonylens_image_chroma_suppress_right(void);

int sonylens_image_gamma_get_count(void);
int sonylens_image_gamma_get(void);
void sonylens_image_gamma_set(int param);
char* sonylens_image_gamma_right(void);

int sonylens_image_mirror_get_count(void);
int sonylens_image_mirror_get(void);
void sonylens_image_mirror_set(int param);
char* sonylens_image_mirror_right(void);

int sonylens_image_flip_get_count(void);
int sonylens_image_flip_get(void);
void sonylens_image_flip_set(int param);
char* sonylens_image_flip_right(void);

int sonylens_image_freeze_get_count(void);
int sonylens_image_freeze_get(void);
void sonylens_image_freeze_set(int param);
char* sonylens_image_freeze_right(void);

int sonylens_image_pic_effect_get_count(void);
int sonylens_image_pic_effect_get(void);
void sonylens_image_pic_effect_set(int param);
char* sonylens_image_pic_effect_right(void);



/* general */
uint8 sonylens_get_monitor_mode(int index);
int sonylens_general_format_get_count(void);
int sonylens_general_format_get(void);
void sonylens_general_format_set(int param);
char* sonylens_general_format_right(void);

int sonylens_general_lr_hand_get_count(void);
int sonylens_general_lr_hand_get(void);
void sonylens_general_lr_hand_set(int param);
char* sonylens_general_lr_hand_right(void);

int sonylens_general_protocol_get_count(void);
int sonylens_general_protocol_get(void);
void sonylens_general_protocol_set(int param);
char* sonylens_general_protocol_right(void);

int sonylens_general_baudrate_get_count(void);
int sonylens_general_baudrate_get(void);
void sonylens_general_baudrate_set(int param);
char* sonylens_general_baudrate_right(void);

int sonylens_general_address_get_count(void);
int sonylens_general_address_get(void);
void sonylens_general_address_set(int param);
char* sonylens_general_address_right(void);
int sonylens_address_get(void);



int sonylens_general_aux_get_count(void);
int sonylens_general_aux_get(void);
void sonylens_general_aux_set(int param);
char* sonylens_general_aux_right(void);


void sonylens_main_menu_init(void);
void sonylens_zoom_menu_init(void);
void sonylens_focus_menu_init(void);
void sonylens_exposure_menu_init(void);
void sonylens_wb_menu_init(void);
void sonylens_advance_menu_init(void);
void sonylens_image_menu_init(void);
void sonylens_general_menu_init(void);
void sonylens_exit_menu_init(void);
void sonylens_privacy_menu_init(void);
void sonylens_camera_menu_init(void);
void sonylens_preset_menu_init(void);


void sonylens_main_menu_zoom_set_action(void);
void sonylens_main_menu_focus_set_action(void);
void sonylens_main_menu_exposure_set_action(void);
void sonylens_main_menu_wb_set_action(void);
void sonylens_main_menu_advance_set_action(void);
void sonylens_main_menu_image_set_action(void);
void sonylens_main_menu_general_set_action(void);
void sonylens_main_menu_exit_action(void);
void sonylens_advance_menu_privacy_action(void);
void sonylens_general_menu_camera_action(void);
void sonylens_general_menu_preset_action(void);


void sonylens_zoom_menu_back_action(void);
void sonylens_focus_menu_back_action(void);
void sonylens_exposure_menu_back_action(void);
void sonylens_wb_menu_back_action(void);
void sonylens_advance_menu_back_action(void);
void sonylens_image_menu_back_action(void);
void sonylens_general_menu_back_action(void);
void sonylens_privacy_menu_back_action(void);
void sonylens_camera_menu_back_action(void);
void sonylens_preset_menu_back_action(void);




/* default */
int sonylens_default_get_count(void);
int sonylens_default_get(void);
void sonylens_default_set(int param);
void sonylens_default_action(void);
char* sonylens_default_right(void);


/* privacy */
int sonylens_privacy_zone_get_count(void);
int sonylens_privacy_zone_get(void);
void sonylens_privacy_zone_set(int param);
char* sonylens_privacy_zone_right(void);

int sonylens_privacy_display_get_count(void);
int sonylens_privacy_display_get(void);
void sonylens_privacy_display_set(int param);
char* sonylens_privacy_display_right(void);

int sonylens_privacy_x_get_count(void);
int sonylens_privacy_x_get(void);
void sonylens_privacy_x_set(int param);
char* sonylens_privacy_x_right(void);


int sonylens_privacy_y_get_count(void);
int sonylens_privacy_y_get(void);
void sonylens_privacy_y_set(int param);
char* sonylens_privacy_y_right(void);


int sonylens_privacy_w_get_count(void);
int sonylens_privacy_w_get(void);
void sonylens_privacy_w_set(int param);
char* sonylens_privacy_w_right(void);

int sonylens_privacy_h_get_count(void);
int sonylens_privacy_h_get(void);
void sonylens_privacy_h_set(int param);
char* sonylens_privacy_h_right(void);

/* camera id */
int sonylens_camera_id_get_count(void);
int sonylens_camera_id_get(void);
void sonylens_camera_id_set(int param);
char* sonylens_camera_id_right(void);

int sonylens_camera_id_display_get_count(void);
int sonylens_camera_id_display_get(void);
void sonylens_camera_id_display_set(int param);
char* sonylens_camera_id_display_right(void);


/* preset conf */
int sonylens_preset_no_get_count(void);
int sonylens_preset_no_get(void);
void sonylens_preset_no_set(int param);
char* sonylens_preset_no_right(void);

int sonylens_preset_time_get_count(void);
int sonylens_preset_time_get(void);
void sonylens_preset_time_set(int param);
char* sonylens_preset_time_right(void);

int sonylens_preset_exist_get_count(void);
int sonylens_preset_exist_get(void);
void sonylens_preset_exist_set(int param);
char* sonylens_preset_exist_right(void);

void sonylens_preset_menu_set_action(void);
void sonylens_preset_menu_run_action(void);
void sonylens_preset_menu_del_action(void);



/* exit */
void sonylens_menu_exit_yes(void);
void sonylens_menu_exit_no(void);

/* splash */
char* sonylens_splash_get_version(void);
char* sonylens_splash_get_format(void);
char* sonylens_splash_get_protocol(void);
char* sonylens_splash_get_baudrate(void);
char* sonylens_splash_get_address(void);

/*iris api*/
#if 0
void sonylens_set_iris_close(void);
void sonylens_set_iris_open(void);
#endif
/* zoom api */
void sonylens_set_zoom_wide(void);
void sonylens_set_zoom_tele(void);
void sonylens_set_zoom_stop(void);
void sonylens_zoom_ratio_display(char* title);
void sonylens_zoom_ratio_clear(void);
bool sonylens_set_zoom_value(uint32 zoom_value);
bool sonylens_get_zoom_value(uint32* zoom_value);
int sonylens_get_zoom_index(uint32 zoom_value);
char* sonylens_get_zoom_desc(int index);
void sonylens_zoom_process(void);

/*manual mode and automatic mode  */
typedef struct MODE_MANAGER {
    uint8 exposure_mode;
    uint8 wb_mode;

    uint8 iris;
    uint8 expcomp;

    uint8 current_iris;
    uint8 current_expcomp;
} MODE_MANAGER_t;

void sonylens_set_auto_mode(bool enable);
bool sonylens_get_auto_mode(void);
void sonylens_set_bright_plus(void);
void sonylens_set_bright_minus(void);
void sonylens_set_exposure_gain_plus(void);


void sonylens_key_wb_mode_set(int param);
void sonylens_key_exposure_ae_mode_set(int param);
void sonylens_key_advance_expcomp_on(bool e);
void sonylens_key_advance_expcomp_set(int param);
void sonylens_key_exposure_iris_set(int param);
void sonylens_key_exposure_shutter_set(int param);
void sonylens_key_image_freeze_set(int param);
void sonylens_key_image_flip_set(int param);
void sonylens_key_exposure_gain_set(int param);


void sonylens_tips_display(char * title);
void sonylens_tips_clear(void);
void sonylens_tips_display_process(void);
void sonylens_menu_display_process(void);

/* remote control apis */
BOOL sonylens_remote_is_menu_on(void);
void sonylens_remote_navi_ok(void);
void sonylens_remote_navi_up(void);
void sonylens_remote_navi_down(void);
void sonylens_remote_navi_left(void);
void sonylens_remote_navi_right(void);

void sonylens_control_f1(uint16 data);
void sonylens_control_f2(void);
void sonylens_control_f3(void);
void sonylens_control_f4(void);
void sonylens_control_f5(void);
void sonylens_control_f6(void);
void sonylens_control_f7(void);
void sonylens_control_f8(void);
void sonylens_control_f9(void);
void sonylens_control_f10(void);
void sonylens_control_f11(void);
void sonylens_control_f12(void);
bool sonylens_is_state_ok(void);


void sonylens_zoom_set_wide_limit(unsigned char value);
void sonylens_zoom_set_tele_limit(unsigned char value);

void sonylens_task(void);

#endif
