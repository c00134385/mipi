
/*----------------------------------------------------------------------------*/
/* sonylens.c                                                                   */
/* 描述:sony 变倍机型通讯程序                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2017/10/17 yihuagang建立 V0.1                                                  */
/* 2017/10/17   */
/* 2017/10/17 yihuagang  v0.3                                                      */
/*            */
/*  */
/*                     */
/*                               */
/*   */
/*                      */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/* include */
#include "includes.h"

/**********************/
/* Register map */
/**********************/

#define SONYLENS_MONITOR_MODE_ADDR (0x72)
#define SONYLENS_WIDE_LIMIT_ADDR   (0x50)
#define SONYLENS_TELE_LIMIT_ADDR   (0x51)


/**********************/
/* Message formatting */
/**********************/

#define VISCA_COMMAND                    0x01
#define VISCA_INQUIRY                    0x09
#define VISCA_TERMINATOR                 0xFF

#define VISCA_CATEGORY_INTERFACE         0x00
#define VISCA_CATEGORY_CAMERA1           0x04
#define VISCA_CATEGORY_PAN_TILTER        0x06
#define VISCA_CATEGORY_CAMERA2           0x07


#define SONY_CAM_TITLE_LINES_UP (10)
#define SONY_CAM_TITLE_CHARACTERS_UP (20)


/**********************/
/* Common constant */
/**********************/
static unsigned char reply_msg[] = {0x90, 0x41, 0xff};
static unsigned char completion_msg[] = {0x90, 0x51, 0xff};
static unsigned char inq_result_msg[] = {0x90, 0x50};
static unsigned char network_change[] = {0x90,0x38,0xff};
static unsigned char command_not_executable[] = {0x90,0x61,0x41,0xff};

static const CHAR_INDEX_t alphabet = {'A', 0x00}; // A,B,C,.....,Z
static const CHAR_INDEX_t number = {'1', 0x1E};  // 1,2,3,4,5,6,7,8,9,0
static const CHAR_INDEX_t space = {' ', 0x1B};
static const CHAR_INDEX_t stride = {'-', 0x4F};
static const CHAR_INDEX_t question = {'?', 0x1C};
static const CHAR_INDEX_t slash = {'/', 0x4E};
static const CHAR_INDEX_t dot = {'.', 0x4C};

const uint32 focus_near_limit_range[] = {0x1000, 0xF000};
uint32 current_focus_near_limit = 0;



uint32 current_focus_near_limit_index = 12; // 30cm
const FOCUS_NEAR_LIMIT_t focus_near_limit_list[] = {
    {SONY_FOCUS_NEAR_LIMIT_OVER, "OVER"},
    {SONY_FOCUS_NEAR_LIMIT_2500, "25M"},
    {SONY_FOCUS_NEAR_LIMIT_1100, "11M"},
    {SONY_FOCUS_NEAR_LIMIT_700, "7M"},
    {SONY_FOCUS_NEAR_LIMIT_490, "4.9M"},
    {SONY_FOCUS_NEAR_LIMIT_370, "3.7M"},
    {SONY_FOCUS_NEAR_LIMIT_290, "2.9M"},
    {SONY_FOCUS_NEAR_LIMIT_230, "2.3M"},
    {SONY_FOCUS_NEAR_LIMIT_185, "1.85M"},
    {SONY_FOCUS_NEAR_LIMIT_150, "1.5M"},
    {SONY_FOCUS_NEAR_LIMIT_123, "1.23M"},
    {SONY_FOCUS_NEAR_LIMIT_100, "1M"},
    {SONY_FOCUS_NEAR_LIMIT_30, "30CM"},
    {SONY_FOCUS_NEAR_LIMIT_8, "8CM"},
    {SONY_FOCUS_NEAR_LIMIT_1, "1CM"},
};


uint32 sonylens_current_monitor_mode_index = 0;
const MONITOR_MODE_t monitor_mode_list[] = {
    {SONY_MONITOR_1080i_59_94, "1080I/59.94"},
    {SONY_MONITOR_1080i_60,    "1080I/60"},
    {SONY_MONITOR_1080i_50,    "1080I/50"},
    {SONY_MONITOR_1080p_29_97, "1080P/29.97"},
    {SONY_MONITOR_1080p_30,    "1080P/30"},
    {SONY_MONITOR_1080p_25,    "1080P/25"},
    {SONY_MONITOR_720p_59_94,  "720P/59.94"},
    {SONY_MONITOR_720p_60,     "720P/60"},
    {SONY_MONITOR_720p_50,     "720P/50"},
    {SONY_MONITOR_720p_29_97,  "720P/29.97"},
    {SONY_MONITOR_720p_30,     "720P/30"},
    {SONY_MONITOR_720p_25,     "720P/25"},
    {SONY_MONITOR_1080p_59_94, "1080P/59.94"},
    {SONY_MONITOR_1080p_50,    "1080P/50"},
    {SONY_MONITOR_1080p_60,    "1080P/60"}
};

const uint32 zoom_ratio_range[] = {0x0000, 0x4000};
#ifdef SONYLENS_10X_SUPPORT
const ZOOM_RATIO_t zoom_ratio_list[] = {
    {SONY_ZOOM_RATIO_1X, "1X"},
    {SONY_ZOOM_RATIO_2X, "2X"},
    {SONY_ZOOM_RATIO_3X, "3X"},
    {SONY_ZOOM_RATIO_4X, "4X"},
    {SONY_ZOOM_RATIO_5X, "5X"},
    {SONY_ZOOM_RATIO_6X, "6X"},
    {SONY_ZOOM_RATIO_7X, "7X"},
    {SONY_ZOOM_RATIO_8X, "8X"},
    {SONY_ZOOM_RATIO_9X, "9X"},
    {SONY_ZOOM_RATIO_10X, "10X"},
};
#else
const ZOOM_RATIO_t zoom_ratio_list[] = {
    {SONY_ZOOM_RATIO_1X, "1X"},
    {SONY_ZOOM_RATIO_2X, "2X"},
    {SONY_ZOOM_RATIO_3X, "3X"},
    {SONY_ZOOM_RATIO_4X, "4X"},
    {SONY_ZOOM_RATIO_5X, "5X"},
    {SONY_ZOOM_RATIO_6X, "6X"},
    {SONY_ZOOM_RATIO_7X, "7X"},
    {SONY_ZOOM_RATIO_8X, "8X"},
    {SONY_ZOOM_RATIO_9X, "9X"},
    {SONY_ZOOM_RATIO_10X, "10X"},
    {SONY_ZOOM_RATIO_11X, "11X"},
    {SONY_ZOOM_RATIO_12X, "12X"},
    {SONY_ZOOM_RATIO_13X, "13X"},
    {SONY_ZOOM_RATIO_14X, "14X"},
    {SONY_ZOOM_RATIO_15X, "15X"},
    {SONY_ZOOM_RATIO_16X, "16X"},
    {SONY_ZOOM_RATIO_17X, "17X"},
    {SONY_ZOOM_RATIO_18X, "18X"},
    {SONY_ZOOM_RATIO_19X, "19X"},
    {SONY_ZOOM_RATIO_20X, "20X"},
};
#endif

static int title_line_index = 0;
static unsigned char* title_line_content;
static bool title_line_on = false;
unsigned long title_line_on_time;
static bool format_display = false;
static bool zoom_ratio_display = false;
static bool splash_display = false;
static bool zoom_ratio_changed = true;



static unsigned long format_display_time;
static unsigned long splash_display_time;



/* external */
extern char sonylens_rx_buffer[],sonylens_tx_buffer[];


/*state machine*/
static uint16_t sonylens_taskstate = SONY_12V_ON;
static uint16_t sonylens_taskcount = 0;
static SONY_CMD_STATE_E sonylens_cmd_state = SONY_CMD_IDLE;
unsigned long sonylens_power_on_time = 0;
static bool power_is_on = false;
static bool sonylens_state_ready = false;
static bool monitor_mode_set_done = false;
static bool zoom_ratio_set_done = false;
static bool splash_set_done = false;
static bool titles_clear_done = false;
static bool monitor_mode_changed = false;


/* private */
unsigned char sonylens_camera_id = 1;
static unsigned long rec_systick_mark_sonylens = 0;
unsigned long sonylens_task_t_sec = 0;  ////单位秒
unsigned char sonylens_power_state;
uint32 model_code;
uint32 rom_version;
uint32 socket_num;
uint32 sonylens_current_zoom_ratio = 0; // 1x



#define SONYLENS_REPLY_MSG_BUFF_LEN 100
unsigned char sonylens_reply_msg[SONYLENS_REPLY_MSG_BUFF_LEN];        //NMEA-0183单帧长度最长210 byte
static int sonylens_reply_msg_len = 0;
static SONY_EVENT_E event_queue[16];

/*define the state machine of command interval*/
#define SONYLENS_SEND_MSG_WAIT_MS  (300)
unsigned long sonylens_send_msg_time = 0;  ////单位秒
#define SONYLENS_10_S_DELAY  (10)
unsigned long sonylens_task1_completion_time;

const char version[SONYLENS_VERSION_SIZE] = "JY1702";
const CONFIG_PARAMS_t default_config_params = {
    0, 1,
    {// zoom
        0, 
        3, // zoom speed 
        0, 
        0, 
        0
    }, 

    {// focus
        0, // mode 
        0, // af sens
        0, // focus light
        12, // near limit
        2, // speed
    },

    {// exposure
        0, //ae_mode;
        10,
        6,
        0, //gain;
        0,
        0, //slow_ae;
        0, //slow_shutter;
    },

    {// wb
        0, //wb_mode;
        195, //red_gain;
        185, //blue_gain;
    },

    {// advance
        0, //{0, 0, 0, 0, 0, 0}, //privacy zone
        0, // blc
        7, // expcomp
        0, // wdr
        0, // ircut
        0, // nr_level
        0, // stabilizer
        0, // defog_mode
        0, // alarm_out
    },

    {// image
        5, // aperture;
        4, // color_gain;
        7, // color_hue;
        2, // chroma_suppress;
        16, // gamma;
        0, // mirror;
        0, // flip;
        0, // freeze;
        0, // pic_effect;
    },
    
    {// general
        {0, 0}, // camera{id, display}
        7, //format
        0, //lr_hand
        0, //protocol
        2, //baudrate
        0, //address
        0, //{0, 0, 0,},// preset conf
        0, //aux
    },
    
    {// privacy zone list
        {0, 0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0},
        {2, 0, 0, 0, 0, 0},
        {3, 0, 0, 0, 0, 0},
    },
    
    {// preset conf list
        {0, 0, 0},
        {1, 0, 0},
        {2, 0, 0},
        {3, 0, 0},
        {4, 0, 0},
        {5, 0, 0},
        {6, 0, 0},
        {7, 0, 0},
        {8, 0, 0},
        {9, 0, 0},
    },
};

CONFIG_PARAMS_t config_params;
MODE_MANAGER_t mode_manager;
uint8 key_freeze;
uint8 key_flip;
uint8 key_exp_gain;

char current_version[SONYLENS_VERSION_SIZE];

SONY_SPLASH_t splash[] = {
    {
        0,
        "VERSION",
        sonylens_splash_get_version,
    },
    {
        1,
        "FORMAT",
        sonylens_splash_get_format,
    },
    {
        2,
        "PROTOCOL",
        sonylens_splash_get_protocol,
    },
    {
        3,
        "BAUDRATE",
        sonylens_splash_get_baudrate,
    },
    {
        4,
        "ADDRESS",
        sonylens_splash_get_address,
    },  
};


bool need_clear_menu = false;
bool menu_display = false;
static unsigned long menu_display_time = 0;
SONY_MENU_t current_menu;

const SONY_MENU_t main_menu = {
    MENU_MAIN, "MENU", 9, 0, sonylens_main_menu_init,
    {
        {
            MENU_ITEM_ZOOM_SET,
            "ZOOM SET", 
            NULL, 
            NULL, 
            NULL,
            NULL,
            sonylens_main_menu_zoom_set_action,
        },
        {
            MENU_ITEM_FOCUS_SET,
            "FOCUS SET", 
            NULL, 
            NULL, 
            NULL, 
            NULL, 
            sonylens_main_menu_focus_set_action,
        },
        {
            MENU_ITEM_EXPOSURE_SET,
            "EXPOSURE SET", 
            NULL, 
            NULL, 
            NULL, 
            NULL, 
            sonylens_main_menu_exposure_set_action,
        },
        {
            MENU_ITEM_WHITE_BALANCE,
            "WHITE BALANCE", 
            NULL, 
            NULL, 
            NULL, 
            NULL, 
            sonylens_main_menu_wb_set_action,
        },
        {
            MENU_ITEM_ADVANCE,
            "ADVANCE", 
            NULL, 
            NULL, 
            NULL, 
            NULL, 
            sonylens_main_menu_advance_set_action,
        },
        {
            MENU_ITEM_IMAGE,
            "IMAGE", 
            NULL, 
            NULL, 
            NULL, 
            NULL, 
            sonylens_main_menu_image_set_action,
        },
        {
            MENU_ITEM_GENERAL,
            "GENERAL", 
            NULL, 
            NULL, 
            NULL, 
            NULL, 
            sonylens_main_menu_general_set_action,
        },
        {
            MENU_ITEM_DEFAULT,
            "DEFAULT", 
            NULL,
            sonylens_default_get,
            sonylens_default_set,
            sonylens_default_right,
            sonylens_default_action,
        },
        {
            MENU_ITEM_EXIT,
            "EXIT", 
            NULL, 
            NULL, 
            NULL, 
            NULL, 
            sonylens_main_menu_exit_action,
        },
    },
};


const SONY_MENU_t zoom_menu = {
    MENU_ZOOM_SET, "ZOOM SET", 5, 0, sonylens_zoom_menu_init,
    {
        {
            MENU_ITEM_ZOOM_SPEED,
            "ZOOM SPEED", 
            sonylens_zoom_speed_get_count, 
            sonylens_zoom_speed_get, 
            sonylens_zoom_speed_set, 
            sonylens_zoom_speed_right,
            NULL, 
        },

        {
            MENU_ITEM_ZOOM_DIG,
            "DIG ZOOM", 
            sonylens_dig_zoom_get_count, 
            sonylens_dig_zoom_get, 
            sonylens_dig_zoom_set, 
            sonylens_dig_zoom_right, 
            NULL,
        },

        {
            MENU_ITEM_ZOOM_OSD,
            "OSD", 
            sonylens_osd_get_count, 
            sonylens_osd_get, 
            sonylens_osd_set, 
            sonylens_osd_right, 
            NULL,
        },
        {
            MENU_ITEM_ZOOM_DISPLAY,
            "ZOOM DISPLAY", 
            sonylens_zoom_display_get_count, 
            sonylens_zoom_display_get, 
            sonylens_zoom_display_set, 
            sonylens_zoom_display_right, 
            NULL,
        },
        {
            MENU_ITEM_ZOOM_BACK,
            "BACK", 
            NULL, 
            NULL, 
            NULL,
            NULL,
            sonylens_zoom_menu_back_action, 
        },
    },
};


const SONY_MENU_t focus_menu = {
    MENU_FOCUS_SET, "FOCUS SET", 5, 0, sonylens_focus_menu_init,
    {
        {
            MENU_ITEM_FOCUS_MODE,
            "MODE", 
            sonylens_focus_mode_get_count, 
            sonylens_focus_mode_get, 
            sonylens_focus_mode_set,
            sonylens_focus_mode_right,
            NULL
        },

        {
            MENU_ITEM_FOCUS_AF_SENS,
            "AF SENS", 
            sonylens_focus_af_sens_get_count, 
            sonylens_focus_af_sens_get, 
            sonylens_focus_af_sens_set,
            sonylens_focus_af_sens_right,
            NULL
        },

        {
            MENU_ITEM_FOCUS_LIGHT,
            "FOCUS LIGHT", 
            sonylens_focus_light_get_count, 
            sonylens_focus_light_get, 
            sonylens_focus_light_set,
            sonylens_focus_light_right,
            NULL
        },

        {
            MENU_ITEM_FOCUS_NEAR_LIMIT,
            "NEAR LIMIT", 
            sonylens_focus_near_limit_get_count, 
            sonylens_focus_near_limit_get, 
            sonylens_focus_near_limit_set,
            sonylens_focus_near_limit_right,
            NULL
        },
        {
            MENU_ITEM_FOCUS_BACK,
            "BACK", 
            NULL, 
            NULL, 
            NULL,
            NULL,
            sonylens_focus_menu_back_action,
        },
    },
};


const SONY_MENU_t exposure_menu = {
    MENU_EXPOSURE_SET, "EXPOSURE SET", 8, 0, sonylens_exposure_menu_init,
    {
        {
            MENU_ITEM_EXPOSURE_AE_MODE,
            "AE MODE", 
            sonylens_exposure_ae_mode_get_count, 
            sonylens_exposure_ae_mode_get, 
            sonylens_exposure_ae_mode_set,
            sonylens_exposure_ae_mode_right,
            NULL
        },

        {
            MENU_ITEM_EXPOSURE_SHUTTER,
            "SHUTTER", 
            sonylens_exposure_shutter_get_count, 
            sonylens_exposure_shutter_get, 
            sonylens_exposure_shutter_set,
            sonylens_exposure_shutter_right,
            NULL
        },

        {
            MENU_ITEM_EXPOSURE_IRIS,
            "IRIS", 
            sonylens_exposure_iris_get_count, 
            sonylens_exposure_iris_get, 
            sonylens_exposure_iris_set,
            sonylens_exposure_iris_right,
            NULL
        },

        {
            MENU_ITEM_EXPOSURE_GAIN,
            "GAIN", 
            sonylens_exposure_gain_get_count, 
            sonylens_exposure_gain_get, 
            sonylens_exposure_gain_set,
            sonylens_exposure_gain_right,
            NULL
        },
        {
            MENU_ITEM_EXPOSURE_BRI,
            "BRI", 
            sonylens_exposure_bri_get_count, 
            sonylens_exposure_bri_get, 
            sonylens_exposure_bri_set,
            sonylens_exposure_bri_right,
            NULL,
        },
        {
            MENU_ITEM_EXPOSURE_SLOW_AE,
            "SLOW AE", 
            sonylens_exposure_slow_ae_get_count, 
            sonylens_exposure_slow_ae_get, 
            sonylens_exposure_slow_ae_set,
            sonylens_exposure_slow_ae_right,
            NULL,
        },
        {
            MENU_ITEM_EXPOSURE_SLOW_SHUTTER,
            "SLOW SHUTTER", 
            sonylens_exposure_slow_shutter_get_count, 
            sonylens_exposure_slow_shutter_get, 
            sonylens_exposure_slow_shutter_set,
            sonylens_exposure_slow_shutter_right,
            NULL,
        },
        {
            MENU_ITEM_EXPOSURE_BACK,
            "BACK", 
            NULL, 
            NULL, 
            NULL,
            NULL,
            sonylens_exposure_menu_back_action,
        },
    },
};


const SONY_MENU_t wb_menu = {
    MENU_WHITE_BALANCE, "WHITE BALANCE", 5, 0, sonylens_wb_menu_init,
    {
        {
            MENU_ITEM_WB_MODE,
            "WB MODE", 
            sonylens_wb_mode_get_count, 
            sonylens_wb_mode_get, 
            sonylens_wb_mode_set,
            sonylens_wb_mode_right, 
            NULL,
        },
        {
            MENU_ITEM_WB_RED_GAIN,
            "RED GAIN", 
            sonylens_wb_red_gain_get_count, 
            sonylens_wb_red_gain_get, 
            sonylens_wb_red_gain_set,
            sonylens_wb_red_gain_right, 
            NULL,
        },
        {
            MENU_ITEM_WB_BLUE_GAIN,
            "BLUE GAIN", 
            sonylens_wb_blue_gain_get_count, 
            sonylens_wb_blue_gain_get, 
            sonylens_wb_blue_gain_set,
            sonylens_wb_blue_gain_right, 
            NULL,
        },
        {
            MENU_ITEM_WB_ONE_PUSH,
            "ONE PUSH", 
            NULL, 
            NULL, 
            NULL,
            NULL, 
            sonylens_wb_one_push_action,
        },
        {
            MENU_ITEM_WB_BACK,
            "BACK", 
            NULL, 
            NULL, 
            NULL,
            NULL,
            sonylens_wb_menu_back_action,
        },
    },
};

