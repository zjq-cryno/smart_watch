SET(SOURCES screens/ui_MainScreen.c
    screens/ui_heartScreen.c
    screens/ui_LightScreen.c
    screens/ui_StepScreen.c
    screens/ui_Screen1.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_step_png.c
    images/ui_img_temperature_png.c
    images/ui_img_drop_png.c
    images/ui_img_tired_png.c
    images/ui_img_bluetooth_png.c
    images/ui_img_heart_png.c
    images/ui_img_oxysatura_png.c
    images/ui_img_shoudian_png.c
    images/ui_img_timer_png.c
    images/ui_img_calorie_png.c
    images/ui_img_1576808831.c)

add_library(ui ${SOURCES})
