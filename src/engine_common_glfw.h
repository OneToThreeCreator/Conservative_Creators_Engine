#include "engine_common.h"

void cce_setWindowParameters__glfw (cce_enum parameter, uint32_t a, uint32_t b);
void showWindow__glfw (void);
void toWindow__glfw (void);
void toFullscreen__glfw (void);
void swapBuffers__glfw (void);
struct cce_uvec2 getCurrentAspectRatio__glfw (void);

int initEngine__glfw (const char *label, uint16_t globalBoolsQuantity);
void engineUpdate__glfw (void);
void terminateEngine__glfw (void);