const SONY_MENU_t advance_menu = {
    MENU_ADVANCE, "ADVANCE", 10, 0, sonylens_advance_menu_init,
    {
        {
            MENU_ITEM_ADVANCE_PRIVACY_SET,
            "PRIVACY SET...", 
            NULL, 
            NULL, 
            NULL,
            NULL, 
            sonylens_advance_menu_privacy_action,
        },
        {
            MENU_ITEM_ADVANCE_BLC,
            "BLC", 
            sonylens_advance_blc_get_count, 
            sonylens_advance_blc_get, 
            sonylens_advance_blc_set,
            sonylens_advance_blc_right, 
            NULL,
        },
        {
            MENU_ITEM_ADVANCE_EXPCOMP,
            "EXPCOMP", 
            sonylens_advance_expcomp_get_count, 
            sonylens_advance_expcomp_get, 
            sonylens_advance_expcomp_set,
            sonylens_advance_expcomp_right, 
            NULL,
        },
        {
            MENU_ITEM_ADVANCE_WDR,
            "WDR", 
            sonylens_advance_wdr_get_count, 
            sonylens_advance_wdr_get, 
            sonylens_advance_wdr_set,
            sonylens_advance_wdr_right, 
            NULL,
        },
        {
            MENU_ITEM_ADVANCE_IRCUT,
            "IRCUT", 
            sonylens_advance_ircut_get_count, 
            sonylens_advance_ircut_get, 
            sonylens_advance_ircut_set,
            sonylens_advance_ircut_right, 
            NULL,
        },
        {
            MENU_ITEM_ADVANCE_NR_LEVEL,
            "NR LEVEL", 
            sonylens_advance_nr_level_get_count, 
            sonylens_advance_nr_level_get, 
            sonylens_advance_nr_level_set,
            sonylens_advance_nr_level_right, 
            NULL,
        },
        {
            MENU_ITEM_ADVANCE_STABILIZER,
            "STABILIZER", 
            sonylens_advance_stabilizer_get_count, 
            sonylens_advance_stabilizer_get, 
            sonylens_advance_stabilizer_set,
            sonylens_advance_stabilizer_right, 
            NULL,
        },
        {
            MENU_ITEM_ADVANCE_DEFOG_MODE,
            "DEFOG MODE", 
            sonylens_advance_defog_mode_get_count, 
            sonylens_advance_defog_mode_get, 
            sonylens_advance_defog_mode_set,
            sonylens_advance_defog_mode_right, 
            NULL,
        },
        {
            MENU_ITEM_ADVANCE_ALARM_OUT,
            "ALARM OUT", 
            sonylens_advance_alarm_out_get_count, 
            sonylens_advance_alarm_out_get, 
            sonylens_advance_alarm_out_set,
            sonylens_advance_alarm_out_right, 
            NULL,
        },
        {
            MENU_ITEM_ADVANCE_BACK,
            "BACK", 
            NULL, 
            NULL,
            NULL,
            NULL,
            sonylens_advance_menu_back_action,
        },
    },
};

const SONY_MENU_t image_menu = {
    MENU_IMAGE, "IMAGE", 10, 0, sonylens_image_menu_init,
    {
        {
            MENU_ITEM_IMAGE_APERTURE,
            "APERTURE", 
            sonylens_image_aperture_get_count, 
            sonylens_image_aperture_get, 
            sonylens_image_aperture_set,
            sonylens_image_aperture_right, 
            NULL,
        },
        {
            MENU_ITEM_IMAGE_COLOR_GAIN,
            "COLOR GAIN", 
            sonylens_image_color_gain_get_count, 
            sonylens_image_color_gain_get, 
            sonylens_image_color_gain_set,
            sonylens_image_color_gain_right, 
            NULL,
        },
        {
            MENU_ITEM_IMAGE_COLOR_HUE,
            "COLOR HUE", 
            sonylens_image_color_hue_get_count, 
            sonylens_image_color_hue_get, 
            sonylens_image_color_hue_set,
            sonylens_image_color_hue_right, 
            NULL,
        },
        {
            MENU_ITEM_IMAGE_CHROMA_SUPPRESS,
            "CHROMA SUPPRESS", 
            sonylens_image_chroma_suppress_get_count, 
            sonylens_image_chroma_suppress_get, 
            sonylens_image_chroma_suppress_set,
            sonylens_image_chroma_suppress_right, 
            NULL,
        },
        {
            MENU_ITEM_IMAGE_GAMMA,
            "GAMMA", 
            sonylens_image_gamma_get_count, 
            sonylens_image_gamma_get, 
            sonylens_image_gamma_set,
            sonylens_image_gamma_right, 
            NULL,
        },
        {
            MENU_ITEM_IMAGE_MIRROR,
            "MIRROR", 
            sonylens_image_mirror_get_count, 
            sonylens_image_mirror_get, 
            sonylens_image_mirror_set,
            sonylens_image_mirror_right, 
            NULL,
        },
        {
            MENU_ITEM_IMAGE_FLIP,
            "FLIP", 
            sonylens_image_flip_get_count, 
            sonylens_image_flip_get, 
            sonylens_image_flip_set,
            sonylens_image_flip_right, 
            NULL,
        },
        {
            MENU_ITEM_IMAGE_FREEZE,
            "FREEZE", 
            sonylens_image_freeze_get_count, 
            sonylens_image_freeze_get, 
            sonylens_image_freeze_set,
            sonylens_image_freeze_right, 
            NULL,
        },
        {
            MENU_ITEM_IMAGE_PIC_EFFECT,
            "PIC EFFECT", 
            sonylens_image_pic_effect_get_count, 
            sonylens_image_pic_effect_get, 
            sonylens_image_pic_effect_set,
            sonylens_image_pic_effect_right, 
            NULL,
        },
        {
            MENU_ITEM_IMAGE_BACK,
            "BACK", 
            NULL,
            NULL,
            NULL,
            NULL,
            sonylens_image_menu_back_action,
        },
    },
};

const SONY_MENU_t general_menu = {
    MENU_GENERAL, "GENERAL", 9, 0, sonylens_general_menu_init,
    {
        {
            MENU_ITEM_GENERAL_CAMERA_ID,
            "CAMERA ID...", 
            NULL, 
            NULL, 
            NULL,
            NULL, 
            sonylens_general_menu_camera_action,
        },
        {
            MENU_ITEM_GENERAL_FORMAT,
            "FORMAT", 
            sonylens_general_format_get_count, 
            sonylens_general_format_get, 
            sonylens_general_format_set,
            sonylens_general_format_right, 
            NULL,
        },
        {
            MENU_ITEM_GENERAL_LR_HAND,
            "L/R HAND", 
            sonylens_general_lr_hand_get_count, 
            sonylens_general_lr_hand_get, 
            sonylens_general_lr_hand_set,
            sonylens_general_lr_hand_right, 
            NULL,
        },
        {
            MENU_ITEM_GENERAL_PROTOCOL,
            "PROTOCOL", 
            sonylens_general_protocol_get_count, 
            sonylens_general_protocol_get, 
            sonylens_general_protocol_set,
            sonylens_general_protocol_right, 
            NULL,
        },
        {
            MENU_ITEM_GENERAL_BAUDRATE,
            "BAUDRATE", 
            sonylens_general_baudrate_get_count, 
            sonylens_general_baudrate_get, 
            sonylens_general_baudrate_set,
            sonylens_general_baudrate_right, 
            NULL,
        },
        {
            MENU_ITEM_GENERAL_ADDRESS,
            "ADDRESS", 
            sonylens_general_address_get_count, 
            sonylens_general_address_get, 
            sonylens_general_address_set,
            sonylens_general_address_right, 
            NULL,
        },
        {
            MENU_ITEM_GENERAL_PRESET_CONF,
            "PRESET CONF...", 
            NULL, 
            NULL, 
            NULL,
            NULL, 
            sonylens_general_menu_preset_action,
        },
        {
            MENU_ITEM_GENERAL_AUX,
            "AUX", 
            sonylens_general_aux_get_count, 
            sonylens_general_aux_get, 
            sonylens_general_aux_set,
            sonylens_general_aux_right, 
            NULL,
        },
        {
            MENU_ITEM_GENERAL_BACK,
            "BACK", 
            NULL,
            NULL,
            NULL,
            NULL,
            sonylens_general_menu_back_action,
        },
    },
};

const SONY_MENU_t exit_menu = {
    MENU_EXIT, "SAVE AND EXIT?", 2, 0, sonylens_exit_menu_init,
    {
        {
            MENU_ITEM_EXIT_YES,
            "YES", 
            NULL, 
            NULL, 
            NULL,
            NULL, 
            sonylens_menu_exit_yes,
        },
        {
            MENU_ITEM_EXIT_NO,
            "NO", 
            NULL, 
            NULL, 
            NULL,
            NULL, 
            sonylens_menu_exit_no,
        },
    },
};

const SONY_MENU_t privacy_set_menu = {
    MENU_PRIVACY_SET, "PRIVACY SET", 7, 0, sonylens_privacy_menu_init,
    {
        {
            MENU_ITEM_PRIVACY_ZONE,
            "ZONE", 
            sonylens_privacy_zone_get_count, 
            sonylens_privacy_zone_get, 
            sonylens_privacy_zone_set,
            sonylens_privacy_zone_right, 
            NULL,
        },
        {
            MENU_ITEM_PRIVACY_DISPLAY,
            "DISPLAY", 
            sonylens_privacy_display_get_count, 
            sonylens_privacy_display_get, 
            sonylens_privacy_display_set,
            sonylens_privacy_display_right, 
            NULL,
        },
        {
            MENU_ITEM_PRIVACY_X,
            "X", 
            sonylens_privacy_x_get_count, 
            sonylens_privacy_x_get, 
            sonylens_privacy_x_set,
            sonylens_privacy_x_right, 
            NULL,
        },
        {
            MENU_ITEM_PRIVACY_Y,
            "Y", 
            sonylens_privacy_y_get_count, 
            sonylens_privacy_y_get, 
            sonylens_privacy_y_set,
            sonylens_privacy_y_right, 
            NULL,
        },
        {
            MENU_ITEM_PRIVACY_W,
            "W", 
            sonylens_privacy_w_get_count, 
            sonylens_privacy_w_get, 
            sonylens_privacy_w_set,
            sonylens_privacy_w_right, 
            NULL,
        },
        {
            MENU_ITEM_PRIVACY_H,
            "H", 
            sonylens_privacy_h_get_count, 
            sonylens_privacy_h_get, 
            sonylens_privacy_h_set,
            sonylens_privacy_h_right, 
            NULL,
        },        
        {
            MENU_ITEM_PRIVACY_BACK,
            "BACK", 
            NULL,
            NULL,
            NULL,
            NULL,
            sonylens_privacy_menu_back_action,
        },
    },
};

const SONY_MENU_t camera_id_menu = {
    MENU_CAMERA_ID, "CAMERA ID", 3, 0, sonylens_camera_menu_init,
    {
        {
            MENU_ITEM_CAMERA_ID,
            "ID", 
            sonylens_camera_id_get_count, 
            sonylens_camera_id_get, 
            sonylens_camera_id_set,
            sonylens_camera_id_right, 
            NULL,
        },
        {
            MENU_ITEM_CAMERA_DISPLAY,
            "DISPLAY", 
            sonylens_camera_id_display_get_count, 
            sonylens_camera_id_display_get, 
            sonylens_camera_id_display_set,
            sonylens_camera_id_display_right, 
            NULL,
        },      
        {
            MENU_ITEM_CAMERA_BACK,
            "BACK", 
            NULL,
            NULL,
            NULL,
            NULL,
            sonylens_camera_menu_back_action,
        },
    },
};

const SONY_MENU_t preset_conf_menu = {
    MENU_PRESET_CONF, "PRESET CONF", 7, 0, sonylens_preset_menu_init,
    {
        {
            MENU_ITEM_PRESET_NO,
            "NO.", 
            sonylens_preset_no_get_count, 
            sonylens_preset_no_get, 
            sonylens_preset_no_set,
            sonylens_preset_no_right, 
            NULL,
        },
        {
            MENU_ITEM_PRESET_TIME,
            "TIME/S", 
            sonylens_preset_time_get_count, 
            sonylens_preset_time_get, 
            sonylens_preset_time_set,
            sonylens_preset_time_right, 
            NULL,
        },
        {
            MENU_ITEM_PRESET_EXIST,
            "EXIST", 
            NULL, 
            NULL, 
            NULL,
            sonylens_preset_exist_right, 
            NULL,
        },
        {
            MENU_ITEM_PRESET_SET,
            "SET", 
            NULL, 
            NULL, 
            NULL,
            NULL, 
            sonylens_preset_menu_set_action,
        },
        {
            MENU_ITEM_PRESET_RUN,
            "RUN", 
            NULL, 
            NULL, 
            NULL,
            NULL, 
            sonylens_preset_menu_run_action,
        },
        {
            MENU_ITEM_PRESET_DEL,
            "DEL", 
            NULL, 
            NULL, 
            NULL,
            NULL, 
            sonylens_preset_menu_del_action,
        },
        {
            MENU_ITEM_PRESET_BACK,
            "BACK", 
            NULL,
            NULL,
            NULL,
            NULL,
            sonylens_preset_menu_back_action,
        },
    },
};




























uint8 wb_reg_gain[] = {
    202, 203, 251, 251
};

uint8 wb_blue_gain[] = {
    175, 175, 207, 207
};

//int current_wb_red_gain = 0;
//int current_wb_blue_gain = 0;


bool sonylens_msg_compare(const unsigned char* src, const unsigned char* dest, int len);

unsigned char sonylens_get_char_index(char in) {

    unsigned char ret = 0x1B;
    if(isnumber(in)) {
        if(in == '0') {
            ret = number.value + (in + 10 - number.key);
        } else {
            ret = number.value + (in - number.key);
        }
    } else if(isalphabet(in)) {
        if((in>='a')&&(in<='z')) {
            ret = alphabet.value + (in - 'a');
        } else {
            ret = alphabet.value + (in - alphabet.key);
        }
    } else if(in == ' ') {
        ret = space.value;
    } else if(in == '?') {
        ret = question.value;
    } else if(in == '-') {
        ret = stride.value;
    } else if(in == '/') {
        ret = slash.value;
    } else if(in == '.') {
        ret = dot.value;
    } else {
        ret = 0x48;
    }

    return ret;
}

bool sonylens_task_sleep(uint32 ms) {
    static uint32 currentMs = 0;
    if(0 == currentMs) {
        currentMs = GetSysTick_Sec();
    }
    if(GetSysTick_Sec() > currentMs + ms) {
        printf("\r\n sonylens_task_sleep is done. %d", GetSysTick_Sec());
        return true;
    } else {
        printf("\r\n sonylens_task_sleep is ing %d", GetSysTick_Sec());
        return false;
    }
}

int sonylens_task_prepare()
{ 
	int result;
    sonylens_task_t_sec = GetSysTick_Sec();
    memset(sonylens_reply_msg, 0, SONYLENS_REPLY_MSG_BUFF_LEN);
    return result;
}

void sonylens_push_event(SONY_EVENT_E event, void* params) {
    event_queue[0] = event;
}

void _VISCA_append_byte(VISCAPacket_t *packet, unsigned char byte)
{
    packet->bytes[packet->length]=byte;
    (packet->length)++;
}


void _VISCA_init_packet(VISCAPacket_t *packet)
{
    // we start writing at byte 1, the first byte will be filled by the
    // packet sending function. This function will also append a terminator.
    packet->bytes[0] = 0x81;
    packet->length=1;
}

void sonylens_print_config_params(CONFIG_PARAMS_t* config)
{
    int i = 0;
    
    printf("\r\n -------------------------------");
    #if 0
    printf("\r\n config params:");
    printf("\r\n  is_default            :%d", config->index);
    printf("\r\n  index                 :%d", config->is_default);
    
    printf("\r\n zoom:");
    printf("\r\n  zoom_ratio_index      :%d", config->zoom.zoom_ratio);
    printf("\r\n  zoom_speed            :%d", config->zoom.zoom_speed);
    printf("\r\n  dig_zoom              :%d", config->zoom.dig_zoom);
    printf("\r\n  osd                   :%d", config->zoom.osd);
    printf("\r\n  zoom_display          :%d", config->zoom.zoom_display);

    printf("\r\n focus:");
    printf("\r\n  mode                  :%d", config->focus.mode);
    printf("\r\n  near_limit            :%d", config->focus.near_limit);

    printf("\r\n exposure:");
    printf("\r\n  ae_mode               :%d", config->exposure.ae_mode);
    printf("\r\n  gain                  :%d", config->exposure.gain);
    printf("\r\n  slow_ae               :%d", config->exposure.slow_ae);
    printf("\r\n  slow_shutter          :%d", config->exposure.slow_shutter);
    
    printf("\r\n general:");
    #endif
    printf("\r\n  format                :%d", config->general.format);


#if 0
    printf("\r\n privacy:");
    for(i = 0; i < 4; i++)
    {
        printf("\r\n  no.:%d display:%d x:%d y:%d w:%d h:%d", 
            config->privacy_zone_list[i].no,
            config->privacy_zone_list[i].display,
            config->privacy_zone_list[i].x,
            config->privacy_zone_list[i].y,
            config->privacy_zone_list[i].w,
            config->privacy_zone_list[i].h);
    }

    printf("\r\n preset conf:");
    for(i = 0; i < 10; i++)
    {
        printf("\r\n  no.:%d time:%d exist:%d zoom_ratio:%d", 
            config->preset_conf_list[i].no,
            config->preset_conf_list[i].time,
            config->preset_conf_list[i].exist,
            config->preset_conf_list[i].zoom_ratio);
    }
#endif
    printf("\r\n -------------------------------");
}

void sonylens_init(void) {
    int len;

    // read version from eeprom
    len = AT24CXX_Read(SONYLENS_VERSION_ADDR, (unsigned char*)current_version, SONYLENS_VERSION_SIZE);
    if(len != 32) {
        printf("\r\n read version from eeprom failed.len:%d", len);
    }

    #if 1
    if(!sonylens_msg_compare(current_version, version, strlen(version)))
    {
        printf("\r\n version is not matched, update version & default config params to eeprom.");
        len = AT24CXX_Write(SONYLENS_CONFIG_PARAMS_ADDR, (unsigned char*)&default_config_params, sizeof(CONFIG_PARAMS_t));
        if(len != sizeof(default_config_params))
        {
            printf("\r\n write default config params to eeprom failed. len:%d", len);
        }
        
        len = AT24CXX_Write(SONYLENS_VERSION_ADDR, (unsigned char*)version, strlen(version) + 1);
        if(len != (strlen(version) + 1))
        {
            printf("\r\n write version to eeprom failed. len:%d", len);
        }
    }
    else
    {
        printf("\r\n version is match: %s", current_version);
    }

    // read version 
    len = AT24CXX_Read(SONYLENS_VERSION_ADDR, (unsigned char*)current_version, SONYLENS_VERSION_SIZE);
    if(len != 32) {
        printf("\r\n read version from eeprom failed.len:%d", len);
    }
    printf("\r\n version:%s", current_version);

    // read config params from eeprom
    len = AT24CXX_Read(SONYLENS_CONFIG_PARAMS_ADDR, (unsigned char*)&config_params, sizeof(CONFIG_PARAMS_t));
    if(len != sizeof(CONFIG_PARAMS_t))
    {
        printf("\r\n read config params from eeprom failed. len:%d", len);
    }
    
    #endif
    //memcpy((unsigned char*)&config_params, (unsigned char*)&default_config_params, sizeof(CONFIG_PARAMS_t));
    sonylens_print_config_params(&config_params);

    // init the global variable
    //sonylens_current_monitor_mode_index = config_params.monitor_mode_index;

    printf("\r\n ver size:%d", SONYLENS_VERSION_SIZE);
    printf("\r\n CONFIG_PARAMS_t size:%d", sizeof(CONFIG_PARAMS_t));

    mode_manager.exposure_mode = config_params.exposure.ae_mode;
    mode_manager.wb_mode = config_params.wb.wb_mode;
    mode_manager.iris = config_params.exposure.iris;
    mode_manager.expcomp = config_params.advance.expcomp;
    mode_manager.current_iris = config_params.exposure.iris;
    mode_manager.current_expcomp = config_params.advance.expcomp;

    key_freeze = config_params.image.freeze;
    key_flip = config_params.image.flip;
    key_exp_gain = config_params.exposure.gain;
    
    sony_ui_init();

}

void sonylens_zoom_init(void) 
{
    // zoom speed
    //sonylens_zoom_speed_set(config_params.zoom.zoom_speed);

    // dig zoom
    sonylens_dig_zoom_set(config_params.zoom.dig_zoom);

    // osd
    //sonylens_osd_set(config_params.zoom.osd);

    // zoom display
    //sonylens_osd_set(config_params.zoom.zoom_display);
}

void sonylens_focus_init(void) 
{
    // focus mode
    sonylens_focus_mode_set(config_params.focus.mode);

    // af sens
    sonylens_focus_af_sens_set(config_params.focus.af_sens);

    // focus light
    sonylens_focus_light_set(config_params.focus.focus_light);

    // neal limit
    sonylens_focus_near_limit_set(config_params.focus.near_limit);
}

void sonylens_exposure_init(void)
{
    // ae mode
    sonylens_exposure_ae_mode_set(config_params.exposure.ae_mode);

    if(0 != config_params.exposure.ae_mode)
    {
        // shutter
        sonylens_exposure_shutter_set(config_params.exposure.shutter);

        // iris
        sonylens_exposure_iris_set(config_params.exposure.iris);

        // gain
        sonylens_exposure_gain_set(config_params.exposure.gain);
    }
}

