#ifndef __VISCA_H
#define __VISCA_H

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


typedef enum _VISCA_state {
    VISCA_state_idle = 0,
    VISCA_state_send,
    VISCA_state_wait_ack,
    VISCA_state_wait_completion,
    VISCA_state_wait_set_address_done,
    VISCA_state_wait_if_clear_done,
    // inquire
    VISCA_state_wait_result,
    VISCA_state_completion,
} VISCA_state_e;

typedef enum _VISCA_result {
    VISCA_result_ok = 0,
    VISCA_result_network_change,
    VISCA_result_command_not_executable,
    VISCA_result_fail,
    VISCA_result_no_response,
    VISCA_result_unknown,
} VISCA_result_e;

typedef enum _VISCA_type {
    VISCA_type_command = 0,
    VISCA_type_inquiry = 1,
} VISCA_type_e;

typedef struct _VISCA_packet
{
    unsigned char bytes[32];
    uint32 length;
    VISCA_type_e type;
} VISCA_packet_t;

void visca_init(void);
void visca_append_byte(VISCA_packet_t *packet, unsigned char byte);
void visca_init_packet(VISCA_packet_t *packet, int address);
void visca_init_broadcast_packet(VISCA_packet_t *packet);
void visca_set_state(VISCA_state_e s);
VISCA_state_e visca_get_state(void);
VISCA_result_e visca_get_result(void);
VISCA_result_e visca_set_address(int address);
VISCA_result_e visca_if_clear(int address);

VISCA_result_e visca_get_version(int address, uint32* model, uint32* rom, uint32* socket);
VISCA_result_e visca_get_self_check_state(int address, uint8 *state);
VISCA_result_e visca_get_power(int address, uint8 *power);
VISCA_result_e visca_set_power(int address, uint8 power);

VISCA_result_e visca_get_register(int address, uint8 register, uint8 *value);
VISCA_result_e visca_set_register(int address, uint8 register, uint8 value);

VISCA_result_e visca_get_zoom_value(int address, uint32 *value);
VISCA_result_e visca_set_zoom_value(int address, uint32 value);

VISCA_result_e visca_get_title_display(int address, uint8 *enable);
VISCA_result_e visca_set_title_display(int address, uint8 lines, uint8 enable);

VISCA_result_e visca_get_title_clear(int address);
VISCA_result_e visca_set_title_clear(int address, uint8 lines);

VISCA_result_e visca_get_title_param(int address);
VISCA_result_e visca_set_title_param(int address, uint8 line, uint8 pos, uint8 color, uint8 blink);

VISCA_result_e visca_get_title(int address);
VISCA_result_e visca_set_title(int address, uint8 line, unsigned char* title);

VISCA_result_e visca_set_zoom_stop(int address);
VISCA_result_e visca_set_zoom_wide(int address);
VISCA_result_e visca_set_zoom_tele(int address);



VISCA_result_e visca_get_zoom_tele_speed(int address, uint8 *speed);
VISCA_result_e visca_set_zoom_tele_speed(int address, uint8 speed);

VISCA_result_e visca_get_zoom_wide_speed(int address, uint8 *speed);
VISCA_result_e visca_set_zoom_wide_speed(int address, uint8 speed);

VISCA_result_e visca_get_digital_zoom(int address, uint8 *enable);
VISCA_result_e visca_set_digital_zoom(int address, uint8 enable);

VISCA_result_e visca_get_zoom_osd(int address, uint8 *enable);
VISCA_result_e visca_set_zoom_osd(int address, uint8 enable);

VISCA_result_e visca_get_zoom_display(int address, uint8 *enable);
VISCA_result_e visca_set_zoom_display(int address, uint8 enable);

VISCA_result_e visca_set_focus_far(int address);
VISCA_result_e visca_set_focus_near(int address);
VISCA_result_e visca_set_focus_far_speed(int address, uint8 speed);
VISCA_result_e visca_set_focus_near_speed(int address, uint8 speed);
VISCA_result_e visca_set_focus_stop(int address);

VISCA_result_e visca_get_focus_mode(int address, uint8* mode);
VISCA_result_e visca_set_focus_mode(int address, uint8 mode);

VISCA_result_e visca_get_focus_af_sens(int address, uint8* mode);
VISCA_result_e visca_set_focus_af_sens(int address, uint8 mode);

VISCA_result_e visca_get_focus_light(int address, uint8* mode);
VISCA_result_e visca_set_focus_light(int address, uint8 mode);


VISCA_result_e visca_get_focus_near_limit(int address, uint32 *value);
VISCA_result_e visca_set_focus_near_limit(int address, uint32 value);

VISCA_result_e visca_get_focus_position(int address, uint32 *value);
VISCA_result_e visca_set_focus_position(int address, uint32 value);

VISCA_result_e visca_get_exposure_ae_mode(int address, uint8 *mode);
VISCA_result_e visca_set_exposure_ae_mode(int address, uint8 mode);

