#ifndef _SD_SPASE_H_
#define _SD_SPACE_H_

long long get_diskfree_space(const char *p);
int get_sd_space(char *mount_path);
int get_meminfo(void);
int path_check(const char *path);
int check_local_path(char *rec_local_path, FileWritingDest	pDest);
int stream_check_cfg(int ch);
int check_disk(char *diskname);
void subscribe_stream(int ch);
void subscribe_path(void);
int file_check(char *file);
int check_stream(int ch);
void check_stream_name(int ch, char *stream_name);
void subscribe_removeiffileold(void);
void subscribe_archiveduration(void);
void subscribe_removeifnospace(void);
void subscribe_reservedspace(void);
void subscribe_path(void);
int dir_check(const char *path);
void on_cfg_changed(int pd);
void init_record_t_params(void);	

#endif