void sonylens_wb_init(void)
{
    // wb mode
    sonylens_wb_mode_set(config_params.wb.wb_mode);

    if(1 == config_params.wb.wb_mode)
    {
        // red gain
        sonylens_wb_red_gain_set(config_params.wb.red_gain);

        // blue gain
        sonylens_wb_blue_gain_set(config_params.wb.blue_gain);
    }
}

void sonylens_advance_init(void)
{
    // expcomp
    sonylens_advance_expcomp_set(config_params.advance.expcomp);

    // blc
    sonylens_advance_blc_set(config_params.advance.blc);    

    // wdr
    sonylens_advance_wdr_set(config_params.advance.wdr);

    // ircut
    sonylens_advance_ircut_set(config_params.advance.ircut);

    // nr level
    sonylens_advance_nr_level_set(config_params.advance.nr_level);

    // stabilizer
    sonylens_advance_stabilizer_set(config_params.advance.stabilizer);

    // defog mode
    //sonylens_advance_defog_mode_set(config_params.advance.defog_mode);

    // alarm out
    sonylens_advance_alarm_out_set(config_params.advance.alarm_out);
}

void sonylens_image_init(void)
{
    // aperture
    sonylens_image_aperture_set(config_params.image.aperture);

    // color gain
    sonylens_image_color_gain_set(config_params.image.color_gain);

    // color hue
    sonylens_image_color_hue_set(config_params.image.color_hue);

    // chroma suppress
    sonylens_image_chroma_suppress_set(config_params.image.chroma_suppress);

    // gamma
    sonylens_image_gamma_set(config_params.image.gamma);

    // mirror
    sonylens_image_mirror_set(config_params.image.mirror);

    // flip
    sonylens_image_flip_set(config_params.image.flip);

    // freeze
    sonylens_image_freeze_set(config_params.image.freeze);

    // pic effect
    sonylens_image_pic_effect_set(config_params.image.pic_effect);
}

void sonylens_general_init(void)
{
    // protocol
    sonylens_general_protocol_set(config_params.general.protocol);

    // baudrate
    sonylens_general_baudrate_set(config_params.general.baudrate);

    // address
    sonylens_general_address_set(config_params.general.address);
}

bool sonylens_set_monitor_mode(void)
{
    VISCA_result_e result;
    uint8 expect_mode;
    uint8 current_mode;
    bool ret = false;
    int count = 0;

    while(true)
    {
        result = visca_get_register(sonylens_camera_id, SONYLENS_MONITOR_MODE_ADDR, &current_mode);
        if(VISCA_result_command_not_executable == result)
        {
            count++;
            if(count > 5)
            {
                printf("\r\n read monitor mode failed. result:%d", result);
                return ret;
            }
            Wait10Ms(100);
            continue;
        }
        else if(VISCA_result_ok == result)
        {
            break;
        }
        else
        {
            printf("\r\n read monitor mode failed. result:%d", result);
            return ret;
        }
    }
    
    switch(config_params.general.format)
    {
 
        case 0:
            expect_mode = 0x15;
            break;
        case 1:
            expect_mode = 0x14;
            break;
        case 2:
            expect_mode = 0x7;
            break;
        case 3:
            expect_mode = 0x8;
            break;
        case 4:
            expect_mode = 0x2;
            break;
        case 5:
            expect_mode = 0x4;
            break;
        case 6:
            expect_mode = 0xA;
            break;
        case 7:
            expect_mode = 0xC;
            break;
        case 8:
            expect_mode = 0xF;
            break;
        case 9:
            expect_mode = 0x11;
            break;
        default:
            expect_mode = 0x11;
            break; 
    }

    if(expect_mode == current_mode)
    {
        printf("\r\n monitor mode is expected. mode:%d", expect_mode);
        monitor_mode_set_done = true;
        return true;
    }
    else
    {
        printf("\r\n monitor mode is not expected. mode:(%d-%d)", current_mode, expect_mode);
    }

    count = 0;
    while(true)
    {
        result = visca_set_register(sonylens_camera_id, SONYLENS_MONITOR_MODE_ADDR, expect_mode);
        if(VISCA_result_command_not_executable == result)
        {
            count++;
            if(count > 5)
            {
                printf("\r\n set monitor mode failed. result:%d", result);
                return ret;
            }
            Wait10Ms(100);
            continue;
        }
        else if(VISCA_result_ok == result)
        {
            break;
        }
        else
        {
            printf("\r\n set monitor mode failed. result:%d", result);
            return ret;
        }
    }

    // power off
    result = visca_set_power(sonylens_camera_id, 0x03);
    if(VISCA_result_ok != result) {
        printf("\r\n power off failed. result:%d", result);
        return ret;
    }

    // power on
    result = visca_set_power(sonylens_camera_id, 0x02);
    if(VISCA_result_ok != result) {
        printf("\r\n power on failed. result:%d", result);
        return ret;
    }

    monitor_mode_set_done = true;
    return true;
}

bool sonylens_wait_ready(void)
{
    if((GetSysTick_10Ms() - sonylens_power_on_time) > 800)
    {
        sonylens_state_ready = true;
    }
    return sonylens_state_ready;
}

bool sonylens_power_on(void)
{
    VISCA_result_e result;
    bool ret = false;

    int count = 0;

    while(true)
    {
        result = visca_set_power(sonylens_camera_id, 0x02);
        if(VISCA_result_command_not_executable == result)
        {
            count++;
            if(count > 5)
            {
                printf("\r\n power on failed. result:%d", result);
                return ret;
            }
            Wait10Ms(100);
            continue;
        }
        else if(VISCA_result_ok == result)
        {
            break;
        }
        else
        {
            printf("\r\n power on failed. result:%d", result);
            return ret;
        }
    }

    power_is_on = true;
    return true;
}

bool sonylens_power_off(void)
{
    VISCA_result_e result;
    bool ret = false;

    int count = 0;

    while(true)
    {
        result = visca_set_power(sonylens_camera_id, 0x03);
        if(VISCA_result_command_not_executable == result)
        {
            count++;
            if(count > 5)
            {
                printf("\r\n power off failed. result:%d", result);
                return ret;
            }
            Wait10Ms(100);
            continue;
        }
        else if(VISCA_result_ok == result)
        {
            break;
        }
        else
        {
            printf("\r\n power off failed. result:%d", result);
            return ret;
        }
    }

    power_is_on = false;
    return true;
}


void sonylens_print_msg(const unsigned char* msg, int len) {
    int i = 0;
    printf("\r\n msg len:%d", len);
    while(i < len) {
        if(i == 0) {
            printf("\r\n msg: 0x%02x", msg[i]);
        } else {
            printf(" 0x%02x", msg[i]);
        }
        i ++;
    }
}

void sonylens_print_packet(VISCAPacket_t packet) {
    int i = 0;
    printf("\r\n packet len:%d", packet.length);
    while(i < packet.length) {
        if(i == 0) {
            printf("\r\n packet: 0x%02x", packet.bytes[i]);
        } else {
            printf(" 0x%02x", packet.bytes[i]);
        }
        i ++;
    }
}


void sonylens_clear_replymsg() {
    memset(sonylens_reply_msg, '\0', SONYLENS_REPLY_MSG_BUFF_LEN);
    sonylens_reply_msg_len = 0;
}

bool sonylens_msg_compare(const unsigned char* src, const unsigned char* dest, int len) {
    bool result = true;
    int i = 0;
    while(i < len) {
        if(src[i] != dest[i]) {
            result = false;
            break;
        }
        i++;
    }

    if(!result) {
        printf("\r\n expected msg is :");
        sonylens_print_msg(dest, len);
        printf("\r\n current msg is :");
        sonylens_print_msg(src, len);
    }
    
    return result;
}

void sonylens_enter_hw_error() {
    sonylens_taskstate = MODEM_HW_ERROR;
}

void sonylens_enter_idle() {
    sonylens_taskstate = SONY_IDLE;
}

SONY_RESULT_E sonylens_send_packet_with_reply(VISCAPacket_t packet) {
    SONY_RESULT_E result = SONY_RESULT_BUSYING;
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n sonylens_send_packet_with_reply...");
            sonylens_print_packet(packet);
            uart2_comm_clr();
            uart2_comm_write(packet.bytes, packet.length);
            sonylens_cmd_state = SONY_CMD_WAIT_ACK;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_CMD_WAIT_ACK:
            //if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
            if( uart2_comm_message_available() > 0){
                sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                //uart2_comm_clr();
                sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);    
                sonylens_cmd_state = SONY_CMD_ACKED;
            }
            //}
            
            break;
        case SONY_CMD_ACKED:
            if(sonylens_msg_compare(sonylens_reply_msg, reply_msg, sizeof(reply_msg))) {
                sonylens_cmd_state = SONY_CMD_WAIT_COMPLETION_MSG;
            } else if(sonylens_msg_compare(sonylens_reply_msg, network_change, sizeof(network_change))) {
                printf("\r\n network change message is accepted.");
                sonylens_enter_idle();
                sonylens_cmd_state = SONY_CMD_IDLE;
            } else if(sonylens_msg_compare(sonylens_reply_msg, command_not_executable, sizeof(command_not_executable))) {
                printf("\r\n command not executable.");
                sonylens_enter_idle();
                sonylens_cmd_state = SONY_CMD_IDLE;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_clear_replymsg();
            break;
        case SONY_CMD_WAIT_COMPLETION_MSG:
            //if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS * 2)) {
                if( uart2_comm_message_available() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    //uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_COMPLETED;
                }
            //}
            break;
        case SONY_CMD_COMPLETED:
            if(sonylens_msg_compare(sonylens_reply_msg, completion_msg, sizeof(completion_msg))) {
                // do something
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            result = SONY_RESULT_OK;
            sonylens_clear_replymsg();
            break;
    }

    return result;
}


void sonylens_set_address(unsigned char id) {
    unsigned char cmd[] = {0x88,0x30,0x01,0xff};//{0x81,0x01,0x04,0x00,0x02,0xff};
    unsigned char reply_msg[] = {0x88,0x30, 0x02, 0xff};

    // id should be less than 8 (1~7)
    cmd[2] = id;
    reply_msg[2] = id+1;
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n set address...");
            uart2_comm_clr();
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_CMD_WAIT_ACK;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_CMD_WAIT_ACK:
            //if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_message_available() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_ACKED;
                }
            //}
            
            break;
        case SONY_CMD_ACKED:
            if(sonylens_msg_compare(sonylens_reply_msg, network_change, sizeof(network_change))) {
                printf("\r\n should re-set the address again");
            } else if(sonylens_msg_compare(sonylens_reply_msg, reply_msg, sizeof(reply_msg))) {
                sonylens_taskstate = SONY_IF_CLEAR;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }
}

bool sonylens_if_clear() {
    unsigned char cmd[] = {0x88,0x01,0x00,0x01,0xff};
    unsigned char reply_msg[] = {0x88,0x01,0x00,0x01,0xff};
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n IF clear...");
            uart2_comm_clr();
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_CMD_WAIT_ACK;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_CMD_WAIT_ACK:
            //if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_message_available() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_ACKED;
                }
            //}
            
            break;
        case SONY_CMD_ACKED:
            if(strcmp(sonylens_reply_msg, reply_msg)==0) {
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return (sonylens_cmd_state == SONY_CMD_IDLE);
}



SONY_RESULT_E sonylens_setRegisterValue(unsigned char reg, unsigned char value){

    SONY_RESULT_E result = SONY_RESULT_BUSYING;
    VISCAPacket_t packet;
    int i;

    _VISCA_init_packet(&packet);
    _VISCA_append_byte(&packet, VISCA_COMMAND);
    _VISCA_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    _VISCA_append_byte(&packet, 0x24);
    _VISCA_append_byte(&packet, reg);
    _VISCA_append_byte(&packet, (value & 0xf0) >> 4);
    _VISCA_append_byte(&packet, value & 0x0f);
    _VISCA_append_byte(&packet, VISCA_TERMINATOR);

    result = sonylens_send_packet_with_reply(packet);

    return result;

    /*
    unsigned char cmd[] = {0x81,0x01,0x04,0x24,0xee,0x0e,0x0e,0xff};
    unsigned char reply_msg[] = {0x90, 0x41, 0xff};
    unsigned char completion_msg[] = {0x90, 0x51, 0xff};

    cmd[4] = reg;
    cmd[5] = (value & 0xf0) >> 4;
    cmd[6] = (value & 0x0f);
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n set register(0x%02x) value(0x%02x)...", reg, value);
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_CMD_WAIT_ACK;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_CMD_WAIT_ACK:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_ACKED;
                }
            }
            
            break;
        case SONY_CMD_ACKED:
            if(sonylens_msg_compare(sonylens_reply_msg, reply_msg, sizeof(reply_msg))) {
                sonylens_cmd_state = SONY_CMD_WAIT_COMPLETION_MSG;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_clear_replymsg();
            break;
        case SONY_CMD_WAIT_COMPLETION_MSG:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS * 2)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_COMPLETED;
                }
            }
            break;
        case SONY_CMD_COMPLETED:
            if(sonylens_msg_compare(sonylens_reply_msg, completion_msg, sizeof(completion_msg))) {
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return (sonylens_cmd_state == SONY_CMD_IDLE);
    */
}

bool sonylens_DisplayMode(bool on){
    unsigned char cmd[] = {0x81,0x01,0x04,0x15,0x02,0xff};
    unsigned char reply_msg[] = {0x90, 0x41, 0xff};
    unsigned char completion_msg[] = {0x90, 0x51, 0xff};

    if(on) {
        cmd[4] = 0x02;
    } else {
        cmd[4] = 0x03;
    }
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n Display Mode %d...", on);
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_CMD_WAIT_ACK;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_CMD_WAIT_ACK:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_ACKED;
                }
            }
            
            break;
        case SONY_CMD_ACKED:
            if(strcmp(sonylens_reply_msg, reply_msg)==0) {
                sonylens_cmd_state = SONY_CMD_WAIT_COMPLETION_MSG;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_clear_replymsg();
            break;
        case SONY_CMD_WAIT_COMPLETION_MSG:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS * 2)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_COMPLETED;
                }
            }
            break;
        case SONY_CMD_COMPLETED:
            if(strcmp(sonylens_reply_msg, completion_msg)==0) {
                //printf("\r\n enter working stage..");
                //sonylens_taskstate = SONY_POWER_WORKING;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return (sonylens_cmd_state == SONY_CMD_IDLE);
}

bool sonylens_MuteMode(bool on){
    unsigned char cmd[] = {0x81,0x01,0x04,0x75,0x02,0xff};
    unsigned char reply_msg[] = {0x90, 0x41, 0xff};
    unsigned char completion_msg[] = {0x90, 0x51, 0xff};

    if(on) {
        cmd[4] = 0x02;
    } else {
        cmd[4] = 0x03;
    }
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n Mute Mode %d...", on);
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_CMD_WAIT_ACK;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_CMD_WAIT_ACK:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_ACKED;
                }
            }
            
            break;
        case SONY_CMD_ACKED:
            if(strcmp(sonylens_reply_msg, reply_msg)==0) {
                sonylens_cmd_state = SONY_CMD_WAIT_COMPLETION_MSG;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_clear_replymsg();
            break;
        case SONY_CMD_WAIT_COMPLETION_MSG:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS * 2)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_COMPLETED;
                }
            }
            break;
        case SONY_CMD_COMPLETED:
            if(strcmp(sonylens_reply_msg, completion_msg)==0) {
                //printf("\r\n enter working stage..");
                //sonylens_taskstate = SONY_POWER_WORKING;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return (sonylens_cmd_state == SONY_CMD_IDLE);
}

bool sonylens_InitializeLens(){
    unsigned char cmd[] = {0x81,0x01,0x04,0x19,0x01,0xff};
    unsigned char reply_msg[] = {0x90, 0x41, 0xff};
    unsigned char completion_msg[] = {0x90, 0x51, 0xff};
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n Initialize Lens ...");
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_CMD_WAIT_ACK;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_CMD_WAIT_ACK:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_ACKED;
                }
            }
            
            break;
        case SONY_CMD_ACKED:
            if(strcmp(sonylens_reply_msg, reply_msg)==0) {
                sonylens_cmd_state = SONY_CMD_WAIT_COMPLETION_MSG;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_clear_replymsg();
            break;
        case SONY_CMD_WAIT_COMPLETION_MSG:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS * 2)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_COMPLETED;
                }
            }
            break;
        case SONY_CMD_COMPLETED:
            if(strcmp(sonylens_reply_msg, completion_msg)==0) {
                //printf("\r\n enter working stage..");
                //sonylens_taskstate = SONY_POWER_WORKING;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return (sonylens_cmd_state == SONY_CMD_IDLE);
}

bool sonylens_InitializeCamera(){
    unsigned char cmd[] = {0x81,0x01,0x04,0x19,0x03,0xff};
    unsigned char reply_msg[] = {0x90, 0x41, 0xff};
    unsigned char completion_msg[] = {0x90, 0x51, 0xff};
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n Initialize Camera ...");
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_CMD_WAIT_ACK;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_CMD_WAIT_ACK:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_ACKED;
                }
            }
            
            break;
        case SONY_CMD_ACKED:
            if(strcmp(sonylens_reply_msg, reply_msg)==0) {
                sonylens_cmd_state = SONY_CMD_WAIT_COMPLETION_MSG;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_clear_replymsg();
            break;
        case SONY_CMD_WAIT_COMPLETION_MSG:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS * 2)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_COMPLETED;
                }
            }
            break;
        case SONY_CMD_COMPLETED:
            if(strcmp(sonylens_reply_msg, completion_msg)==0) {
                //printf("\r\n enter working stage..");
                //sonylens_taskstate = SONY_POWER_WORKING;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return (sonylens_cmd_state == SONY_CMD_IDLE);
}

bool sonylens_setCamAE() {
    unsigned char cmd[] = {0x81,0x01,0x04,0x39,0x00,0xff};
    unsigned char reply_msg[] = {0x90, 0x41, 0xff};
    unsigned char completion_msg[] = {0x90, 0x51, 0xff};
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n set CamAE...");
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_CMD_WAIT_ACK;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_CMD_WAIT_ACK:
             printf("\r\n set CamAE...~~");
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_ACKED;
                }
            }
            
            break;
        case SONY_CMD_ACKED:
            if(sonylens_msg_compare(sonylens_reply_msg, reply_msg, sizeof(reply_msg))) {
                sonylens_cmd_state = SONY_CMD_WAIT_COMPLETION_MSG;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_clear_replymsg();
            break;
        case SONY_CMD_WAIT_COMPLETION_MSG:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS * 2)) {
                if( uart2_comm_get_len() > 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_CMD_COMPLETED;
                }
            }
            break;
        case SONY_CMD_COMPLETED:
            if(sonylens_msg_compare(sonylens_reply_msg, completion_msg, sizeof(completion_msg))) {
                // do something
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return (sonylens_cmd_state == SONY_CMD_IDLE);
}

bool sonylens_PowerInq() {
    unsigned char cmd[] = {0x81,0x09,0x04,0x00,0xff};
    unsigned char msg_on[] = {0x90, 0x50, 0x02, 0xff};
    unsigned char msg_off[] = {0x90, 0x50, 0x03, 0xff};
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n Inquiry Power...");
            uart2_comm_clr();
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_INQ_WAIT_RESULT;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_INQ_WAIT_RESULT:
            //if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_message_available()> 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_INQ_GET_RESULT;
                }
            //}
            
            break;
        case SONY_INQ_GET_RESULT:
            if(sonylens_msg_compare(sonylens_reply_msg, msg_on, sizeof(msg_on))) {
                sonylens_power_state = 1;
                printf("\r\n power is ON.");
            } else if(sonylens_msg_compare(sonylens_reply_msg, msg_off, sizeof(msg_off))) {
                printf("\r\n power is OFF.");
                sonylens_power_state = 0;
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return (sonylens_cmd_state == SONY_CMD_IDLE);
}

bool sonylens_DisplayModeInq() {
    unsigned char cmd[] = {0x81,0x09,0x04,0x15,0xff};
    unsigned char msg_on[] = {0x90, 0x50, 0x02, 0xff};
    unsigned char msg_off[] = {0x90, 0x50, 0x03, 0xff};
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n Inquiry Display Mode...");
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_INQ_WAIT_RESULT;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_INQ_WAIT_RESULT:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_message_available()> 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_INQ_GET_RESULT;
                }
            }
            
            break;
        case SONY_INQ_GET_RESULT:
            if(sonylens_msg_compare(sonylens_reply_msg, msg_on, sizeof(msg_on))) {
                printf("\r\n Display Mode is ON.");
            } else if(sonylens_msg_compare(sonylens_reply_msg, msg_off, sizeof(msg_off))) {
                printf("\r\n Display Mode is OFF.");
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return (sonylens_cmd_state == SONY_CMD_IDLE);
}


