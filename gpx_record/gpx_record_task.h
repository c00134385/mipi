#ifndef __GPX_RECORD_H__
#define __GPX_RECORD_H__

//
enum{
    GPX_REC_IDEL,
    GPX_REC_START,
    GPX_RECORDING,
	GPX_REC_END=10,
	GPX_REC_FULL,
	//
	GPX_REC_ERROR
};
//
int gpx_record_taskinit(void);
void gpx_record_taskctrl(int cmd);
int gpx_record_task(void *p);
void gpx_file_save(void);
void gpx_file_saveexit(void);

#endif