VISCA_result_e visca_get_exposure_shutter(int address, uint32 *value);
VISCA_result_e visca_set_exposure_shutter(int address, uint32 value);

VISCA_result_e visca_get_exposure_iris(int address, uint32 *value);
VISCA_result_e visca_set_exposure_iris(int address, uint32 value);

VISCA_result_e visca_get_exposure_gain(int address, uint32 *gain);
VISCA_result_e visca_set_exposure_gain(int address, uint32 gain);

VISCA_result_e visca_get_exposure_slow_ae(int address, uint8 *value);
VISCA_result_e visca_set_exposure_slow_ae(int address, uint8 value);

VISCA_result_e visca_get_exposure_slow_shutter(int address, uint8 *value);
VISCA_result_e visca_set_exposure_slow_shutter(int address, uint8 value);

VISCA_result_e visca_get_wb_mode(int address, uint8 *mode);
VISCA_result_e visca_set_wb_mode(int address, uint8 mode);

VISCA_result_e visca_get_wb_red_gain(int address, uint32 *value);
VISCA_result_e visca_set_wb_red_gain(int address, uint32 value);

VISCA_result_e visca_get_wb_blue_gain(int address, uint32 *value);
VISCA_result_e visca_set_wb_blue_gain(int address, uint32 value);

VISCA_result_e visca_set_wb_one_push(int address);

/* advance */
VISCA_result_e visca_get_advance_blc(int address, uint8 *value);
VISCA_result_e visca_set_advance_blc(int address, uint8 value);

VISCA_result_e visca_get_advance_expcomp(int address, uint8 *value);
VISCA_result_e visca_set_advance_expcomp(int address, uint8 value);

VISCA_result_e visca_get_advance_expcomp_value(int address, uint32 *value);
VISCA_result_e visca_set_advance_expcomp_value(int address, uint32 value);

VISCA_result_e visca_get_advance_wdr(int address, uint8 *value);
VISCA_result_e visca_set_advance_wdr(int address, uint8 value);

VISCA_result_e visca_get_advance_ircut(int address, uint8 *value);
VISCA_result_e visca_set_advance_ircut(int address, uint8 value);

VISCA_result_e visca_get_advance_nr_level(int address, uint8 *value);
VISCA_result_e visca_set_advance_nr_level(int address, uint8 value);

VISCA_result_e visca_get_advance_stabilizer(int address, uint8 *value);
VISCA_result_e visca_set_advance_stabilizer(int address, uint8 value);

VISCA_result_e visca_get_advance_defog_mode(int address, uint8 *value);
VISCA_result_e visca_set_advance_defog_mode(int address, uint8 value, uint8 level);

VISCA_result_e visca_get_advance_alarm_out(int address, uint8 *value);
VISCA_result_e visca_set_advance_alarm_out(int address, uint8 value);


/* image */
VISCA_result_e visca_get_image_aperture(int address, uint8 *value);
VISCA_result_e visca_set_image_aperture(int address, uint8 value);

VISCA_result_e visca_get_image_color_gain(int address, uint8 *value);
VISCA_result_e visca_set_image_color_gain(int address, uint8 value);

VISCA_result_e visca_get_image_color_hue(int address, uint8 *value);
VISCA_result_e visca_set_image_color_hue(int address, uint8 value);

VISCA_result_e visca_get_image_chroma_suppress(int address, uint8 *value);
VISCA_result_e visca_set_image_chroma_suppress(int address, uint8 value);

VISCA_result_e visca_get_image_gamma(int address, int *value);
VISCA_result_e visca_set_image_gamma(int address, int value);

VISCA_result_e visca_get_image_mirror(int address, uint8* mirror);
VISCA_result_e visca_set_image_mirror(int address, uint8 mirror);

VISCA_result_e visca_get_image_flip(int address, uint8* flip);
VISCA_result_e visca_set_image_flip(int address, uint8 flip);

VISCA_result_e visca_get_image_freeze(int address, uint8* mirror);
VISCA_result_e visca_set_image_freeze(int address, uint8 mirror);

VISCA_result_e visca_get_image_pic_effect(int address, uint8* mirror);
VISCA_result_e visca_set_image_pic_effect(int address, uint8 mirror);

/* privacy zone set */
VISCA_result_e visca_get_privacy_mask(int address);
VISCA_result_e visca_set_privacy_mask(int address, uint8 mask, uint8 w, uint8 h);

VISCA_result_e visca_get_privacy_non_interlock_mask(int address);
VISCA_result_e visca_set_privacy_non_interlock_mask(int address, uint8 mask, uint8 x, uint8 y, uint8 w, uint8 h);

VISCA_result_e visca_get_privacy_display(int address, uint32* display);
VISCA_result_e visca_set_privacy_display(int address, uint32 display);

VISCA_result_e visca_get_center_line_display(int address);
VISCA_result_e visca_set_center_line_display(int address, uint8 enable);


#endif