bool sonylens_MuteModeInq() {
    unsigned char cmd[] = {0x81,0x09,0x04,0x75,0xff};
    unsigned char msg_on[] = {0x90, 0x50, 0x02, 0xff};
    unsigned char msg_off[] = {0x90, 0x50, 0x03, 0xff};
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n Inquiry Mute Mode...");
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_INQ_WAIT_RESULT;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_INQ_WAIT_RESULT:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_message_available()> 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_INQ_GET_RESULT;
                }
            }
            
            break;
        case SONY_INQ_GET_RESULT:
            if(sonylens_msg_compare(sonylens_reply_msg, msg_on, sizeof(msg_on))) {
                printf("\r\n Mute Mode is ON.");
            } else if(sonylens_msg_compare(sonylens_reply_msg, msg_off, sizeof(msg_off))) {
                printf("\r\n Mute Mode is OFF.");
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return (sonylens_cmd_state == SONY_CMD_IDLE);
}

unsigned char sonylens_RegisterValueInq(unsigned char reg) {
    unsigned char result = 0xff;
    unsigned char cmd[] = {0x81,0x09,0x04,0x24,0x00,0xff};
    unsigned char msg_result[] = {0x90, 0x50};

    cmd[4] = reg;
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n Inquiry register(0x%02x) value...", reg);
            uart2_comm_clr();
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_INQ_WAIT_RESULT;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_INQ_WAIT_RESULT:
            //if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_message_available()> 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_INQ_GET_RESULT;
                }
            //}
            
            break;
        case SONY_INQ_GET_RESULT:
            if(sonylens_msg_compare(sonylens_reply_msg, msg_result, sizeof(msg_result))) {
                result = ((sonylens_reply_msg[2]&0x0f)<<4) | (sonylens_reply_msg[3]&0x0f);
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return result;
}

unsigned char sonylens_get_Cam(unsigned char reg) {
    unsigned char result = 0xff;
    unsigned char cmd[] = {0x81,0x09,0x04,0x24,0x00,0xff};
    unsigned char msg_result[] = {0x90, 0x50};

    cmd[4] = reg;
    
    switch(sonylens_cmd_state) {
        case SONY_CMD_IDLE:
            printf("\r\n Inquiry register(0x%02x) value...", reg);
            uart2_comm_write(cmd, sizeof(cmd));
            sonylens_cmd_state = SONY_INQ_WAIT_RESULT;
            sonylens_send_msg_time = GetSysTick_10Ms();
            break;
        case SONY_INQ_WAIT_RESULT:
            if(GetSysTick_10Ms() >= (sonylens_send_msg_time + SONYLENS_SEND_MSG_WAIT_MS)) {
                if( uart2_comm_message_available()> 0){
                    sonylens_reply_msg_len = uart2_comm_read(sonylens_reply_msg, SONYLENS_REPLY_MSG_BUFF_LEN);
                    uart2_comm_clr();
                    sonylens_print_msg(sonylens_reply_msg, sonylens_reply_msg_len);
                    sonylens_cmd_state = SONY_INQ_GET_RESULT;
                }
            }
            
            break;
        case SONY_INQ_GET_RESULT:
            if(sonylens_msg_compare(sonylens_reply_msg, msg_result, sizeof(msg_result))) {
                result = ((sonylens_reply_msg[2]&0x0f)<<4) | (sonylens_reply_msg[3]&0x0f);
            } else {
                printf("\r\n unexpected reply message is accepted.");
                sonylens_enter_hw_error();
            }
            sonylens_cmd_state = SONY_CMD_IDLE;
            sonylens_clear_replymsg();
            break;
    }

    return result;
}

void sonylens_splash_display(void)
{
    int i;
    int count = sizeof(splash)/sizeof(SONY_SPLASH_t);
    sony_ui_display_all(false);
    for(i = 0; i < count; i++)
    {
        sony_ui_set_title(i, NULL, splash[i].key, splash[i].get?splash[i].get():NULL);
    }

    sony_ui_display_all(true);
    splash_display_time = GetSysTick_10Ms();
    splash_display = true;
}

void sonylens_splash_clear(void)
{
    if((true == splash_display) && ((GetSysTick_10Ms() - splash_display_time) > 800))
    {
        sony_ui_display_all(false);
        sony_ui_clear_all();
        splash_display = false;
    }
}


void sonylens_menu_on1(void)
{
    int i;
    SONY_MENU_ITEM_t* menu_item;
    int menu_count = current_menu.menu_item_count;
    char* right;
    char* head;
    //current_menu.init();
    if(current_menu.id == MENU_PRESET_CONF) {
        config_params.general.preset_conf_no = 0;
    }
    
    sony_ui_clear_all();
    
    sony_ui_set_title(0, NULL, current_menu.menu_name, NULL);
    sony_ui_set_color(0, 1);

    for(i = 0; i < menu_count; i++)
    {
        sonylens_menu_item_update(i);
    }
    sony_ui_display_all(true);
    
    menu_display = true;
    menu_display_time = GetSysTick_10Ms();
    zoom_ratio_display = false;
    format_display = false;
    splash_display = false;
}

void sonylens_menu_off(void)
{
    if(menu_display)
    {
        sony_ui_clear_all();
        sony_ui_display_all(false);
        menu_display = false;
    }
}

void sonylens_menu_move_updown1(int step) {
    // select menu item
    SONY_MENU_ITEM_t* menu_item;
    int old_selected = current_menu.menu_item_selected;
    char* right;
    char* head;

    if(step > 0) {
        current_menu.menu_item_selected++;
        if(current_menu.menu_item_selected >= current_menu.menu_item_count) {
            current_menu.menu_item_selected = 0;
        }
    } else {
        if(current_menu.menu_item_selected <= 0) {
            current_menu.menu_item_selected = current_menu.menu_item_count;
        }
        current_menu.menu_item_selected--;
    }

    sonylens_menu_item_update(old_selected);
    sonylens_menu_item_update(current_menu.menu_item_selected);

    menu_display_time = GetSysTick_10Ms();

    #if 0
    menu_item = &current_menu.menu_item_list[old_selected];
    if(menu_item->disable)
    {
        right = "-";
    }
    else
    {
        right = menu_item->get?menu_item->right():NULL;
    }        
    sony_ui_set_title(old_selected + 1, " ", menu_item->name, right);

    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];
    if(menu_item->disable)
    {
        right = "-";
    }
    else
    {
        right = menu_item->get?menu_item->right():NULL;
    }
    
    if(menu_item->ineffective)
    {
        head = "?";
    }
    else
    {
        head = "-";
    }
    sony_ui_set_title(current_menu.menu_item_selected + 1, head, menu_item->name, right);
    #endif
}

void sonylens_menu_move_updown(int step) {
    // select menu item
    VISCA_result_e result;
    VISCA_title_t t;
    SONY_MENU_ITEM_t* menu_item;
    int len;
    int j;
    int old_menu_item_selected = current_menu.menu_item_selected;

    

    #if 0
    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];
    len = strlen(menu_item->name);
     
    t.vposition = current_menu.menu_item_selected + 1;
    t.hposition = 0;
    t.blink = 0;
    t.color = 0;
        
    memset(t.title, 0x1B, sizeof(t.title));
    sonylens_print_msg(menu_item->name, len);
    t.title[0] = sonylens_get_char_index(' ');
    
    for(j = 0; j < len; j++) {
        t.title[j+1] = sonylens_get_char_index(menu_item->name[j]);
    }

    if(menu_item->get != NULL) {
        if(menu_item->disable)
        {
            t.title[19] = sonylens_get_char_index('-');
        }
        else
        {
            menu_item->get(&menu_item->param, t.title);
        }
    }
    
    sonylens_print_msg(t.title, sizeof(t.title));
    sonylens_set_title(&t);
    #endif
    
    if(step > 0) {
        current_menu.menu_item_selected++;
        if(current_menu.menu_item_selected >= current_menu.menu_item_count) {
            current_menu.menu_item_selected = 0;
        }
    } else {
        if(current_menu.menu_item_selected <= 0) {
            current_menu.menu_item_selected = current_menu.menu_item_count;
        }
        current_menu.menu_item_selected--;
    }

    sonylens_menu_update(old_menu_item_selected);
    sonylens_menu_update(current_menu.menu_item_selected);
    #if 0

    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];
    len = strlen(menu_item->name);
     
    t.vposition = current_menu.menu_item_selected + 1;
    t.hposition = 0;
    t.blink = 0;
    t.color = 0;
        
    memset(t.title, 0x1B, sizeof(t.title));
    sonylens_print_msg(menu_item->name, len);
    t.title[0] = sonylens_get_char_index('-');
    
    for(j = 0; j < len; j++) {
        t.title[j+1] = sonylens_get_char_index(menu_item->name[j]);
    }

    if(menu_item->get != NULL) {
        if(menu_item->disable)
        {
            t.title[19] = sonylens_get_char_index('-');
        }
        else
        {
            menu_item->get(&menu_item->param, t.title);
        }
    }
    
    sonylens_print_msg(t.title, sizeof(t.title));
    sonylens_set_title(&t);
    #endif
}

void sonylens_menu_move_leftright(int step) {
    // select menu item
    VISCA_result_e result;
    VISCA_title_t t;
    SONY_MENU_ITEM_t* menu_item;
    int len;
    int j;
    int count, value;
    
    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];

    if(menu_item->disable)
    {
        return;
    }

    if((!menu_item->get_count) || (!menu_item->get) || (!menu_item->set) || (!menu_item->right))
    {
        return;
    }

    count = menu_item->get_count();
    value = menu_item->get();
    
    if(step > 0) {
        value++;
        if(value >= count) {
            value = 0;
        }
    } else {
        value--;
        if(value < 0) {
            value = count - 1;
        }
    }

    menu_item->set(value);
    
    if(menu_item->id == MENU_ITEM_FOCUS_MODE)
    {
        current_menu.init();
        sonylens_menu_item_update(current_menu.menu_item_selected);
    }
    else if(menu_item->id == MENU_ITEM_EXPOSURE_AE_MODE)
    {
        current_menu.init();
        sonylens_menu_item_update(current_menu.menu_item_selected);
        sonylens_menu_item_update(current_menu.menu_item_selected + 1);
        sonylens_menu_item_update(current_menu.menu_item_selected + 2);
        sonylens_menu_item_update(current_menu.menu_item_selected + 3);
        sonylens_menu_item_update(current_menu.menu_item_selected + 4);
    }
    else if(menu_item->id == MENU_ITEM_WB_MODE)
    {
        current_menu.init();
        sonylens_menu_item_update(current_menu.menu_item_selected);
        sonylens_menu_item_update(current_menu.menu_item_selected + 1);
        sonylens_menu_item_update(current_menu.menu_item_selected + 2);
    }
    else if(menu_item->id == MENU_ITEM_PRIVACY_ZONE)
    {
        //current_menu.init();
        sonylens_menu_item_update(current_menu.menu_item_selected);
        sonylens_menu_item_update(current_menu.menu_item_selected + 1);
        sonylens_menu_item_update(current_menu.menu_item_selected + 2);
        sonylens_menu_item_update(current_menu.menu_item_selected + 3);
        sonylens_menu_item_update(current_menu.menu_item_selected + 4);
        sonylens_menu_item_update(current_menu.menu_item_selected + 5);
    }
    else if(menu_item->id == MENU_ITEM_PRESET_NO)
    {
        //current_menu.init();
        sonylens_menu_item_update(current_menu.menu_item_selected);
        sonylens_menu_item_update(current_menu.menu_item_selected + 1);
        sonylens_menu_item_update(current_menu.menu_item_selected + 2);
    }
    else if(menu_item->id == MENU_ITEM_GENERAL_FORMAT)
    {
        monitor_mode_changed = true;
        menu_item->ineffective = 1;
        sonylens_menu_item_update(current_menu.menu_item_selected);
    }
    else
    {
        sonylens_menu_item_update(current_menu.menu_item_selected);
    }

    menu_display_time = GetSysTick_10Ms();
}


void sonylens_menu_update(int index)
{
    #if 0
    VISCA_result_e result;
    VISCA_title_t t;
    SONY_MENU_ITEM_t* menu_item;
    int len;
    int j;

    menu_item = &current_menu.menu_item_list[index];

    len = strlen(menu_item->name);
     
    t.vposition = index + 1;
    t.hposition = 0;
    t.blink = 0;
    t.color = 0;
        
    memset(t.title, 0x1B, sizeof(t.title));

    if(index == current_menu.menu_item_selected)
    {
        if(menu_item->ineffective)
        {
            t.title[0] = sonylens_get_char_index('?');
        }
        else
        {
            t.title[0] = sonylens_get_char_index('-');
        }
    }
    
    
    for(j = 0; j < len; j++) {
        t.title[j+1] = sonylens_get_char_index(menu_item->name[j]);
    }

    if(menu_item->get != NULL) {
        if(menu_item->disable)
        {
            t.title[19] = sonylens_get_char_index('-');
        }
        else
        {
            menu_item->get(&menu_item->param, t.title);
        }
    }
    
    sonylens_print_msg(t.title, sizeof(t.title));
    sonylens_set_title(&t);
    #endif
}

void sonylens_menu_item_update(int index)
{
    char* head;
    char* right;
    
    SONY_MENU_ITEM_t* menu_item;
    menu_item = &current_menu.menu_item_list[index];

    if(index == current_menu.menu_item_selected)
    {
        head = menu_item->ineffective?"?":"-";
        if((menu_item->id == MENU_ITEM_DEFAULT) && (menu_item->ineffective))
        {
            sony_ui_set_color(index + 1, 0x03);
        }
    }
    else
    {
        head = " ";
        if(menu_item->id == MENU_ITEM_DEFAULT)
        {
            menu_item->ineffective = 0;
            menu_item->set(0);
            sony_ui_set_color(index + 1, 0x00);
        }
    }

    if(menu_item->right)
    {
        right = menu_item->disable?"-":menu_item->right();
    }
    else
    {
        right = NULL;
    }

    

    sony_ui_set_title(index + 1, head, menu_item->name, right);
}

void sonylens_menu_action() {
    SONY_MENU_ITEM_t* menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];

    if(NULL != menu_item->action) {
        // do something
        menu_item->action();
        if(menu_item->id == MENU_ITEM_DEFAULT)
        {
            if(0 == menu_item->ineffective)
            {
                menu_item->ineffective = 1;
                sonylens_menu_item_update(current_menu.menu_item_selected);
            }
        }
        else if(menu_item->id == MENU_ITEM_PRESET_SET)
        {
            sonylens_menu_item_update(2);
        }
        else if(menu_item->id == MENU_ITEM_PRESET_DEL)
        {
            sonylens_menu_item_update(2);
        }
    }
}

void sonylens_focus_menu_update(void)
{
    #if 0
    SONY_MENU_ITEM_t* menu_item;
    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];

    if(current_menu.menu_item_selected == 0)
    {
        if(menu_item->param == 0)
        {
            current_menu.menu_item_list[1].disable = 1;
            current_menu.menu_item_list[2].disable = 1;
            current_menu.menu_item_list[3].disable = 1;
        }
        else
        {
            current_menu.menu_item_list[1].disable = 0;
            current_menu.menu_item_list[2].disable = 0;
            current_menu.menu_item_list[3].disable = 0;
        }        
    }
    #endif
}

void sonylens_exposure_menu_update(void)
{
    #if 0
    SONY_MENU_ITEM_t* menu_item;
    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];

    if(current_menu.menu_item_selected == 0)
    {
        if(menu_item->param == 0)
        {
            current_menu.menu_item_list[1].disable = 1;
            current_menu.menu_item_list[2].disable = 1;
            current_menu.menu_item_list[3].disable = 1;
            current_menu.menu_item_list[4].disable = 1;
        }
        else
        {
            current_menu.menu_item_list[1].disable = 0;
            current_menu.menu_item_list[2].disable = 0;
            current_menu.menu_item_list[3].disable = 0;
            current_menu.menu_item_list[4].disable = 1;
        }        
    }
    #endif
}

void sonylens_wb_menu_update(void)
{
    #if 0
    SONY_MENU_ITEM_t* menu_item;
    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];

    if(current_menu.menu_item_selected == 0)
    {
        if(menu_item->param == 3)
        {
            current_menu.menu_item_list[1].disable = 0;
            current_menu.menu_item_list[2].disable = 0;
        }
        else
        {
            current_menu.menu_item_list[1].disable = 1;
            current_menu.menu_item_list[2].disable = 1;
        }        
    }
    #endif
}

void sonylens_privacy_menu_update(void)
{
    #if 0
    SONY_MENU_ITEM_t* menu_item;
    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];

    if(current_menu.menu_item_selected == 0)
    {
        current_menu.menu_item_list[1].param = privacy_zone_list[menu_item->param].display;
        current_menu.menu_item_list[2].param = privacy_zone_list[menu_item->param].x;
        current_menu.menu_item_list[3].param = privacy_zone_list[menu_item->param].y;
        current_menu.menu_item_list[4].param = privacy_zone_list[menu_item->param].w;
        current_menu.menu_item_list[5].param = privacy_zone_list[menu_item->param].h;     
    }
    #endif
}

void sonylens_preset_conf_menu_update(void)
{
    SONY_MENU_ITEM_t* menu_item;
    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];

    if(current_menu.menu_item_selected == 0)
    {
        //current_menu.menu_item_list[1].param = preset_conf_list[menu_item->param].time;
        //current_menu.menu_item_list[2].param = preset_conf_list[menu_item->param].exist;
    }
}

void sonylens_general_menu_update(void)
{
    SONY_MENU_ITEM_t* menu_item;
    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];
    menu_item->ineffective = 1;
}

void sonylens_set_title(VISCA_title_t* t) {
    VISCA_result_e result = VISCA_result_ok;

    result = visca_set_title(sonylens_camera_id, t->vposition, t->title);
    if(result != VISCA_result_ok) {
        return;
    }
}

void sonylens_show_title(uint8 lines, uint8 enable) {
    VISCA_result_e result;
    result = visca_set_title_display(sonylens_camera_id, lines, enable);
    if(result != VISCA_result_ok) {
        return;
    }
}

void sonylens_clear_title(uint8 lines) {
    VISCA_result_e result;
    result = visca_set_title_clear(sonylens_camera_id, lines);
    if(result != VISCA_result_ok) {
        return;
    }
}

void sonylens_monitor_mode_switch(int step) {
    VISCA_result_e result;
    VISCA_title_t t;
    unsigned char mode;
    int len, j;

    int list_count = sizeof(monitor_mode_list)/sizeof(ZOOM_RATIO_t);
    
    if(step > 0) {
        sonylens_current_monitor_mode_index ++;
        if(sonylens_current_monitor_mode_index >= list_count) {
            sonylens_current_monitor_mode_index = 0;
        }
    } else if(step < 0) {
        if(sonylens_current_monitor_mode_index <= 0) {
            sonylens_current_monitor_mode_index = list_count;
        }
        sonylens_current_monitor_mode_index --;
    }

    len = strlen(monitor_mode_list[sonylens_current_monitor_mode_index].desc);

    t.vposition = 0;
    t.hposition = 0;
    t.blink = 0;
    t.color = 0;
    memset(t.title, 0x1B, sizeof(t.title));
    for(j = 0; j < len; j++) {
        t.title[j] = sonylens_get_char_index(monitor_mode_list[sonylens_current_monitor_mode_index].desc[j]);
    }
    sonylens_show_title(0, 0);
    sonylens_set_title(&t);
    
    format_display = true;
    format_display_time = GetSysTick_10Ms();
    
    mode = monitor_mode_list[sonylens_current_monitor_mode_index].mode;
    result = visca_set_register(sonylens_camera_id, SONYLENS_MONITOR_MODE_ADDR, mode);
    if(VISCA_result_ok != result) {
        printf("\r\n set monitor mode failed.");
    }

    //config_params.monitor_mode_index = sonylens_current_monitor_mode_index;

    sonylens_taskstate = SONY_POWER_OFF;
}
void sonylens_zoom_ratio_switch(int step) {
    VISCA_result_e result;
    VISCA_title_t t;
    uint32 ratio;
    int len;
    int j;

    int zoom_ratio_list_count = sizeof(zoom_ratio_list)/sizeof(ZOOM_RATIO_t);
    
    if(step > 0) {
        if(sonylens_current_zoom_ratio < (zoom_ratio_list_count - 1)) {
            sonylens_current_zoom_ratio ++;
        } else {
            return;
        }
    } else if(step < 0) {
        if(sonylens_current_zoom_ratio > 0) {
            sonylens_current_zoom_ratio--;
        } else {
            return;
        }
    }
    
    ratio = zoom_ratio_list[sonylens_current_zoom_ratio].ratio;
    result = visca_set_zoom_value(sonylens_camera_id, ratio);
    if(VISCA_result_ok != result) {
        printf("\r\n set zoom ratio failed.");
        return;
    }

    zoom_ratio_changed = true;
}

void sonylens_set_zoom_ratio(int step)
{
    VISCA_result_e result;
    int min = zoom_ratio_range[0];
    int max = zoom_ratio_range[1];
    int ratio;

    ratio = (int)config_params.zoom.zoom_ratio + step;
    if(ratio < min)
    {
        ratio = min;
    }
    else if(ratio > max)
    {
        ratio = max;
    }
    result = visca_set_zoom_value(sonylens_camera_id, (uint32)ratio);
    if(VISCA_result_ok != result) {
        printf("\r\n set zoom ratio failed.");
        return;
    }
    config_params.zoom.zoom_ratio = (uint32)ratio;
}

