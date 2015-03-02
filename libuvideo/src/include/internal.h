#pragma once

/**
 * \typedef pf_picture_is
 * \brief Function type for check picture format function.
 */
typedef bool (* pf_picture_is)(unsigned char * data, unsigned int size);

/**
 * \typedef pf_picture_display
 * \brief Function type for display picture function.
 * \return video error code
 */
typedef uvideo_err_t (* pf_picture_display)(video_console * cons, unsigned char * data, unsigned int size);


typedef unsigned char (* TAB_FONT_8x16)[256][16];

TAB_FONT_8x16  uvideo_get_fonts_8x16(void);
video_conf *   uvideo_get_mainconf(void);
uvideo_err_t   uvideo_internal_display(video_console * cons, unsigned char * data, unsigned int size);

