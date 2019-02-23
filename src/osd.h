#ifndef OSD_H_
#define OSD_H_

#include <stdarg.h>

struct dtx_font;

struct dtx_font *ui_font;
int ui_font_size;

void set_message_timeout(long timeout);
void osd_printf(const char *fmt, ...);
void show_message(long timeout, float r, float g, float b, const char *fmt, ...);
void show_messagev(long timeout, float r, float g, float b, const char *fmt, va_list ap);

void print_text(float x, float y, float r, float g, float b, const char *fmt, ...);
void print_textv(float x, float y, float r, float g, float b, const char *fmt, va_list ap);

void draw_osd(void);

#endif	/* OSD_H_ */