void sonylens_set_focus_near_limit(int step)
{
    VISCA_result_e result;
    int min = focus_near_limit_range[0];
    int max = focus_near_limit_range[1];
    int ratio;

    ratio = (int)current_focus_near_limit + step;
    if(ratio < min)
    {
        ratio = min;
    }
    else if(ratio > max)
    {
        ratio = max;
    }
    result = visca_set_focus_near_limit(sonylens_camera_id, (uint32)ratio);
    if(VISCA_result_ok != result) {
        printf("\r\n set focus_near_limit failed.");
        return;
    }
    current_focus_near_limit = (uint32)ratio;
}

void sonylens_focus_near_limit_switch(int step)
{
    VISCA_result_e result;
    int j;
    uint32 limit;
    int list_count = sizeof(focus_near_limit_list)/sizeof(FOCUS_NEAR_LIMIT_t);
    
    if(step > 0) {
        if(current_focus_near_limit_index < (list_count - 1)) {
            current_focus_near_limit_index ++;
        } else {
            return;
        }
    } else if(step < 0) {
        if(current_focus_near_limit_index > 0) {
            current_focus_near_limit_index--;
        } else {
            return;
        }
    }
    
    limit = focus_near_limit_list[current_focus_near_limit_index].limit;
    result = visca_set_focus_near_limit(sonylens_camera_id, limit);
    if(VISCA_result_ok != result) {
        printf("\r\n set zoom ratio failed.");
        return;
    }
}

int focus_flag = 0;
void sonylens_set_focus_near()
{
    VISCA_result_e result;

    result = visca_set_focus_near_speed(sonylens_camera_id, config_params.focus.focus_speed);
    if(VISCA_result_ok != result) {
        printf("\r\n set set_focus_near failed.");
        return;
    }
    focus_flag= 1;
}
void sonylens_set_focus_far()
{
    VISCA_result_e result;

    result = visca_set_focus_far_speed(sonylens_camera_id, config_params.focus.focus_speed);
    if(VISCA_result_ok != result) {
        printf("\r\n set set_focus_near failed.");
        return;
    }
    focus_flag = 2;
}
void sonylens_set_focus_stop()
{
    VISCA_result_e result;

    result = visca_set_focus_stop(sonylens_camera_id);
    if(VISCA_result_ok != result) {
        printf("\r\n set set_focus_near failed.");
        return;
    }
    
}

bool sonylens_write_config_params(const CONFIG_PARAMS_t *param)
{
    int len;
    len = eeprom_write(SONYLENS_CONFIG_PARAMS_ADDR, (unsigned char*)param, sizeof(CONFIG_PARAMS_t));
    if(len != sizeof(CONFIG_PARAMS_t))
    {
        printf("\r\n write config params to eeprom failed. len:%d", len);
        return FALSE;
    }
    else
    {
        printf("\r\n write config params to eeprom success.");
        return TRUE;
    }
}

bool sonylens_read_config_params(CONFIG_PARAMS_t *param)
{
    int len;
    len = eeprom_read(SONYLENS_CONFIG_PARAMS_ADDR, (unsigned char*)param, sizeof(CONFIG_PARAMS_t));
    if(len != sizeof(CONFIG_PARAMS_t))
    {
        printf("\r\n read config params from eeprom failed. len:%d", len);
        return FALSE;
    }
    else
    {
        printf("\r\n read config params from eeprom success.");
        return TRUE;
    }
}


void sonylens_reset_default_config_params(void)
{
    sonylens_write_config_params(&default_config_params);
    sonylens_restart();
}

void sonylens_restart(void)
{
    zoom_ratio_set_done = false;
    splash_set_done = false;
    titles_clear_done = false;
    monitor_mode_changed = false;

    sonylens_menu_off();
    sonylens_taskstate = SONY_INIT_CONFIG_PARAMS;
}

void sonylens_key_handle_process(void)
{
    if(!menu_display)
    {
        #if 1
        if(key_condition(KEY_ID_M, 150))
        {
            current_menu = main_menu;
            sonylens_menu_on1(); 
            return;
        }
        #else
        if(key_is_pressed(KEY_ID_M))
        {
            current_menu = main_menu;
            sonylens_menu_on1(); 
            return;
        }
        #endif
       

        if(key_is_pressed(KEY_ID_U))
        {
            // focus near limit +
            //uint32 tempValue = 0;
            //sonylens_get_zoom_value(&tempValue);
            //printf("\r\n current zoom value is :~~~~~~~~~~~~~~~~~~~~~~%08x", tempValue);
            sonylens_set_focus_far();
            return;
        }
        else if(key_is_unpressed(KEY_ID_U))
        {
            sonylens_set_focus_stop();
            return;
        }

        if(key_is_pressed(KEY_ID_D))
        {
            // focus near limit +
            sonylens_set_focus_near();
            return;
        }
        else if(key_is_unpressed(KEY_ID_D))
        {
            sonylens_set_focus_stop();
            return;
        }

        if(key_is_pressed(KEY_ID_L))
        {
            // zoom-
            sonylens_set_zoom_wide();
            return;
        }
        else if(key_is_unpressed(KEY_ID_L))
        {
            sonylens_set_zoom_stop();
            return;
        }

        if(key_is_pressed(KEY_ID_R))
        {
            // zoom-
            sonylens_set_zoom_tele();
            return;
        }
        else if(key_is_unpressed(KEY_ID_R))
        {
            sonylens_set_zoom_stop();
            return;
        }
    }
    else
    {
        SONY_MENU_ITEM_t* menu_item;
        menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];
        if((menu_item->id == MENU_ITEM_DEFAULT) && (menu_item->get() != 0))
        {
            if(key_condition(KEY_ID_M, 500))
            {
                printf("\r\n reset to default config parameters");
                sonylens_reset_default_config_params();
                printf("\r\n reset to default config parameters done.");
    			if(menu_item->set)
    			{
    				 menu_item->set(0);
                     menu_item->ineffective = 0;
    				 sonylens_menu_item_update(current_menu.menu_item_selected);
    			}
                return;
            }
        }
        else
        {
            if(key_is_pressed(KEY_ID_M))
            {
                sonylens_menu_action();
                return;
            }
        }
    
        if(key_is_pressed(KEY_ID_U))
        {
            sonylens_menu_move_updown1(-1);
            return;
        }
        else if(key_is_unpressed(KEY_ID_U))
        {
        }
        

        if(key_is_pressed(KEY_ID_D))
        {
            sonylens_menu_move_updown1(1);
            return;
        }
        else if(key_is_unpressed(KEY_ID_D))
        {
        }

        if(key_is_pressed(KEY_ID_L))
        {
            sonylens_menu_move_leftright(-1);
            return;
        }
        else if(key_is_unpressed(KEY_ID_L))
        {
        }
        

        if(key_is_pressed(KEY_ID_R))
        {
            sonylens_menu_move_leftright(1);
            return;
        }
        else if(key_is_unpressed(KEY_ID_R))
        {
        }
    }

    if(key_condition(KEY_ID_F1, 200))
    {
        int count = sonylens_general_format_get_count();
        int index = sonylens_general_format_get();
        if(2 == index) {
            index = 3;
        } else if(3 == index) {
            index = 5;
        } else if(5 == index) {
            index = 6;
        } else if(6 == index) {
            index = 2;
        } else {
            index = 2;
        }
        sonylens_general_format_set(index);
        sonylens_taskstate = SONY_GET_MONITOR_MODE;
        return;
    }

    if(key_is_pressed(KEY_ID_F2))
    {
        int val = key_freeze;
        sonylens_key_image_freeze_set((val == 0)?1:0);
        return;
    }

    if(key_is_pressed(KEY_ID_F3))
    {
        sonylens_set_exposure_gain_plus();
        return;
    }

    if(key_is_pressed(KEY_ID_F4))
    {
        // situation
        int i = 0;
        int count = sonylens_preset_no_get_count();
        int index = sonylens_preset_no_get();
        for(i = 1; i <= count; i++)
        {
            uint32 zoom_value;
            int preset_no = (index + i) % count;
            sonylens_preset_no_set(preset_no);
            if(sonylens_preset_exist_get())
            {
                sonylens_preset_menu_run_action();
                zoom_value = config_params.preset_conf_list[preset_no].zoom_ratio;

                index = sonylens_get_zoom_index(zoom_value);
                sonylens_tips_display(sonylens_get_zoom_desc(index));
                break;
            }
        }
        return;
    }

    if(key_is_pressed(KEY_ID_F5))
    {
        // bright +
        sonylens_set_bright_plus();
        return;
    }

    if(key_is_pressed(KEY_ID_F6))
    {
        // bright -
        sonylens_set_bright_minus();
        return;
    }

    if(key_is_pressed(KEY_ID_F7))
    {
        // white balance
        bool enable = sonylens_get_auto_mode();
        sonylens_set_auto_mode(enable?false:true);
        return;
    }

    if(key_is_pressed(KEY_ID_F8))
    {
        int val = key_flip;
        sonylens_key_image_flip_set((val == 0)?1:0);
        return;
    }
}


/* zoom set */
char* zoom_speed[] = {
    "0","1","2","3","4","5","6","7" 
};
int sonylens_zoom_speed_get_count(void)
{
    return 8;
}

int sonylens_zoom_speed_get(void) {
    return config_params.zoom.zoom_speed;
}

void sonylens_zoom_speed_set(int param) {
    config_params.zoom.zoom_speed = param;
}

char* sonylens_zoom_speed_right(void)
{
    return zoom_speed[config_params.zoom.zoom_speed];
}


char* dig_zoom[] = {
    "OFF","ON"
};

int sonylens_dig_zoom_get_count(void)
{
    return 2;
}
int sonylens_dig_zoom_get(void)
{
    return config_params.zoom.dig_zoom;
}
char* sonylens_dig_zoom_right(void)
{
    #if 0
    VISCA_result_e result = VISCA_result_ok;
    uint8 enable;
    
    result = visca_get_digital_zoom(sonylens_camera_id, &enable);
    if(result != VISCA_result_ok) {
        printf("\r\n %s failed. result:%d", __FUNCTION__, result);
        return "ERR";
    }
    if(enable == 0x02)
    {
        config_params.zoom.dig_zoom = 1;
    }
    else
    {
        config_params.zoom.dig_zoom = 0;
    }
    #endif
    
    return dig_zoom[config_params.zoom.dig_zoom];
}
void sonylens_dig_zoom_set(int param) {
    VISCA_result_e result = VISCA_result_ok;
    uint8 enable;
    
    if(param == 0) {
        enable = 0x03;
    } else {
        enable = 0x02;
    }
    
    result = visca_set_digital_zoom(sonylens_camera_id, enable);
    if(result != VISCA_result_ok) {
        printf("\r\n sonylens_dig_zoom_set failed. result:%d", result);
        return;
    }

    config_params.zoom.dig_zoom = param;
}

//int current_osd = 0;
char* osd[] = {
    "OFF","ON"
};
int sonylens_osd_get_count(void)
{
    return 2;
}
int sonylens_osd_get(void)
{
    return config_params.zoom.osd;
}
void sonylens_osd_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 enable;
    
    if(param == 0) {
        enable = 0x03;
    } else {
        enable = 0x02;
    }
    
    result = visca_set_zoom_osd(sonylens_camera_id, enable);
    if(result != VISCA_result_ok) {
        printf("\r\n sonylens_osd_set failed. result:%d", result);
        return;
    }

    config_params.zoom.osd = param;
}
char* sonylens_osd_right(void)
{
    return osd[config_params.zoom.osd];
}

char* zoom_display[] = {
    "OFF","ON"
};
int sonylens_zoom_display_get_count(void)
{
    return 2;
}
int sonylens_zoom_display_get(void)
{
    return config_params.zoom.zoom_display;
}
void sonylens_zoom_display_set(int param)
{
    config_params.zoom.zoom_display = param;
}
char* sonylens_zoom_display_right(void)
{
    return zoom_display[config_params.zoom.zoom_display];
}

/* focus menu */
//int current_focus_mode = 0;
char* focus_mode[] = {
    "AUTO","MANUAL"
};
int sonylens_focus_mode_get_count(void)
{
    return 2;
}
int sonylens_focus_mode_get(void)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 mode;
    result = visca_get_focus_mode(sonylens_camera_id, &mode);
    if(result != VISCA_result_ok) {
        return config_params.focus.mode;
    }

    if(mode == 0x02) {
        config_params.focus.mode = 0;
    } else {
        config_params.focus.mode = 1;
    }
    
    return config_params.focus.mode;
}
void sonylens_focus_mode_set(int value)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 mode;

    if(value == 0) {
        mode = 0x02;
    } else {
        mode = 0x03;
    }

    result = visca_set_focus_mode(sonylens_camera_id, mode);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.focus.mode = value;
}
char* sonylens_focus_mode_right(void)
{
    return focus_mode[config_params.focus.mode];
}

char* focus_af_sens[] = {
    "NORMAL","LOW"
};
int sonylens_focus_af_sens_get_count(void)
{
    return 2;
}
int sonylens_focus_af_sens_get(void)
{
    return config_params.focus.af_sens;
}
void sonylens_focus_af_sens_set(int value)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 mode;

    if(value == 0) {
        mode = 0x02;
    } else {
        mode = 0x03;
    }

    result = visca_set_focus_af_sens(sonylens_camera_id, mode);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.focus.af_sens = value;
}
char* sonylens_focus_af_sens_right(void)
{
    return focus_af_sens[config_params.focus.af_sens];
}

char* focus_light[] = {
    "NORMAL","IR"
};
int sonylens_focus_light_get_count(void)
{
    return 2;
}
int sonylens_focus_light_get(void)
{
    return config_params.focus.focus_light;
}
void sonylens_focus_light_set(int value)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 mode;

    if(value == 0) {
        mode = 0x00;
    } else {
        mode = 0x01;
    }

    result = visca_set_focus_light(sonylens_camera_id, mode);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.focus.focus_light = value;
}
char* sonylens_focus_light_right(void)
{
    return focus_light[config_params.focus.focus_light];
}


//int current_focus_near_limit = 12; //(initial setting)
char* focus_near_limit[15] = {
    "OVER", 
    "25M","11M","7M","4.9M","3.7M",
    "2.9M","2.3M","1.85M","1.5M","1.23M",
    "1.0M","30CM","8CM","1CM"
};
int sonylens_focus_near_limit_get_count(void)
{
    return 15;
}
int sonylens_focus_near_limit_get(void)
{
    return config_params.focus.near_limit;
}
void sonylens_focus_near_limit_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value;

    value = (param + 1) << 12;

    result = visca_set_focus_near_limit(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.focus.near_limit = param;
}
char* sonylens_focus_near_limit_right(void)
{
    return focus_near_limit[config_params.focus.near_limit];
}

//int current_exposure_ae_mode = 0;
char* exposure_ae_mode[2] = {
    "AUTO", "MANUAL"
};

int sonylens_exposure_ae_mode_get_count(void)
{
    return 2;
}
int sonylens_exposure_ae_mode_get(void)
{
    #if 0
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;
    uint8 index = 0;
    result = visca_get_exposure_ae_mode(sonylens_camera_id, &value);
    if(result != VISCA_result_ok) {
        return;
    }

    switch(value)
    {
        case 0:
            index = 0;
            break;
        case 3:
            index = 1;
            break;
        default:
            break;
    }

    config_params.exposure.ae_mode = index;
    #endif
    return config_params.exposure.ae_mode;
}
void sonylens_exposure_ae_mode_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;

    if(param == 0)
    {
        value = 0x00;
    }
    else
    {
        value = 0x03;
    }
    
    result = visca_set_exposure_ae_mode(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.exposure.ae_mode = param;

    mode_manager.exposure_mode = config_params.exposure.ae_mode;
}
char* sonylens_exposure_ae_mode_right(void)
{
    return exposure_ae_mode[config_params.exposure.ae_mode];
}

char* exposure_shutter[] = {
    "1/1", "1/2", "1/4", "1/8", "1/15",
    "1/30", "1/60", "1/90", "1/100", "1/125",
    "1/180", "1/250", "1/350", "1/500", "1/720",
    "1/1000", "1/1500", "1/2000", "1/3000", "1/4000",
    "1/6000", "1/10000",
};

char* exposure_shutter2[] = {
    "1/1", "1/2", "1/3", "1/6", "1/12",
    "1/25", "1/50", "1/75", "1/100", "1/120",
    "1/150", "1/215", "1/300", "1/420", "1/600",
    "1/1000", "1/1250", "1/1750", "1/2500", "1/3500",
    "1/6000", "1/10000",
};

int sonylens_exposure_shutter_get_count(void)
{
    return 22;
}
int sonylens_exposure_shutter_get(void)
{
    return config_params.exposure.shutter;
}
void sonylens_exposure_shutter_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = 0;

    value = param;
    
    result = visca_set_exposure_shutter(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.exposure.shutter = param;
}
char* sonylens_exposure_shutter_right(void)
{
    switch(config_params.general.format)
    {
        case 1:
		case 3:
		case 5:
		case 7:
		case 9:
            return exposure_shutter2[config_params.exposure.shutter];
        default:
            return exposure_shutter[config_params.exposure.shutter];
            break;
    }    
}


char* exposure_iris[] = {
    "F14", "F11", "F9.6", "F8",
    "F6.8", "F5.6", "F4.8", "F4", "F3.4",
    "F2.8", "F2.4", "F2", "F1.6"
};
int sonylens_exposure_iris_get_count(void)
{
    return 13;
}
int sonylens_exposure_iris_get(void)
{
    return config_params.exposure.iris;
}
void sonylens_exposure_iris_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = 0;

    if(param == 0)
    {
        value = 0;
    }
    else
    {
        value = param + 4;
    }
    
    result = visca_set_exposure_iris(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.exposure.iris = param;
    mode_manager.iris = config_params.exposure.iris;
    mode_manager.current_iris = config_params.exposure.iris;
}
char* sonylens_exposure_iris_right(void)
{
    return exposure_iris[config_params.exposure.iris];
}

//int current_exposure_gain = 0;
char* exposure_gain[15] = {
    "0DB", "3.4DB", "6.8DB", "10.2DB", "13.7DB",
    "17.1DB", "20.5DB", "23.9DB", "27.3DB", "30.7DB",
    "34.1DB", "37.5DB", "40.1DB", "44.4DB", "47.8DB",
};

int sonylens_exposure_gain_get_count(void)
{
    return 15;
}
int sonylens_exposure_gain_get(void)
{
    return config_params.exposure.gain;
}
void sonylens_exposure_gain_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = 0;

    value = param + 1;
    
    result = visca_set_exposure_gain(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.exposure.gain = param;
    key_exp_gain = param;
}
char* sonylens_exposure_gain_right(void)
{
    return exposure_gain[config_params.exposure.gain];
}

int sonylens_exposure_bri_get_count(void)
{
    return 100;
}
int sonylens_exposure_bri_get(void)
{
    return config_params.exposure.bri;
}
void sonylens_exposure_bri_set(int param)
{
    config_params.exposure.bri = param;
}
char* sonylens_exposure_bri_right(void)
{
    return Int2DecStr(config_params.exposure.bri);
}

//int current_exposure_slow_ae = 0;
char* exposure_slow_ae[48] = {
    "1", "2", "3", "4", "5","6", "7", "8", "9", "10",
    "11", "12", "13", "14", "15","16", "17", "18", "19", "20",
    "21", "22", "23", "24", "25","26", "27", "28", "29", "30",
    "31", "32", "33", "34", "35","36", "37", "38", "39", "40",
    "41", "42", "43", "44", "45","46", "47", "48"
};
int sonylens_exposure_slow_ae_get_count(void)
{
    return 48;
}
int sonylens_exposure_slow_ae_get(void)
{
    return config_params.exposure.slow_ae;
}
void sonylens_exposure_slow_ae_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;

    value = param + 1;
    
    result = visca_set_exposure_slow_ae(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.exposure.slow_ae = param;
}
char* sonylens_exposure_slow_ae_right(void)
{
    return exposure_slow_ae[config_params.exposure.slow_ae];
}

//int current_exposure_slow_shutter = 0;
char* exposure_slow_shutter[2] = {
    "OFF", "ON"
};

int sonylens_exposure_slow_shutter_get_count(void)
{
    return 2;
}
int sonylens_exposure_slow_shutter_get(void)
{
    return config_params.exposure.slow_shutter;
}
void sonylens_exposure_slow_shutter_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;

    if(0 == param)
    {
        value = 0x03;
    }
    else
    {
        value = 0x02;
    }

    result = visca_set_exposure_slow_shutter(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.exposure.slow_shutter = param;
}
char* sonylens_exposure_slow_shutter_right(void)
{
    return exposure_slow_shutter[config_params.exposure.slow_shutter];
}

//int current_wb_mode = 0;
char* wb_mode[] = {
    "ATW", "MANUAL", "LAM FIX", "OUT AUTO", "AUTO"
};
int sonylens_wb_mode_get_count(void)
{
    return 5;
}
int sonylens_wb_mode_get(void)
{
    #if 0
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;
    uint8 index = 0;
    result = visca_get_wb_mode(sonylens_camera_id, &value);
    if(result != VISCA_result_ok) {
        return;
    }

    switch(value)
    {
        case 0:
            index = 4;
            break;
        case 4:
            index = 0;
            break;
        case 5:
            index = 1;
            break;
        case 6:
            index = 3;
            break;
        case 8:
            index = 2;
            break;
        default:
            break;
    }
    config_params.wb.wb_mode = index;
    #endif
    return config_params.wb.wb_mode;
}
void sonylens_wb_mode_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;

    if(0 == param)
    {
        value = 0x04;
    }
    else if(1 == param)
    {
        value = 0x05;
    }
    else if(2 == param)
    {
        value = 0x08;
    }
    else if(3 == param)
    {
        value = 0x06;
    }
    else
    {
        value = 0x00;
    }

    result = visca_set_wb_mode(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.wb.wb_mode = param;

    mode_manager.wb_mode = config_params.wb.wb_mode;
}

char* sonylens_wb_mode_right(void)
{
    return wb_mode[config_params.wb.wb_mode];
}

int sonylens_wb_red_gain_get_count(void)
{
    return 256;
}
int sonylens_wb_red_gain_get(void)
{
    return config_params.wb.red_gain;
}
void sonylens_wb_red_gain_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = param;

    result = visca_set_wb_red_gain(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    
    config_params.wb.red_gain = param;
}
char* sonylens_wb_red_gain_right(void)
{
    return Int2DecStr(config_params.wb.red_gain);
}


int sonylens_wb_blue_gain_get_count(void)
{
    return 256;
}
int sonylens_wb_blue_gain_get(void)
{
    return config_params.wb.blue_gain;
}
void sonylens_wb_blue_gain_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = param;

    result = visca_set_wb_blue_gain(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    
    config_params.wb.blue_gain = param;
}
char* sonylens_wb_blue_gain_right(void)
{
    return Int2DecStr(config_params.wb.blue_gain);
}


void sonylens_wb_one_push_action(void)
{
    VISCA_result_e result = VISCA_result_ok;

    //result = visca_set_wb_one_push(sonylens_camera_id);
    if(result != VISCA_result_ok) {
        return;
    }
}


/* advance */
//int current_advance_blc = 0;
char* advance_blc[2] = {
    "OFF", "ON"
};
int sonylens_advance_blc_get_count(void)
{
    return 2;
}
int sonylens_advance_blc_get(void)
{
    return config_params.advance.blc;
}
void sonylens_advance_blc_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;

    if(param == 0)
    {
        value = 0x03;
    }
    else
    {
        value = 0x02;
    }
   
    result = visca_set_advance_blc(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.advance.blc = param;
}
char* sonylens_advance_blc_right(void)
{
    return advance_blc[config_params.advance.blc];
}

bool advance_expcomp_on = false;
void sonylens_advance_expcomp_on(bool e)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 enable = 0;

    if(e)
    {
        enable = 0x02;
    }
    else
    {
        enable = 0x03;        
    }

    result = visca_set_advance_expcomp(sonylens_camera_id, enable);
    if(result != VISCA_result_ok) {
        return;
    }

    advance_expcomp_on = e;
}

char* advance_expcomp[] = {
    "-7", "-6", "-5", "-4", "-3", "-2", "-1", 
    "0", "1", "2", "3", "4", "5", "6", "7"
};

int sonylens_advance_expcomp_get_count(void)
{
    return 15;
}
int sonylens_advance_expcomp_get(void)
{
    return config_params.advance.expcomp;
}
void sonylens_advance_expcomp_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 enable = 0;
    uint32 value = 0;

    if(!advance_expcomp_on)
    {
        sonylens_advance_expcomp_on(true);
    }

    value = param;
    result = visca_set_advance_expcomp_value(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }

   
    config_params.advance.expcomp = param;
    
    mode_manager.expcomp = config_params.advance.expcomp;
    mode_manager.current_iris = config_params.exposure.iris;
}

char* sonylens_advance_expcomp_right(void)
{
    return advance_expcomp[config_params.advance.expcomp];
}

//int current_advance_wdr = 0;
char* advance_wdr[3] = {
    "OFF", "ON", "AUTO"
};
int sonylens_advance_wdr_get_count(void)
{
    return 3;
}
int sonylens_advance_wdr_get(void)
{
    return config_params.advance.wdr;
}
void sonylens_advance_wdr_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = 0;

    //value = param << 12;

    /*
    result = visca_set_wb_blue_gain(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    */
    config_params.advance.wdr = param;
}
char* sonylens_advance_wdr_right(void)
{
    return advance_wdr[config_params.advance.wdr];
}

//int current_advance_ircut = 0;
char* advance_ircut[3] = {
    "AUTO", "MANUAL", "IR SYNC"
};
int sonylens_advance_ircut_get_count(void)
{
    return 3;
}
int sonylens_advance_ircut_get(void)
{
    return config_params.advance.ircut;
}
void sonylens_advance_ircut_set(int param)
{
    config_params.advance.ircut = param;
}
char* sonylens_advance_ircut_right(void)
{
    return advance_ircut[config_params.advance.ircut];
}

//int current_advance_nr_level = 0;
char* advance_nr_level[6] = {
    "OFF", "1", "2", "3", "4", "5"
};

int sonylens_advance_nr_level_get_count(void)
{
    return 6;
}
int sonylens_advance_nr_level_get(void)
{
    return config_params.advance.nr_level;
}
void sonylens_advance_nr_level_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;

    value = param;

    result = visca_set_advance_nr_level(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    
    config_params.advance.nr_level = param;
}
char* sonylens_advance_nr_level_right(void)
{
    return advance_nr_level[config_params.advance.nr_level];
}


//int current_advance_stabilizer = 0;
char* advance_stabilizer[2] = {
    "OFF", "ON"
};
int sonylens_advance_stabilizer_get_count(void)
{
    return 2;
}
int sonylens_advance_stabilizer_get(void)
{
    return config_params.advance.stabilizer;
}
void sonylens_advance_stabilizer_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;

    if(0 == param)
    {
        value = 0x03;
    }
    else
    {
        value = 0x02;
    }

    //result = visca_set_advance_stabilizer(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.advance.stabilizer = param;
}
char* sonylens_advance_stabilizer_right(void)
{
    return advance_stabilizer[config_params.advance.stabilizer];
}


//int current_advance_defog_mode = 0;
char* advance_defog_mode[] = {
    "OFF", "LOW", "MID", "HIGH"
};
int sonylens_advance_defog_mode_get_count(void)
{
    return 4;
}
int sonylens_advance_defog_mode_get(void)
{
    return config_params.advance.defog_mode;
}
void sonylens_advance_defog_mode_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;
    uint8 level = 0;

    if(0 == param)
    {
        value = 0x03;
    }
    else if(1 == param)
    {
        value = 0x02;
        level = 0x01;
    }
    else if(2 == param)
    {
        value = 0x02;
        level = 0x02;
    }
    else
    {
        value = 0x02;
        level = 0x03;
    }

    result = visca_set_advance_defog_mode(sonylens_camera_id, value, level);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.advance.defog_mode = param;
}
char* sonylens_advance_defog_mode_right(void)
{
    return advance_defog_mode[config_params.advance.defog_mode];
}

//int current_advance_alarm_out = 0;
char* advance_alarm_out[5] = {
    "OFF", "A", "B", "C", "D"
};
int sonylens_advance_alarm_out_get_count(void)
{
    return 5;
}
int sonylens_advance_alarm_out_get(void)
{
    return config_params.advance.alarm_out;
}
void sonylens_advance_alarm_out_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = 0;

    config_params.advance.alarm_out = param;
}
char* sonylens_advance_alarm_out_right(void)
{
    return advance_alarm_out[config_params.advance.alarm_out];
}


/* image */
//int current_image_aperture = 0;
int sonylens_image_aperture_get_count(void)
{
    return 16;
}
int sonylens_image_aperture_get(void)
{
    return config_params.image.aperture;
}
void sonylens_image_aperture_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = param;

    result = visca_set_image_aperture(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.image.aperture = param;
}
char* sonylens_image_aperture_right(void)
{
    return Int2DecStr(config_params.image.aperture);
}

//int current_image_color_gain = 0;
int sonylens_image_color_gain_get_count(void)
{
    return 15;
}
int sonylens_image_color_gain_get(void)
{
    return config_params.image.color_gain;
}
void sonylens_image_color_gain_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = param;

    result = visca_set_image_color_gain(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    
    config_params.image.color_gain = param;
}
char* sonylens_image_color_gain_right(void)
{
    return Int2DecStr(config_params.image.color_gain);
}



//int current_image_color_hue = 0;
int sonylens_image_color_hue_get_count(void)
{
    return 15;
}
int sonylens_image_color_hue_get(void)
{
    return config_params.image.color_hue;
}
void sonylens_image_color_hue_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = param;

    result = visca_set_image_color_hue(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    
    config_params.image.color_hue = param;
}
char* sonylens_image_color_hue_right(void)
{
    return Int2DecStr(config_params.image.color_hue);
}


//int current_image_chroma_suppress = 0;
int sonylens_image_chroma_suppress_get_count(void)
{
    return 4;
}
int sonylens_image_chroma_suppress_get(void)
{
    return config_params.image.chroma_suppress;
}
void sonylens_image_chroma_suppress_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = param;

    result = visca_set_image_chroma_suppress(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    
    config_params.image.chroma_suppress = param;
}
char* sonylens_image_chroma_suppress_right(void)
{
    return Int2DecStr(config_params.image.chroma_suppress);
}


int sonylens_image_gamma_get_count(void)
{
    return 81;
}
int sonylens_image_gamma_get(void)
{
    return config_params.image.gamma;
}
void sonylens_image_gamma_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    int value = param - 16; 
 
    result = visca_set_image_gamma(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.image.gamma = param;
}
char* sonylens_image_gamma_right(void)
{
    Int2DecStr(config_params.image.gamma - 16);
}

char* image_mirror[2] = {
    "OFF", "ON"
};

int sonylens_image_mirror_get_count(void)
{
    return 2;
}
int sonylens_image_mirror_get(void)
{
    return config_params.image.mirror;
}
void sonylens_image_mirror_set(int param)
{
    VISCA_result_e result;
    uint8 mirror;
    

    // set the mirror
    if(param == 0) {
        mirror = 0x03;
    } else {
        mirror = 0x02;
    }

    result = visca_set_image_mirror(sonylens_camera_id, mirror);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.image.mirror = param;
}
char* sonylens_image_mirror_right(void)
{
    return image_mirror[config_params.image.mirror];
}


//int current_image_flip = 0;
char* image_flip[2] = {
    "OFF", "ON"
};

int sonylens_image_flip_get_count(void)
{
    return 2;
}
int sonylens_image_flip_get(void)
{
    return config_params.image.flip;
}
void sonylens_image_flip_set(int param)
{
    VISCA_result_e result;
    uint8 mirror;
    

    // set the mirror
    if(param == 0) {
        mirror = 0x03;
    } else {
        mirror = 0x02;
    }

    result = visca_set_image_flip(sonylens_camera_id, mirror);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.image.flip = param;
    key_flip = config_params.image.flip;
}
char* sonylens_image_flip_right(void)
{
    return image_flip[config_params.image.flip];
}


//int current_image_freeze = 0;
char* image_freeze[2] = {
    "OFF", "ON"
};

int sonylens_image_freeze_get_count(void)
{
    return 2;
}
int sonylens_image_freeze_get(void)
{
    return config_params.image.freeze;
}
void sonylens_image_freeze_set(int param)
{
    VISCA_result_e result;
    uint8 mirror;
    
    if(param == 0) {
        mirror = 0x03;
    } else {
        mirror = 0x02;
    }

    result = visca_set_image_freeze(sonylens_camera_id, mirror);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.image.freeze = param;

    key_freeze = config_params.image.freeze;
}
char* sonylens_image_freeze_right(void)
{
    return image_freeze[config_params.image.freeze];
}


//int current_image_pic_effect = 0;
char* image_pic_effect[] = {
    "OFF", "B.W", "NEG.ART"
};
int sonylens_image_pic_effect_get_count(void)
{
    return 3;
}
int sonylens_image_pic_effect_get(void)
{
    return config_params.image.pic_effect;
}
void sonylens_image_pic_effect_set(int param)
{
    VISCA_result_e result;
    uint8 value;
    

    // set the mirror
    if(param == 0)
    {
        value = 0x00;
    }
    else if(param == 1)
    {
        value = 0x04;
    }
    else
    {
        value = 0x02;
    }

    result = visca_set_image_pic_effect(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.image.pic_effect = param;
}
char* sonylens_image_pic_effect_right(void)
{
    return image_pic_effect[config_params.image.pic_effect];
}



/* general */
//int current_general_format = 0;
char* general_format[] = {
    #if 1
    "1080P60", "1080P50", "1080P30", "1080P25", 
    "1080I60", "1080I50", 
    "720P60", "720P50", "720P30", "720P25",
    #endif
};

uint8 sonylens_get_monitor_mode(int index)
{
    uint8 value = 0x8;
    switch(index)
    {
        case 0:
            value = 0x15;
            break;
        case 1:
            value = 0x14;
            break;
        case 2:
            value = 0x6;
            break;
        case 3:
            value = 0x8;
            break;
        case 4:
            value = 0x2;
            break;
        case 5:
            value = 0x4;
            break;
        case 6:
            value = 0x9;
            break;
        case 7:
            value = 0xC;
            break;
        case 8:
            value = 0xF;
            break;
        case 9:
            value = 0x11;
            break;
        case 10:
            value = 0x13;
            break;
    }

    return value;
}

int sonylens_general_format_get_count(void)
{
    return 10;
}
int sonylens_general_format_get(void)
{
    return config_params.general.format;
}
void sonylens_general_format_set(int param)
{  
    config_params.general.format = param;
}
char* sonylens_general_format_right(void)
{
    return general_format[config_params.general.format];
}

//int current_general_lr_hand = 0;
char* general_lr_hand[2] = {
    "OFF", "ON"
};

int sonylens_general_lr_hand_get_count(void)
{
    return 2;
}
int sonylens_general_lr_hand_get(void)
{
    return config_params.general.lr_hand;
}
void sonylens_general_lr_hand_set(int param)
{
    config_params.general.lr_hand = param;
}
char* sonylens_general_lr_hand_right(void)
{
    return general_lr_hand[config_params.general.lr_hand];
}

//int current_general_protocol = 0;
char* general_protocol[] = {
    "PELCO-D/P", "VISCA"
};
int sonylens_general_protocol_get_count(void)
{
    return 2;
}
int sonylens_general_protocol_get(void)
{
    return config_params.general.protocol;
}
void sonylens_general_protocol_set(int param)
{
    if(0 == param)
    {
        pelco_d_init();
    }
    else if(1 == param)
    {
        sony_visca_init();
    }
    else
    {
        param = 0;
        pelco_d_init();
    }
    config_params.general.protocol = param;
}
char* sonylens_general_protocol_right(void)
{
    return general_protocol[config_params.general.protocol];
}


//int current_general_baudrate = 0;
char* general_baudrate[5] = {
    "2400", "4800", "9600", "19200", "38400"
};
int sonylens_general_baudrate_get_count(void)
{
    return 5;
}
int sonylens_general_baudrate_get(void)
{
    return config_params.general.baudrate;
}
void sonylens_general_baudrate_set(int param)
{
    int baudrate = 9600;
    switch(param)
    {
        case 0:
            baudrate = 2400;
            break;
        case 1:
            baudrate = 4800;
            break;
        case 2:
            baudrate = 9600;
            break;
        case 3:
            baudrate = 19200;
            break;
        case 4:
            baudrate = 38400;
            break;
        default:
            baudrate = 9600;
            break;
    }
    UART3_set_baudrate(baudrate);
    config_params.general.baudrate = param;
}
char* sonylens_general_baudrate_right(void)
{
    return general_baudrate[config_params.general.baudrate];
}


//int current_general_address = 0;
int sonylens_general_address_get_count(void)
{
    return 255;
}
int sonylens_general_address_get(void)
{
    return config_params.general.address;
}
void sonylens_general_address_set(int param)
{
    config_params.general.address = param;
}
char* sonylens_general_address_right(void)
{
    return Int2DecStr(config_params.general.address + 1);
}

int sonylens_address_get(void)
{
    return config_params.general.address + 1;
}


//int current_general_aux = 0;
char* general_aux[2] = {
    "OFF", "ON"
};

int sonylens_general_aux_get_count(void)
{
    return 2;
}
int sonylens_general_aux_get(void)
{
    return config_params.general.aux;
}
void sonylens_general_aux_set(int param)
{
    config_params.general.aux = param;
}
char* sonylens_general_aux_right(void)
{
    return general_aux[config_params.general.aux];
}

void sonylens_main_menu_init(void)
{
    int i = 0;
    SONY_MENU_ITEM_t* menu_item;
    int menu_count = current_menu.menu_item_count;

    for(i = 0; i < menu_count; i++)
    {
        menu_item = &current_menu.menu_item_list[i];
        if(menu_item->id == MENU_ITEM_DEFAULT)
        {
            // default;
            menu_item->set(0);
        }
    }
}

void sonylens_zoom_menu_init(void)
{
    SONY_MENU_ITEM_t* menu_item;
    int menu_count = current_menu.menu_item_count;
}

void sonylens_focus_menu_init(void)
{
    if(current_menu.menu_item_list[0].get)
    {
        #if 0
        int mode = current_menu.menu_item_list[0].get();
        if(0 == mode)
        {
            current_menu.menu_item_list[1].disable = 1;
            current_menu.menu_item_list[2].disable = 1;
            current_menu.menu_item_list[3].disable = 1;
        }
        else
        {
            current_menu.menu_item_list[1].disable = 0;
            current_menu.menu_item_list[2].disable = 0;
            current_menu.menu_item_list[3].disable = 0;
        }
        #endif
    }
}
void sonylens_exposure_menu_init(void)
{
    if(current_menu.menu_item_list[0].get)
    {
        int mode = current_menu.menu_item_list[0].get();
        if(0 == mode)
        {
            current_menu.menu_item_list[1].disable = 1;
            current_menu.menu_item_list[2].disable = 1;
            current_menu.menu_item_list[3].disable = 1;
            current_menu.menu_item_list[4].disable = 1;
        }
        else
        {
            current_menu.menu_item_list[1].disable = 0;
            current_menu.menu_item_list[1].set(current_menu.menu_item_list[1].get());
            
            current_menu.menu_item_list[2].disable = 0;
            current_menu.menu_item_list[2].set(current_menu.menu_item_list[2].get());

            current_menu.menu_item_list[3].disable = 0;
            current_menu.menu_item_list[3].set(current_menu.menu_item_list[3].get());

            current_menu.menu_item_list[4].disable = 1;
        }
    }
}
void sonylens_wb_menu_init(void)
{
    if(current_menu.menu_item_list[0].get)
    {
        int mode = current_menu.menu_item_list[0].get();
        if(1 == mode)
        {
            current_menu.menu_item_list[1].disable = 0;
            current_menu.menu_item_list[1].set(current_menu.menu_item_list[1].get());
            
            current_menu.menu_item_list[2].disable = 0;
            current_menu.menu_item_list[2].set(current_menu.menu_item_list[2].get());
        }
        else
        {
            current_menu.menu_item_list[1].disable = 1;
            current_menu.menu_item_list[2].disable = 1;
        }
    }
}
void sonylens_advance_menu_init(void)
{
}
void sonylens_image_menu_init(void)
{
}
void sonylens_general_menu_init(void)
{
}

void sonylens_exit_menu_init(void)
{
}

void sonylens_privacy_menu_init(void)
{
}
void sonylens_camera_menu_init(void)
{
}
void sonylens_preset_menu_init(void)
{
    config_params.general.preset_conf_no = 0;
}

void sonylens_main_menu_zoom_set_action(void)
{
    current_menu = zoom_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}

void sonylens_main_menu_focus_set_action(void)
{
    current_menu = focus_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}

void sonylens_main_menu_exposure_set_action(void)
{
    current_menu = exposure_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}
void sonylens_main_menu_wb_set_action(void)
{
    current_menu = wb_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}
void sonylens_main_menu_advance_set_action(void)
{
    current_menu = advance_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}
void sonylens_main_menu_image_set_action(void)
{
    current_menu = image_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}
void sonylens_main_menu_general_set_action(void)
{
    current_menu = general_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}
void sonylens_main_menu_exit_action(void)
{
    current_menu = exit_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1();
}

void sonylens_advance_menu_privacy_action(void)
{
    current_menu = privacy_set_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}
void sonylens_general_menu_camera_action(void)
{
    current_menu = camera_id_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1();
}
void sonylens_general_menu_preset_action(void)
{
    current_menu = preset_conf_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1();
}


void sonylens_zoom_menu_back_action(void)
{
    current_menu = main_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}

void sonylens_focus_menu_back_action()
{
    current_menu = main_menu;
    current_menu.menu_item_selected = 1;
    sonylens_menu_on1(); 
}
void sonylens_exposure_menu_back_action()
{
    current_menu = main_menu;
    current_menu.menu_item_selected = 2;
    sonylens_menu_on1(); 
}
void sonylens_wb_menu_back_action()
{
    current_menu = main_menu;
    current_menu.menu_item_selected = 3;
    sonylens_menu_on1(); 
}
void sonylens_advance_menu_back_action()
{
    current_menu = main_menu;
    current_menu.menu_item_selected = 4;
    sonylens_menu_on1(); 
}
void sonylens_image_menu_back_action()
{
    current_menu = main_menu;
    current_menu.menu_item_selected = 5;
    sonylens_menu_on1(); 
}
void sonylens_general_menu_back_action()
{
    current_menu = main_menu;
    current_menu.menu_item_selected = 6;
    sonylens_menu_on1(); 
}

void sonylens_privacy_menu_back_action()
{
    current_menu = advance_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}
void sonylens_camera_menu_back_action()
{
    current_menu = general_menu;
    current_menu.menu_item_selected = 0;
    sonylens_menu_on1(); 
}
void sonylens_preset_menu_back_action()
{
    current_menu = general_menu;
    current_menu.menu_item_selected = 6;
    sonylens_menu_on1(); 
}


/* default */
char* default_value[] = {
    "", "CONFIRM"
};
int current_default_value = 0;
int sonylens_default_get_count(void)
{
    return 2;
}

void sonylens_default_set(int param)
{
    current_default_value = param;
}
int sonylens_default_get(void)
{
    return current_default_value;
}
void sonylens_default_action(void)
{
    if(current_default_value == 0)
    {
        current_default_value = 1;
    }
    printf("\r\n %s() is called.", __FUNCTION__);
}

char* sonylens_default_right(void)
{
    return default_value[current_default_value];
}
#if 0
void sonylens_default_set(void)
{
    VISCA_title_t t;
    SONY_MENU_ITEM_t* menu_item;
    int len;
    int j;
    char* confirm = "CONFIRM";
    
    menu_item = &current_menu.menu_item_list[current_menu.menu_item_selected];
    if(menu_item->ineffective == 1)
    {
        return;
    }
    menu_item->ineffective = 1;
    //sonylens_menu_update(current_menu.menu_item_selected);
    len = strlen(menu_item->name);
     
    t.vposition = current_menu.menu_item_selected + 1;
    t.hposition = 0;
    t.blink = 0;
    t.color = 0;
        
    memset(t.title, 0x1B, sizeof(t.title));

    if(menu_item->ineffective)
    {
        t.title[0] = sonylens_get_char_index('?');
    }
    else
    {
        t.title[0] = sonylens_get_char_index('-');
    }
    
    for(j = 0; j < len; j++) {
        t.title[j+1] = sonylens_get_char_index(menu_item->name[j]);
    }
    
    len = strlen(confirm);
    
    for(j = 0; j < len; j++) {
        t.title[20 - len + j] = sonylens_get_char_index(confirm[j]);
    }
    
    sonylens_print_msg(t.title, sizeof(t.title));
    sonylens_set_title(&t);
}
#endif
/* privacy */
//int current_privacy_zone = 0;
char* privacy_zone[4] = {
    "A", "B", "C", "D"
};
int sonylens_privacy_zone_get_count(void)
{
    return 4;
}
int sonylens_privacy_zone_get(void)
{
    return config_params.advance.privacy_zone_no;
}
void sonylens_privacy_zone_set(int param)
{
    config_params.advance.privacy_zone_no = param;
}
char* sonylens_privacy_zone_right(void)
{
    return privacy_zone[config_params.advance.privacy_zone_no];
}

#if 0
void sonylens_privacy_zone_set(int param){
    VISCA_result_e result = VISCA_result_ok;

    PRIVACY_ZONE_t* zone = &privacy_zone_list[param];
    
    result = visca_set_privacy_non_interlock_mask(sonylens_camera_id, param, zone->x, zone->y, zone->w, zone->h);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.advance.privacy.no = param;
    config_params.advance.privacy.display = privacy_zone_list[param].display;
    config_params.advance.privacy.x = privacy_zone_list[param].x;
    config_params.advance.privacy.y = privacy_zone_list[param].y;
    config_params.advance.privacy.w = privacy_zone_list[param].w;
    config_params.advance.privacy.h = privacy_zone_list[param].h;
}
void sonylens_privacy_zone_get(int* param, char* title){
    int i = 0;
    int len = 0;
    char* str;
    
    *param = config_params.advance.privacy.no;
    str = privacy_zone[config_params.advance.privacy.no];
    len = strlen(str);
    
    for(i = 0; i < len; i++) {
        title[20 - len + i] = sonylens_get_char_index(str[i]);
    }

    //0x76
}
#endif

//int current_privacy_display = 0;
char* privacy_display[2] = {
    "OFF", "ON"
};

int sonylens_privacy_display_get_count(void)
{
    return 2;
}
int sonylens_privacy_display_get(void)
{
    uint8 index = config_params.advance.privacy_zone_no;
    return config_params.privacy_zone_list[index].display;
}
void sonylens_privacy_display_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 display = 0;
    uint8 index = config_params.advance.privacy_zone_no;
    
    result = visca_get_privacy_display(sonylens_camera_id, &display);
    if(result != VISCA_result_ok) {
        return;
    }

    printf("\r\n the privacy display is:0x%x", display);

    if(param == 0)
    {
        display = display & (~(1 << index));
    }
    else
    {
        display = display | (1 << index);
    }

    printf("\r\n the new privacy display is:0x%x", display);

    result = visca_set_privacy_display(sonylens_camera_id, display);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.privacy_zone_list[index].display = param;
}
char* sonylens_privacy_display_right(void)
{
    uint8 index = config_params.advance.privacy_zone_no;
    return privacy_display[config_params.privacy_zone_list[index].display];
}

#if 0
void sonylens_privacy_display_set(int param){
    VISCA_result_e result = VISCA_result_ok;
    uint32 display = 0;
    result = visca_get_privacy_display(sonylens_camera_id, &display);
    if(result != VISCA_result_ok) {
        return;
    }

    printf("\r\n the privacy display is:0x%x", display);

    if(param == 0)
    {
        display = display & (~(1 << config_params.advance.privacy.no));
    }
    else
    {
        display = display | (1 << config_params.advance.privacy.no);
    }

    printf("\r\n the new privacy display is:0x%x", display);

    result = visca_set_privacy_display(sonylens_camera_id, display);
    if(result != VISCA_result_ok) {
        return;
    }
    config_params.advance.privacy.display = param;
    privacy_zone_list[config_params.advance.privacy.no].display = param;
}
void sonylens_privacy_display_get(int* param, char* title){
    int i = 0;
    int len = 0;
    char* str;
    
    *param = config_params.advance.privacy.display;
    str = privacy_display[config_params.advance.privacy.display];
    len = strlen(str);
    
    for(i = 0; i < len; i++) {
        title[20 - len + i] = sonylens_get_char_index(str[i]);
    }
}
#endif

//int current_privacy_x = 0;
int sonylens_privacy_x_get_count(void)
{
    return 256;
}
int sonylens_privacy_x_get(void)
{
    uint8 index = config_params.advance.privacy_zone_no;
    return config_params.privacy_zone_list[index].x;
}
void sonylens_privacy_x_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 index = config_params.advance.privacy_zone_no;
    
    PRIVACY_ZONE_t* zone = &config_params.privacy_zone_list[index];
    
    result = visca_set_privacy_non_interlock_mask(sonylens_camera_id, zone->no, param, zone->y, zone->w, zone->h);
    if(result != VISCA_result_ok) {
        return;
    }   

    config_params.privacy_zone_list[index].x = param;
}
char* sonylens_privacy_x_right(void)
{
    uint8 index = config_params.advance.privacy_zone_no;
    return Int2DecStr(config_params.privacy_zone_list[index].x);
}

//int current_privacy_y = 0;
int sonylens_privacy_y_get_count(void)
{
    return 256;
}
int sonylens_privacy_y_get(void)
{
    uint8 index = config_params.advance.privacy_zone_no;
    return config_params.privacy_zone_list[index].y;
}
void sonylens_privacy_y_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 index = config_params.advance.privacy_zone_no;
    
    PRIVACY_ZONE_t* zone = &config_params.privacy_zone_list[index];
    
    result = visca_set_privacy_non_interlock_mask(sonylens_camera_id, zone->no, zone->x, param, zone->w, zone->h);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.privacy_zone_list[index].y = param;
}
char* sonylens_privacy_y_right(void)
{
    uint8 index = config_params.advance.privacy_zone_no;
    return Int2DecStr(config_params.privacy_zone_list[index].y);
}


//int current_privacy_w = 0;
int sonylens_privacy_w_get_count(void)
{
    return 256;
}
int sonylens_privacy_w_get(void)
{
    uint8 index = config_params.advance.privacy_zone_no;
    return config_params.privacy_zone_list[index].w;
}
void sonylens_privacy_w_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 index = config_params.advance.privacy_zone_no;    
    PRIVACY_ZONE_t* zone = &config_params.privacy_zone_list[index];
    
    result = visca_set_privacy_non_interlock_mask(sonylens_camera_id, zone->no, zone->x, zone->y, param, zone->h);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.privacy_zone_list[index].w = param;
}
char* sonylens_privacy_w_right(void)
{
    uint8 index = config_params.advance.privacy_zone_no;
    return Int2DecStr(config_params.privacy_zone_list[index].w);
}

int sonylens_privacy_h_get_count(void)
{
    return 256;
}
int sonylens_privacy_h_get(void)
{
    uint8 index = config_params.advance.privacy_zone_no;
    return config_params.privacy_zone_list[index].h;
}
void sonylens_privacy_h_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 index = config_params.advance.privacy_zone_no;        
    PRIVACY_ZONE_t* zone = &config_params.privacy_zone_list[index];
    
    result = visca_set_privacy_non_interlock_mask(sonylens_camera_id, zone->no, zone->x, zone->y, zone->w, param);
    if(result != VISCA_result_ok) {
        return;
    }

    config_params.privacy_zone_list[index].h = param;
}
char* sonylens_privacy_h_right(void)
{
    uint8 index = config_params.advance.privacy_zone_no;
    return Int2DecStr(config_params.privacy_zone_list[index].h);
}

/* camera id */
//int current_camera_id = 0;
int sonylens_camera_id_get_count(void)
{
    return 999;
}
int sonylens_camera_id_get(void)
{
    return config_params.general.camera.id;
}
void sonylens_camera_id_set(int param)
{
    config_params.general.camera.id = param;
}
char* sonylens_camera_id_right(void)
{
    return Int2DecStr(config_params.general.camera.id + 1);
}

//int current_camera_id_display = 0;
char* camera_id_display[2] = {
    "OFF", "ON"
};

int sonylens_camera_id_display_get_count(void)
{
    return 2;
}
int sonylens_camera_id_display_get(void)
{
    return config_params.general.camera.display;
}
void sonylens_camera_id_display_set(int param)
{
    config_params.general.camera.display = param;
}
char* sonylens_camera_id_display_right(void)
{
    return camera_id_display[config_params.general.camera.display];
}

/* preset conf */
//int current_preset_no = 0;
int sonylens_preset_no_get_count(void)
{
    return 10;
}
int sonylens_preset_no_get(void)
{
    return config_params.general.preset_conf_no;
}
void sonylens_preset_no_set(int param)
{
    config_params.general.preset_conf_no = param;
}
char* sonylens_preset_no_right(void)
{
    return Int2DecStr(config_params.general.preset_conf_no + 1);
}

//int current_preset_time = 0;
int sonylens_preset_time_get_count(void)
{
    return 256;
}
int sonylens_preset_time_get(void)
{
    uint8 index = config_params.general.preset_conf_no;
    return config_params.preset_conf_list[index].time;
}
void sonylens_preset_time_set(int param)
{
    uint8 index = config_params.general.preset_conf_no;
    config_params.preset_conf_list[index].time = param;
}
char* sonylens_preset_time_right(void)
{
    uint8 index = config_params.general.preset_conf_no;
    return Int2DecStr(config_params.preset_conf_list[index].time);
}

//int current_preset_exist = 0;
char* preset_exist[2] = {
    "UNDEF", "DEF"
};
int sonylens_preset_exist_get_count(void)
{
    return 2;
}
int sonylens_preset_exist_get(void)
{
    uint8 index = config_params.general.preset_conf_no;
    return config_params.preset_conf_list[index].exist;
}
void sonylens_preset_exist_set(int param)
{
    uint8 index = config_params.general.preset_conf_no;
    config_params.preset_conf_list[index].exist = param;
}
char* sonylens_preset_exist_right(void)
{
    uint8 index = config_params.general.preset_conf_no;
    return preset_exist[config_params.preset_conf_list[index].exist];
}

void sonylens_preset_menu_set_action()
{
    uint8 index = config_params.general.preset_conf_no;
    uint32 zoom_value = 0;
    sonylens_get_zoom_value(&zoom_value);
    config_params.preset_conf_list[index].zoom_ratio = zoom_value;
    config_params.preset_conf_list[index].exist = 1;
}
void sonylens_preset_menu_run_action()
{
    uint8 index = config_params.general.preset_conf_no;
    printf("\r\n perset no:%d is selected", index);
    if(config_params.preset_conf_list[index].exist == 1)
    {
        sonylens_set_zoom_value(config_params.preset_conf_list[index].zoom_ratio);
    }
}
void sonylens_preset_menu_del_action()
{
    uint8 index = config_params.general.preset_conf_no;
    config_params.preset_conf_list[index].exist = 0;
}


/* exit */
void sonylens_menu_exit_yes() {
    sonylens_menu_off();
    sonylens_write_config_params(&config_params);
    if(monitor_mode_changed == true)
    {
        sonylens_restart();
    }
}
void sonylens_menu_exit_no() {
    sonylens_menu_off();
}
char* sonylens_splash_get_version(void)
{
    return current_version;
}
char* sonylens_splash_get_format(void)
{
    return sonylens_general_format_right();
}
char* sonylens_splash_get_protocol(void)
{
    return sonylens_general_protocol_right();
}
char* sonylens_splash_get_baudrate(void)
{
    return sonylens_general_baudrate_right();
}
char* sonylens_splash_get_address(void)
{
    return sonylens_general_address_right();
}


/*iris api*/
#if 0
int current_iris = 0;
void sonylens_set_iris_close(void)
{
    int iris;
    iris = sonylens_exposure_iris_get();
    if(0 != iris)
    {
        current_iris = iris;
        sonylens_exposure_iris_set(0);
    }
}
void sonylens_set_iris_open(void)
{
    int iris;
    iris = sonylens_exposure_iris_get();
    if(0 == iris)
    {
        sonylens_exposure_iris_set(current_iris);
    }
}
#endif

/*  zoom related api */

unsigned long zoom_ratio_read_time = 0;
static unsigned long zoom_ratio_display_time = 0;
int zoom_flag = 0;
uint32 current_zoom_ratio = 0;
int current_zoom_index = 0;


void sonylens_set_zoom_wide()
{
    VISCA_result_e result;
    
    result = visca_set_zoom_wide_speed(sonylens_camera_id, config_params.zoom.zoom_speed);
    if(VISCA_result_ok != result) {
        printf("\r\n %s failed.", __FUNCTION__);
        return;
    }
    zoom_flag = 1;
}
void sonylens_set_zoom_tele()
{
    VISCA_result_e result;
    
    result = visca_set_zoom_tele_speed(sonylens_camera_id, config_params.zoom.zoom_speed);
    if(VISCA_result_ok != result) {
        printf("\r\n %s failed.", __FUNCTION__);
        return;
    }
    zoom_flag = 2; 
}
void sonylens_set_zoom_stop()
{
    VISCA_result_e result;

    if(zoom_flag != 0)
    {
        result = visca_set_zoom_stop(sonylens_camera_id);
        if(VISCA_result_ok != result) {
            printf("\r\n %s failed.", __FUNCTION__);
            return;
        }
    }
    zoom_flag = 0;
}

void sonylens_zoom_ratio_display(char * title)
{
    sony_ui_set_title(0xA, NULL, title, NULL);
    sony_ui_display_line(0xA, true);
    zoom_ratio_display = true;
    zoom_ratio_display_time = GetSysTick_10Ms();
}

void sonylens_zoom_ratio_clear(void)
{
    if(zoom_ratio_display)
    {
        sony_ui_clear_line(0xA);
        sony_ui_display_line(0xA, false);
        zoom_ratio_display = false;
    }
}

bool sonylens_set_zoom_value(uint32 zoom_value)
{
    VISCA_result_e result;
    bool ret = false;

    int count = 0;

    while(true)
    {
        result = visca_set_zoom_value(sonylens_camera_id, (uint32)zoom_value);
        if(VISCA_result_command_not_executable == result)
        {
            count++;
            if(count > 5)
            {
                printf("\r\n set zoom value failed. result:%d", result);
                return ret;
            }
            Wait10Ms(100);
            continue;
        }
        else if(VISCA_result_ok == result)
        {
            break;
        }
        else
        {
            printf("\r\n set zoom value failed. result:%d", result);
            return ret;
        }
    }

    return true;
}

bool sonylens_get_zoom_value(uint32* zoom_value)
{
    VISCA_result_e result;
    bool ret = false;

    int count = 0;

    while(true)
    {
        result = visca_get_zoom_value(sonylens_camera_id, zoom_value);
        if(VISCA_result_command_not_executable == result)
        {
            count++;
            if(count > 5)
            {
                printf("\r\n get zoom value failed. result:%d", result);
                return ret;
            }
            Wait10Ms(100);
            continue;
        }
        else if(VISCA_result_ok == result)
        {
            break;
        }
        else
        {
            printf("\r\n get zoom value failed. result:%d", result);
            return ret;
        }
    }

    return true;
}

int sonylens_get_zoom_index(uint32 zoom)
{
    int i = 0;
    int index;
    int count = sizeof(zoom_ratio_list)/sizeof(ZOOM_RATIO_t);
    
    for(i = 0; i < count-1; i++)
    {
        if((zoom >= zoom_ratio_list[i].ratio) && (zoom < zoom_ratio_list[i+1].ratio))
        {
            break;
        }
    }

    return i;
}

char* sonylens_get_zoom_desc(int index)
{
    return zoom_ratio_list[index].desc;
}


void sonylens_zoom_process(void)
{
    int index;
    
    if(menu_display || splash_display)
    {
        return;
    }

    if((0 != zoom_flag) && ((GetSysTick_10Ms() - zoom_ratio_read_time) > 50))
    {
        sonylens_get_zoom_value(&current_zoom_ratio);
        index = sonylens_get_zoom_index(current_zoom_ratio);
        if(current_zoom_index != index)
        {
            current_zoom_index = index;
            if(config_params.zoom.zoom_display == 1)
            {
                sonylens_zoom_ratio_display(sonylens_get_zoom_desc(current_zoom_index));
            }
        }
        zoom_ratio_read_time = GetSysTick_10Ms();

        if((!zoom_ratio_display) && (config_params.zoom.zoom_display == 1))
        {
            sonylens_zoom_ratio_display(sonylens_get_zoom_desc(current_zoom_index));
        }
    }

    if((0 == zoom_flag) && zoom_ratio_display && ((GetSysTick_10Ms() - zoom_ratio_display_time) > 200))
    {
        sonylens_zoom_ratio_clear();
    }

    #if 0
    if((!zoom_ratio_display) && (config_params.zoom.zoom_display == 1))
    {
        sonylens_get_zoom_value(&current_zoom_ratio);
        current_zoom_index = sonylens_get_zoom_index(current_zoom_ratio);
        sonylens_zoom_ratio_display(sonylens_get_zoom_desc(current_zoom_index));
    } 
    #endif
}


/*manual mode and automatic mode  */
MODE_MANAGER_t mode_manager;

bool current_auto_mode = false;
char* mode_tips[] = {
    "A", "M"
};

#if 0
void sonylens_expcomp_enable(bool e)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 enable = 0;

    if(e)
    {
        enable = 0x02;
    }
    else
    {
        enable = 0x03;

    }
    result = visca_set_advance_expcomp(sonylens_camera_id, enable);
    if(result != VISCA_result_ok) {
        printf("\r\n L:%d %s() failed", __LINE__, __FUNCTION__);
        return;
    }
}

void sonylens_wb_auto_mode(bool e)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value;
    if(e)
    {
        value = 0x04;
    }
    else
    {
        value = 0x05;
    }
    
    
    result = visca_set_wb_mode(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
}

void sonylens_exposure_auto_mode(bool e)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value;
    if(e)
    {
        value = 0x00;
    }
    else
    {
        value = 0x03;
    }
    
    result = visca_set_exposure_ae_mode(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
}


#endif

void sonylens_expcomp_set(uint8 value)
{
    VISCA_result_e result = VISCA_result_ok;
    result = visca_set_advance_expcomp_value(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        printf("\r\n L:%d %s() failed", __LINE__, __FUNCTION__);
        return;
    }
}

void sonylens_iris_set(uint8 param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = 0;

    if(param == 0)
    {
        value = 0;
    }
    else
    {
        value = param + 4;
    }
    
    result = visca_set_exposure_iris(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        printf("\r\n L:%d %s() failed", __LINE__, __FUNCTION__);
        return;
    }
}


void sonylens_get_mode_info(void)
{
    uint8 ae_mode = mode_manager.exposure_mode;
    uint8 wb_mode = mode_manager.wb_mode;
    if((1 == ae_mode) && (1 == wb_mode))
    {
        current_auto_mode = false;
    }
    else
    {
        current_auto_mode = true;
    }
}

void sonylens_set_auto_mode(bool enable)
{
    if(enable)
    {
        sonylens_key_wb_mode_set(0);
        
        sonylens_key_exposure_ae_mode_set(0);

        sonylens_key_advance_expcomp_on(true);

        sonylens_key_advance_expcomp_set(mode_manager.expcomp);

        sonylens_tips_display(mode_tips[0]);
    }
    else
    {
        sonylens_key_wb_mode_set(1);
        
        sonylens_key_exposure_ae_mode_set(1);

        sonylens_key_exposure_iris_set(mode_manager.iris);

        sonylens_key_exposure_shutter_set(config_params.exposure.shutter);

        // iris
        //sonylens_exposure_iris_set(config_params.exposure.iris);

        // gain
        sonylens_key_exposure_gain_set(key_exp_gain);
        
        
        sonylens_key_advance_expcomp_on(false);
        
        sonylens_tips_display(mode_tips[1]);
    }

    mode_manager.current_expcomp = mode_manager.expcomp;
    mode_manager.current_iris = mode_manager.iris;

    current_auto_mode = enable;
}
bool sonylens_get_auto_mode(void)
{
    sonylens_get_mode_info();

    return current_auto_mode;
}

void sonylens_set_bright_plus(void)
{
    int count = 0;
    int level = 0;

    sonylens_get_auto_mode();

    if(current_auto_mode)
    {
        count = sonylens_advance_expcomp_get_count();
        printf("\r\n expcomp:%d-%d", count, mode_manager.current_expcomp);
        if(mode_manager.current_expcomp < (count - 1))
        {
            mode_manager.current_expcomp++;
            sonylens_expcomp_set(mode_manager.current_expcomp);
        }

        level = mode_manager.current_expcomp - count/2;
        sonylens_tips_display(Int2DecStr(level));
    }
    else
    {
        count = sonylens_exposure_iris_get_count();
        printf("\r\n iris:%d-%d", count, mode_manager.current_iris);
        if(mode_manager.current_iris < (count - 1))
        {
            mode_manager.current_iris++;
            sonylens_iris_set(mode_manager.current_iris);
        }

        level = mode_manager.current_iris - count/2;
        sonylens_tips_display(Int2DecStr(level));
    }
}
void sonylens_set_bright_minus(void)
{
    int count = 0;
    int level = 0;

    sonylens_get_auto_mode();
    
    if(current_auto_mode)
    {
        count = sonylens_advance_expcomp_get_count();
        printf("\r\n expcomp:%d-%d", count, mode_manager.current_expcomp);
        if(mode_manager.current_expcomp > 0)
        {
            mode_manager.current_expcomp--;
            sonylens_expcomp_set(mode_manager.current_expcomp);
        }
        level = mode_manager.current_expcomp - count/2;
        sonylens_tips_display(Int2DecStr(level));
    }
    else
    {
        count = sonylens_exposure_iris_get_count();
        printf("\r\n iris:%d-%d", count, mode_manager.current_iris);
        if(mode_manager.current_iris > 0)
        {
            mode_manager.current_iris--;
            sonylens_iris_set(mode_manager.current_iris);
        }
        level = mode_manager.current_iris - count/2;
        sonylens_tips_display(Int2DecStr(level));
    }
}

void sonylens_set_exposure_gain_plus(void)
{
    int count;
    count = sonylens_exposure_gain_get_count();
    if(count > 5) {
        count = 5;
    }

    if(key_exp_gain > 4) {
        key_exp_gain = 4;
    }

    key_exp_gain ++;
    key_exp_gain %= count;
    sonylens_key_exposure_gain_set(key_exp_gain);
}

void sonylens_key_wb_mode_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;

    if(0 == param)
    {
        value = 0x04;
    }
    else if(1 == param)
    {
        value = 0x05;
    }
    else if(2 == param)
    {
        value = 0x08;
    }
    else if(3 == param)
    {
        value = 0x06;
    }
    else
    {
        value = 0x00;
    }

    result = visca_set_wb_mode(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }

    mode_manager.wb_mode = param;
}
void sonylens_key_exposure_ae_mode_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 value = 0;

    if(param == 0)
    {
        value = 0x00;
    }
    else
    {
        value = 0x03;
    }
    
    result = visca_set_exposure_ae_mode(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }

    mode_manager.exposure_mode = param;
}
void sonylens_key_advance_expcomp_on(bool e)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 enable = 0;

    if(e)
    {
        enable = 0x02;
    }
    else
    {
        enable = 0x03;        
    }

    result = visca_set_advance_expcomp(sonylens_camera_id, enable);
    if(result != VISCA_result_ok) {
        return;
    }
}

void sonylens_key_advance_expcomp_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint8 enable = 0;
    uint32 value = 0;

    if(!advance_expcomp_on)
    {
        sonylens_advance_expcomp_on(true);
    }

    value = param;
    result = visca_set_advance_expcomp_value(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }

    mode_manager.current_expcomp = param;
}
void sonylens_key_exposure_iris_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = 0;

    if(param == 0)
    {
        value = 0;
    }
    else
    {
        value = param + 4;
    }
    
    result = visca_set_exposure_iris(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
    mode_manager.current_iris = param;
}
void sonylens_key_exposure_shutter_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = 0;

    value = param;
    
    result = visca_set_exposure_shutter(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
}

void sonylens_key_image_freeze_set(int param) {
    VISCA_result_e result;
    uint8 mirror;
    
    if(param == 0) {
        mirror = 0x03;
    } else {
        mirror = 0x02;
    }

    result = visca_set_image_freeze(sonylens_camera_id, mirror);
    if(result != VISCA_result_ok) {
        return;
    }
    key_freeze = param;
}

void sonylens_key_image_flip_set(int param)
{
    VISCA_result_e result;
    uint8 mirror;
    
    // set the mirror
    if(param == 0) {
        mirror = 0x03;
    } else {
        mirror = 0x02;
    }

    result = visca_set_image_flip(sonylens_camera_id, mirror);
    if(result != VISCA_result_ok) {
        return;
    }

    key_flip = param;
}

void sonylens_key_exposure_gain_set(int param)
{
    VISCA_result_e result = VISCA_result_ok;
    uint32 value = 0;

    value = param + 1;
    
    result = visca_set_exposure_gain(sonylens_camera_id, value);
    if(result != VISCA_result_ok) {
        return;
    }
}

static unsigned long tips_display_time = 0;
static bool tips_display = false;

void sonylens_tips_display(char * title)
{
    if(menu_display || splash_display)
    {
        return;
    }
    sony_ui_set_title(0xA, NULL, title, NULL);
    sony_ui_display_line(0xA, true);
    tips_display = true;
    tips_display_time = GetSysTick_10Ms();
}

void sonylens_tips_clear(void)
{
    if(tips_display)
    {
        sony_ui_clear_line(0xA);
        sony_ui_display_line(0xA, false);
        tips_display = false;
    }
}

void sonylens_tips_display_process(void)
{
    int index;
    
    if(menu_display || splash_display)
    {
        return;
    }
    
    if(tips_display && ((GetSysTick_10Ms() - tips_display_time) > 250))
    {
        sonylens_tips_clear();
    }
}


void sonylens_menu_display_process(void)
{    
    if(menu_display && ((GetSysTick_10Ms() - menu_display_time) > 3000))
    {
        sonylens_menu_off();
    }
}


BOOL sonylens_remote_is_menu_on(void)
{
    return menu_display;
}
void sonylens_remote_navi_ok(void)
{
    if(!menu_display)
    {
        current_menu = main_menu;
        sonylens_menu_on1(); 
    }
    else
    {
        sonylens_menu_action();
    }
}
void sonylens_remote_navi_up(void)
{
    sonylens_menu_move_updown1(-1);
}
void sonylens_remote_navi_down(void)
{
    sonylens_menu_move_updown1(1);
}
void sonylens_remote_navi_left(void)
{
    sonylens_menu_move_leftright(-1);
}
void sonylens_remote_navi_right(void)
{
    sonylens_menu_move_leftright(1);
}

void sonylens_control_f1(uint16 data) {
    int count = sonylens_general_format_get_count();
    int index = sonylens_general_format_get();
    
    if((2 == data) || (3 == data) || (6 == data) || (7 == data)) {
        index = data;
    } else {
        return;
    }

    /*
    if(2 == index) {
        index = 3;
    } else if(3 == index) {
        index = 6;
    } else if(6 == index) {
        index = 7;
    } else if(7 == index) {
        index = 2;
    } else {
        index = 2;
    }
    */
    sonylens_general_format_set(index);
    sonylens_taskstate = SONY_GET_MONITOR_MODE;
    return;

}
void sonylens_control_f2(void) {

}
void sonylens_control_f3(void) {

}
void sonylens_control_f4(void) {
    
}
void sonylens_control_f5(void) {

}
void sonylens_control_f6(void) {

}
void sonylens_control_f7(void) {

}
void sonylens_control_f8(void) {

}
void sonylens_control_f9(void) {
    
}
void sonylens_control_f10(void) {
    
}
void sonylens_control_f11(void) {
}
void sonylens_control_f12(void) {
}

bool sonylens_is_state_ok(void) {
    if(SONY_IDLE_2 == sonylens_taskstate) {
        return true;
    } else {
        return false;
    }
}
void sonylens_zoom_set_wide_limit(unsigned char value) {
    VISCA_result_e visca_result;
    visca_result = visca_set_register(sonylens_camera_id, SONYLENS_WIDE_LIMIT_ADDR, value);
    if(VISCA_result_ok != visca_result) {
        printf("\r\n set wide limit failed");
    }

    value = 0;
    
    visca_result = visca_get_register(sonylens_camera_id, SONYLENS_WIDE_LIMIT_ADDR, &value);
    if(VISCA_result_ok != visca_result) {
        printf("\r\n get wide limit failed");
    } else {
        printf("\r\n value:%d", value);
    }
}
void sonylens_zoom_set_tele_limit(unsigned char value) {
    VISCA_result_e visca_result;
    visca_result = visca_set_register(sonylens_camera_id, SONYLENS_TELE_LIMIT_ADDR, value);
    if(VISCA_result_ok != visca_result) {
        printf("\r\n set tele limit failed");
    }

    value = 0;
    
    visca_result = visca_get_register(sonylens_camera_id, SONYLENS_TELE_LIMIT_ADDR, &value);
    if(VISCA_result_ok != visca_result) {
        printf("\r\n get tele limit failed");
    } else {
        printf("\r\n value:%d", value);
    }
}


void sonylens_task(void)
{
    int i;
    int get_len;
    char *in=sonylens_rx_buffer;
    unsigned char regValue;
    uint32 u32Value;
    SONY_RESULT_E result;
    uint8 power;
    uint8 value;
    VISCA_result_e visca_result;
    unsigned long keying_time = 0;

    if( GetSysTick_Sec() > (sonylens_task_t_sec + 1) )
    {
        sonylens_task_t_sec = GetSysTick_Sec();
        printf("\r\n\r\nTask_t_sec:%ld state:%d cmd state:%d\r\n", sonylens_task_t_sec, sonylens_taskstate, sonylens_cmd_state);
        key_print();
    }

    // key scanning
    key_scan_process();

    //sonylens task main loop
    switch(sonylens_taskstate)
    {
    case SONY_12V_ON:
		rec_systick_mark_sonylens = GetSysTick_10Ms();
        if(key_is_long_pressed(KEY_ID_L, &keying_time))
        {
            if(keying_time > 500)
            {
                printf("\r\n set format to 720P25");
                config_params.general.format = 9; // 720P25
                sonylens_write_config_params(&config_params);
                sonylens_taskstate = SONY_VISCA_INIT;
            }
        }
        else if(key_is_long_pressed(KEY_ID_R, &keying_time))
        {
            if(keying_time > 500)
            {
                printf("\r\n set format to 1080P25");
                config_params.general.format = 3; // 1080P25
                sonylens_write_config_params(&config_params);
                sonylens_taskstate = SONY_VISCA_INIT;
            }
        }
        else
        {
            sonylens_taskstate = SONY_VISCA_INIT;
        }
		break;
    case SONY_INIT_CONFIG_PARAMS:
        if(sonylens_read_config_params(&config_params))
        {
            sonylens_taskstate = SONY_POWER_OFF;
        }
        else
        {
            Wait10Ms(100);
        }
        break;
    case SONY_VISCA_INIT:
        visca_init();
        if(0 == config_params.general.protocol)
        {
            pelco_d_init();
        }
        else if(1 == config_params.general.protocol)
        {
            sony_visca_init();
        }
        else
        {
            pelco_d_init();
        }
        
        sonylens_taskstate = SONY_SELF_CHECK_DONE;
        break;
    case SONY_SELF_CHECK_DONE:
        visca_result = visca_get_self_check_state(sonylens_camera_id, &value);
        if(VISCA_result_ok == visca_result) {
            if(value == 0x02) {
                printf("\r\n self check done.");
                sonylens_taskstate = SONY_SET_ADDRESS;
            } else if(value == 0x03) {
                printf("\r\n self check not done");
                Wait10Ms(50);
                sonylens_taskstate = SONY_SELF_CHECK_DONE;
            }
        } else {
            printf("\r\n visca_result:%d", visca_result);
            Wait10Ms(100);
        }
        break;
    case SONY_SET_ADDRESS:
        visca_result = visca_set_address(sonylens_camera_id);
        if(VISCA_result_ok == visca_result) {
            sonylens_taskstate = SONY_IF_CLEAR;
        } else if(VISCA_result_network_change == visca_result) {
            sonylens_taskstate = SONY_SET_ADDRESS;
        } else {
            printf("\r\n visca_result:%d", visca_result);
            Wait10Ms(100);
        }
        break;
    case SONY_IF_CLEAR:
        visca_result = visca_if_clear(sonylens_camera_id);
        if(VISCA_result_ok == visca_result) {
            sonylens_taskstate = SONY_GET_VERSION;
        } else {
            printf("\r\n visca_result:%d", visca_result);
            Wait10Ms(100);
        }
        break;
    case SONY_GET_VERSION:
        if(VISCA_result_ok == visca_get_version(sonylens_camera_id, &model_code, &rom_version, &socket_num))
        {
            printf("\r\n model:%x, version:%x, socket:%d", model_code, rom_version, socket_num);
            sonylens_taskstate = SONY_GET_POWER_STATE;
        }
        break;
    case SONY_GET_POWER_STATE:
        visca_result = visca_get_power(sonylens_camera_id, &power);
        if(VISCA_result_ok == visca_result) {
            if(power == 0x02) {
                printf("\r\n power is ON");
                power_is_on = true;
                sonylens_taskstate = SONY_POWER_ON;
            } else if(power == 0x03) {
                printf("\r\n power is OFF");
                sonylens_taskstate = SONY_POWER_ON;
            }
        } else {
            printf("\r\n visca_result:%d", visca_result);
            Wait10Ms(100);
        }
        break;
    case SONY_POWER_ON:
        if((0 == config_params.general.format) || (1 == config_params.general.format))
        {
            gennum_3g_enable(true);
        }
        else
        {
            gennum_3g_enable(false);
        }
        power = 0x02;
        if(VISCA_result_ok == visca_set_power(sonylens_camera_id, power)) {
            sonylens_power_on_time = GetSysTick_10Ms();
            sonylens_state_ready = false;
            power_is_on = true;
            sonylens_taskstate = SONY_GET_MONITOR_MODE;
        } else {
            Wait10Ms(50);
        }
        
        break;
    case SONY_POWER_OFF:
        power = 0x03;
        if(VISCA_result_ok == visca_set_power(sonylens_camera_id, power)) {
            sonylens_taskstate = SONY_POWER_ON;
        }
        break;
    case SONY_GET_MONITOR_MODE:
        visca_result = visca_get_register(sonylens_camera_id, SONYLENS_MONITOR_MODE_ADDR, &value);
        if(VISCA_result_ok == visca_result) {
            printf("\r\n current monitor register value is:0x%x", value);
            if(value == sonylens_get_monitor_mode(config_params.general.format))
            {
                Lt8918_setVideoResolution(config_params.general.format);
                printf("\r\n it is expected:%d(0x%x)", config_params.general.format, value);
                sonylens_taskstate = SONY_IDLE_2;
            }
            else
            {
                printf("\r\n it is not expected. request:%s(%d-%d)", 
                    general_format[config_params.general.format], 
                    config_params.general.format,
                    sonylens_get_monitor_mode(config_params.general.format));
                sonylens_taskstate = SONY_SET_MONITOR_MODE;
            }
        }
        else
        {
            printf("\r\n L:%d result:%d", __LINE__, visca_result);
            Wait10Ms(100);
        }
        break;
    case SONY_SET_MONITOR_MODE:
        printf("\r\n L:%d switch to :%s. value:%d", __LINE__, 
            general_format[config_params.general.format], 
            sonylens_get_monitor_mode(config_params.general.format));
        visca_result = visca_set_register(sonylens_camera_id, SONYLENS_MONITOR_MODE_ADDR, sonylens_get_monitor_mode(config_params.general.format));
        if(VISCA_result_ok == visca_result) {
            sonylens_write_config_params(&config_params);
            Wait10Ms(100);
            video_enable(false);
            Wait10Ms(50);
            video_enable(true);
            system_reset();
            sonylens_taskstate = SONY_SELF_CHECK_DONE;
        }
        else
        {
            printf("\r\n L:%d result:%d", __LINE__, visca_result);
            Wait10Ms(100);
        }
        break;
    case SONY_WAIT_READY:
        if(sonylens_wait_ready())
        {
            sonylens_taskstate = SONY_IDLE;
        }
        break;
    case SONY_CLEAR_TITLES:
        visca_result = visca_set_title_clear(sonylens_camera_id, 0xF);
        if(VISCA_result_ok == visca_result) {
            titles_clear_done = true;
            sonylens_taskstate = SONY_INIT_CONFIG;
        }
        else
        {
            printf("\r\n L:%d result:%d", __LINE__, visca_result);
            Wait10Ms(100);
        }
        break;
    case SONY_INIT_CONFIG:
        sonylens_zoom_init();
        sonylens_focus_init();
        sonylens_general_init();
        sonylens_exposure_init();
        sonylens_wb_init();
        sonylens_advance_init();
        sonylens_image_init();
        sonylens_taskstate = SONY_IDLE;
        break;
    case SONY_SET_ZOOM_RATIO:
        #ifdef SONYLENS_10X_SUPPORT
        visca_result = visca_set_zoom_value(sonylens_camera_id, SONY_ZOOM_RATIO_5X);
        if(VISCA_result_ok == visca_result) {
            zoom_ratio_set_done = true;
            sonylens_taskstate = SONY_IDLE;
        }
        else
        {
            printf("\r\n L:%d result:%d", __LINE__, visca_result);
            Wait10Ms(100);
        }

        sonylens_zoom_set_wide_limit(0x40);
        sonylens_zoom_set_tele_limit(0x30);
        #else
        visca_result = visca_set_zoom_value(sonylens_camera_id, SONY_ZOOM_RATIO_10X);
        if(VISCA_result_ok == visca_result) {
            zoom_ratio_set_done = true;
            sonylens_taskstate = SONY_IDLE;
        }
        else
        {
            printf("\r\n L:%d result:%d", __LINE__, visca_result);
            Wait10Ms(100);
        }
        #endif
        break;
    case SONY_SET_SPLASH:
        sonylens_splash_display();
        splash_set_done = true;
        sonylens_taskstate = SONY_IDLE;
        break;
    
    case SONY_IDLE:
        // clear titles
        if(!titles_clear_done)
        {
            sonylens_taskstate = SONY_CLEAR_TITLES;
            break;
        }
        
        // check zoom ratio
        if(!zoom_ratio_set_done)
        {
            sonylens_taskstate = SONY_SET_ZOOM_RATIO;
            break;
        }

        // display splash titles
        if(!splash_set_done)
        {
            sonylens_taskstate = SONY_SET_SPLASH;
            break;
        }
        else
        {
            sonylens_splash_clear();
        }
            
        sonylens_zoom_process();
        
        sonylens_tips_display_process();

        sonylens_menu_display_process();
        
        // key process
        sonylens_key_handle_process();

        pelco_d_process();

        visca_process();
        
		break;
    case SONY_IDLE_2:
        // key process
        sonylens_key_handle_process();
        {
            lt8918l_Process();
        }

        pelco_d_process();
        break;
	default:
		break;
   }
  

    rec_systick_mark_sonylens = GetSysTick_10Ms();
}


