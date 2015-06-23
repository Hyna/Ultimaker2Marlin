#include "Configuration.h"
#ifdef ENABLE_ULTILCD2
#include "UltiLCD2.h"
#include "UltiLCD2_hi_lib.h"
#include "UltiLCD2_gfx.h"
#include "UltiLCD2_menu_maintenance.h"
#include "UltiLCD2_menu_first_run.h"
#include "UltiLCD2_menu_material.h"
#include "cardreader.h"
#include "lifetime_stats.h"
#include "ConfigurationStore.h"
#include "temperature.h"
#include "pins.h"


static void lcd_menu_maintenance_advanced();
static void lcd_menu_maintenance_advanced_heatup();
static void lcd_menu_maintenance_led();
static void lcd_menu_maintenance_extrude();
static void lcd_menu_maintenance_retraction();
static void lcd_menu_advanced_version();
static void lcd_menu_advanced_stats();
static void lcd_menu_maintenance_motion();
static void lcd_menu_advanced_factory_reset();

void lcd_menu_maintenance()
{
    lcd_tripple_menu(PSTR("TISKOVA|PODLOZKA"), PSTR("POKROCILE|NASTAVENI"), PSTR("ZPET"));

    if (lcd_lib_button_pressed)
    {
        if (IS_SELECTED_MAIN(0))
            lcd_change_to_menu(lcd_menu_first_run_start_bed_leveling);
        else if (IS_SELECTED_MAIN(1))
            lcd_change_to_menu(lcd_menu_maintenance_advanced);
        else if (IS_SELECTED_MAIN(2))
            lcd_change_to_menu(lcd_menu_main);
    }

    lcd_lib_update_screen();
}

static char* lcd_advanced_item(uint8_t nr)
{
    if (nr == 0)
        strcpy_P(card.longFilename, PSTR("< ZPET"));
    else if (nr == 1)
        strcpy_P(card.longFilename, PSTR("Nastaveni LED"));
    else if (nr == 2)
#if EXTRUDERS < 2
        strcpy_P(card.longFilename, PSTR("Zahrat trysku"));
#else
        strcpy_P(card.longFilename, PSTR("Zahrat prvni trysku"));
    else if (nr == 3)
        strcpy_P(card.longFilename, PSTR("Zahrat druhou trysku"));
#endif
#if TEMP_SENSOR_BED != 0
    else if (nr == 2 + EXTRUDERS)
        strcpy_P(card.longFilename, PSTR("Heatup buildplate"));
#endif
    else if (nr == 2 + BED_MENU_OFFSET + EXTRUDERS)
        strcpy_P(card.longFilename, PSTR("Zaparkovat hlavu"));
    else if (nr == 3 + BED_MENU_OFFSET + EXTRUDERS)
        strcpy_P(card.longFilename, PSTR("Snizit tisk. podlozku"));
    else if (nr == 4 + BED_MENU_OFFSET + EXTRUDERS)
        strcpy_P(card.longFilename, PSTR("Zvysit tisk. podlozku"));
    else if (nr == 5 + BED_MENU_OFFSET + EXTRUDERS)
        strcpy_P(card.longFilename, PSTR("Zavedeni materialu"));
    else if (nr == 6 + BED_MENU_OFFSET + EXTRUDERS)
#if EXTRUDERS < 2
        strcpy_P(card.longFilename, PSTR("Pohyb s materialem"));
#else
        strcpy_P(card.longFilename, PSTR("Move material (1)"));
    else if (nr == 7 + BED_MENU_OFFSET + EXTRUDERS)
        strcpy_P(card.longFilename, PSTR("Move material (2)"));
#endif
    else if (nr == 6 + BED_MENU_OFFSET + EXTRUDERS * 2)
        strcpy_P(card.longFilename, PSTR("Rychlost vetraku"));
    else if (nr == 7 + BED_MENU_OFFSET + EXTRUDERS * 2)
        strcpy_P(card.longFilename, PSTR("Nastaveni retrakce"));
    else if (nr == 8 + BED_MENU_OFFSET + EXTRUDERS * 2)
        strcpy_P(card.longFilename, PSTR("Nastaveni motoru"));
    else if (nr == 9 + BED_MENU_OFFSET + EXTRUDERS * 2)
        strcpy_P(card.longFilename, PSTR("Verze firmware"));
    else if (nr == 10 + BED_MENU_OFFSET + EXTRUDERS * 2)
        strcpy_P(card.longFilename, PSTR("Statistiky tiskarny"));
    else if (nr == 11 + BED_MENU_OFFSET + EXTRUDERS * 2)
        strcpy_P(card.longFilename, PSTR("Tovarni nastaveni"));
    else
        strcpy_P(card.longFilename, PSTR("???"));
    return card.longFilename;
}

static void lcd_advanced_details(uint8_t nr)
{
    char buffer[16];
    buffer[0] = '\0';
    if (nr == 1)
    {
        int_to_string(led_brightness_level, buffer, PSTR("%"));
    }else if (nr == 2)
    {
        int_to_string(int(dsp_temperature[0]), buffer, PSTR("C/"));
        int_to_string(int(target_temperature[0]), buffer+strlen(buffer), PSTR("C"));
#if EXTRUDERS > 1
    }else if (nr == 3)
    {
        int_to_string(int(dsp_temperature[1]), buffer, PSTR("C/"));
        int_to_string(int(target_temperature[1]), buffer+strlen(buffer), PSTR("C"));
#endif
#if TEMP_SENSOR_BED != 0
    }else if (nr == 2 + EXTRUDERS)
    {
        int_to_string(int(dsp_temperature_bed), buffer, PSTR("C/"));
        int_to_string(int(target_temperature_bed), buffer+strlen(buffer), PSTR("C"));
#endif
    }else if (nr == 6 + BED_MENU_OFFSET + EXTRUDERS * 2)
    {
        int_to_string(int(fanSpeed) * 100 / 255, buffer, PSTR("%"));
    }else if (nr == 9 + BED_MENU_OFFSET + EXTRUDERS * 2)
    {
        lcd_lib_draw_stringP(5, 53, PSTR(STRING_CONFIG_H_AUTHOR));
        return;
    }else{
        return;
    }
    lcd_lib_draw_string(5, 53, buffer);
}

static void lcd_menu_maintenance_advanced_return()
{
    doCooldown();
    enquecommand_P(PSTR("G28 X0 Y0"));
    currentMenu = lcd_menu_maintenance_advanced;
}

static void lcd_menu_maintenance_advanced()
{
    lcd_scroll_menu(PSTR("POKROCILE NASTAVENI"), 12 + BED_MENU_OFFSET + EXTRUDERS * 2, lcd_advanced_item, lcd_advanced_details);
    if (lcd_lib_button_pressed)
    {
        if (IS_SELECTED_SCROLL(0))
            lcd_change_to_menu(lcd_menu_maintenance);
        else if (IS_SELECTED_SCROLL(1))
            lcd_change_to_menu(lcd_menu_maintenance_led, 0);
        else if (IS_SELECTED_SCROLL(2))
        {
            active_extruder = 0;
            lcd_change_to_menu(lcd_menu_maintenance_advanced_heatup, 0);
        }
#if EXTRUDERS > 1
        else if (IS_SELECTED_SCROLL(3))
        {
            active_extruder = 1;
            lcd_change_to_menu(lcd_menu_maintenance_advanced_heatup, 0);
        }
#endif
#if TEMP_SENSOR_BED != 0
        else if (IS_SELECTED_SCROLL(2 + EXTRUDERS))
            lcd_change_to_menu(lcd_menu_maintenance_advanced_bed_heatup, 0);
#endif
        else if (IS_SELECTED_SCROLL(2 + BED_MENU_OFFSET + EXTRUDERS))
        {
            lcd_lib_beep();
            enquecommand_P(PSTR("G28 X0 Y0"));
        }
        else if (IS_SELECTED_SCROLL(3 + BED_MENU_OFFSET + EXTRUDERS))
        {
            lcd_lib_beep();
            enquecommand_P(PSTR("G28 Z0"));
        }
        else if (IS_SELECTED_SCROLL(4 + BED_MENU_OFFSET + EXTRUDERS))
        {
            lcd_lib_beep();
            enquecommand_P(PSTR("G28 Z0"));
            enquecommand_P(PSTR("G1 Z40"));
        }
        else if (IS_SELECTED_SCROLL(5 + BED_MENU_OFFSET + EXTRUDERS))
        {
            char buffer[32];
            enquecommand_P(PSTR("G28 X0 Y0"));
            sprintf_P(buffer, PSTR("G1 F%i X%i Y%i"), int(homing_feedrate[0]), X_MAX_LENGTH/2, 10);
            enquecommand(buffer);
            
            lcd_change_to_menu_insert_material(lcd_menu_maintenance_advanced_return);
        }
        else if (IS_SELECTED_SCROLL(6 + BED_MENU_OFFSET + EXTRUDERS))
        {
            set_extrude_min_temp(0);
            active_extruder = 0;
            target_temperature[active_extruder] = material[active_extruder].temperature;
            lcd_change_to_menu(lcd_menu_maintenance_extrude, 0);
        }
#if EXTRUDERS > 1
        else if (IS_SELECTED_SCROLL(7 + BED_MENU_OFFSET + EXTRUDERS))
        {
            set_extrude_min_temp(0);
            active_extruder = 1;
            target_temperature[active_extruder] = material[active_extruder].temperature;
            lcd_change_to_menu(lcd_menu_maintenance_extrude, 0);
        }
#endif
        else if (IS_SELECTED_SCROLL(6 + BED_MENU_OFFSET + EXTRUDERS * 2))
            LCD_EDIT_SETTING_BYTE_PERCENT(fanSpeed, "Rychlost vetraku", "%", 0, 100);
        else if (IS_SELECTED_SCROLL(7 + BED_MENU_OFFSET + EXTRUDERS * 2))
            lcd_change_to_menu(lcd_menu_maintenance_retraction, SCROLL_MENU_ITEM_POS(0));
        else if (IS_SELECTED_SCROLL(8 + BED_MENU_OFFSET + EXTRUDERS * 2))
            lcd_change_to_menu(lcd_menu_maintenance_motion, SCROLL_MENU_ITEM_POS(0));
        else if (IS_SELECTED_SCROLL(9 + BED_MENU_OFFSET + EXTRUDERS * 2))
            lcd_change_to_menu(lcd_menu_advanced_version, SCROLL_MENU_ITEM_POS(0));
        else if (IS_SELECTED_SCROLL(10 + BED_MENU_OFFSET + EXTRUDERS * 2))
            lcd_change_to_menu(lcd_menu_advanced_stats, SCROLL_MENU_ITEM_POS(0));
        else if (IS_SELECTED_SCROLL(11 + BED_MENU_OFFSET + EXTRUDERS * 2))
            lcd_change_to_menu(lcd_menu_advanced_factory_reset, SCROLL_MENU_ITEM_POS(1));
    }
}

static void lcd_menu_maintenance_advanced_heatup()
{
    if (lcd_lib_encoder_pos / ENCODER_TICKS_PER_SCROLL_MENU_ITEM != 0)
    {
        target_temperature[active_extruder] = int(target_temperature[active_extruder]) + (lcd_lib_encoder_pos / ENCODER_TICKS_PER_SCROLL_MENU_ITEM);
        if (target_temperature[active_extruder] < 0)
            target_temperature[active_extruder] = 0;
        if (target_temperature[active_extruder] > HEATER_0_MAXTEMP - 15)
            target_temperature[active_extruder] = HEATER_0_MAXTEMP - 15;
        lcd_lib_encoder_pos = 0;
    }
    if (lcd_lib_button_pressed)
        lcd_change_to_menu(previousMenu, previousEncoderPos);

    lcd_lib_clear();
    lcd_lib_draw_string_centerP(20, PSTR("Teplota trysky:"));
    lcd_lib_draw_string_centerP(53, PSTR("Zmacknete pro navrat"));
    char buffer[16];
    int_to_string(int(dsp_temperature[active_extruder]), buffer, PSTR("C/"));
    int_to_string(int(target_temperature[active_extruder]), buffer+strlen(buffer), PSTR("C"));
    lcd_lib_draw_string_center(30, buffer);
    lcd_lib_update_screen();
}

void lcd_menu_maintenance_extrude()
{
    if (lcd_lib_encoder_pos / ENCODER_TICKS_PER_SCROLL_MENU_ITEM != 0)
    {
        if (printing_state == PRINT_STATE_NORMAL && movesplanned() < 3)
        {
            current_position[E_AXIS] += lcd_lib_encoder_pos * 0.1;
            plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], 10, active_extruder);
            lcd_lib_encoder_pos = 0;
        }
    }
    if (lcd_lib_button_pressed)
    {
        set_extrude_min_temp(EXTRUDE_MINTEMP);
        target_temperature[active_extruder] = 0;
        lcd_change_to_menu(previousMenu, previousEncoderPos);
    }

    lcd_lib_clear();
    lcd_lib_draw_string_centerP(20, PSTR("Teplota trysky:"));
    lcd_lib_draw_string_centerP(40, PSTR("Otacenim extrudujte"));
    lcd_lib_draw_string_centerP(53, PSTR("Zmacknete pro navrat"));
    char buffer[16];
    int_to_string(int(dsp_temperature[active_extruder]), buffer, PSTR("C/"));
    int_to_string(int(target_temperature[active_extruder]), buffer+strlen(buffer), PSTR("C"));
    lcd_lib_draw_string_center(30, buffer);
    lcd_lib_update_screen();
}

#if TEMP_SENSOR_BED != 0
void lcd_menu_maintenance_advanced_bed_heatup()
{
    if (lcd_lib_encoder_pos / ENCODER_TICKS_PER_SCROLL_MENU_ITEM != 0)
    {
        target_temperature_bed = int(target_temperature_bed) + (lcd_lib_encoder_pos / ENCODER_TICKS_PER_SCROLL_MENU_ITEM);
        if (target_temperature_bed < 0)
            target_temperature_bed = 0;
        if (target_temperature_bed > BED_MAXTEMP - 15)
            target_temperature_bed = BED_MAXTEMP - 15;
        lcd_lib_encoder_pos = 0;
    }
    if (lcd_lib_button_pressed)
        lcd_change_to_menu(previousMenu, previousEncoderPos);

    lcd_lib_clear();
    lcd_lib_draw_string_centerP(20, PSTR("Buildplate temp.:"));
    lcd_lib_draw_string_centerP(53, PSTR("Click to return"));
    char buffer[16];
    int_to_string(int(dsp_temperature_bed), buffer, PSTR("C/"));
    int_to_string(int(target_temperature_bed), buffer+strlen(buffer), PSTR("C"));
    lcd_lib_draw_string_center(30, buffer);
    lcd_lib_update_screen();
}
#endif

void lcd_menu_advanced_version()
{
    lcd_info_screen(previousMenu, NULL, PSTR("Zpet"));
    lcd_lib_draw_string_centerP(30, PSTR(STRING_VERSION_CONFIG_H));
    lcd_lib_draw_string_centerP(40, PSTR(STRING_CONFIG_H_AUTHOR));
    lcd_lib_update_screen();
}

void lcd_menu_advanced_stats()
{
    lcd_info_screen(previousMenu, NULL, PSTR("Zpet"));
    lcd_lib_draw_string_centerP(10, PSTR("Tiskarna zapnuta:"));
    char buffer[16];
    char* c = int_to_string(lifetime_minutes / 60, buffer, PSTR(":"));
    if (lifetime_minutes % 60 < 10)
        *c++ = '0';
    c = int_to_string(lifetime_minutes % 60, c);
    lcd_lib_draw_string_center(20, buffer);

    lcd_lib_draw_string_centerP(30, PSTR("Doba tisku:"));
    c = int_to_string(lifetime_print_minutes / 60, buffer, PSTR(":"));
    if (lifetime_print_minutes % 60 < 10)
        *c++ = '0';
    c = int_to_string(lifetime_print_minutes % 60, c);
    strcpy_P(c, PSTR(" Mat:"));
    c += 5;
    c = int_to_string(lifetime_print_centimeters / 100, c, PSTR("m"));
    lcd_lib_draw_string_center(40, buffer);
    lcd_lib_update_screen();
}

static void doFactoryReset()
{
    //Clear the EEPROM settings so they get read from default.
    eeprom_write_byte((uint8_t*)100, 0);
    eeprom_write_byte((uint8_t*)101, 0);
    eeprom_write_byte((uint8_t*)102, 0);
    eeprom_write_byte((uint8_t*)EEPROM_FIRST_RUN_DONE_OFFSET, 0);
    eeprom_write_byte(EEPROM_MATERIAL_COUNT_OFFSET(), 0);

    cli();
    //NOTE: Jumping to address 0 is not a fully proper way to reset.
    // Letting the watchdog timeout is a better reset, but the bootloader does not continue on a watchdog timeout.
    // So we disable interrupts and hope for the best!
    //Jump to address 0x0000
#ifdef __AVR__
    asm volatile(
            "clr	r30		\n\t"
            "clr	r31		\n\t"
            "ijmp	\n\t"
            );
#else
    //TODO
#endif
}

static void lcd_menu_advanced_factory_reset()
{
    lcd_question_screen(NULL, doFactoryReset, PSTR("ANO"), previousMenu, NULL, PSTR("NE"));

    lcd_lib_draw_string_centerP(10, PSTR("Opravdu chcete"));
    lcd_lib_draw_string_centerP(20, PSTR("vse resetovat?"));
    lcd_lib_update_screen();
}


static char* lcd_retraction_item(uint8_t nr)
{
    if (nr == 0)
        strcpy_P(card.longFilename, PSTR("< ZPET"));
    else if (nr == 1)
        strcpy_P(card.longFilename, PSTR("Delka retrakce"));
    else if (nr == 2)
        strcpy_P(card.longFilename, PSTR("Rychlost retrakce"));
    else
        strcpy_P(card.longFilename, PSTR("???"));
    return card.longFilename;
}

static void lcd_retraction_details(uint8_t nr)
{
    char buffer[16];
    if (nr == 0)
        return;
    else if(nr == 1)
        float_to_string(retract_length, buffer, PSTR("mm"));
    else if(nr == 2)
        int_to_string(retract_feedrate / 60 + 0.5, buffer, PSTR("mm/s"));
    lcd_lib_draw_string(5, 53, buffer);
}

static void lcd_menu_maintenance_retraction()
{
    lcd_scroll_menu(PSTR("RETRAKCE"), 3, lcd_retraction_item, lcd_retraction_details);
    if (lcd_lib_button_pressed)
    {
        if (IS_SELECTED_SCROLL(0))
        {
            Config_StoreSettings();
            lcd_change_to_menu(lcd_menu_maintenance_advanced, SCROLL_MENU_ITEM_POS(6 + EXTRUDERS * 2));
        }
        else if (IS_SELECTED_SCROLL(1))
            LCD_EDIT_SETTING_FLOAT001(retract_length, "Delka retrakce", "mm", 0, 50);
        else if (IS_SELECTED_SCROLL(2))
            LCD_EDIT_SETTING_SPEED(retract_feedrate, "Rychlost retrakce", "mm/s", 0, max_feedrate[E_AXIS] * 60);
    }
}

static char* lcd_motion_item(uint8_t nr)
{
    if (nr == 0)
        strcpy_P(card.longFilename, PSTR("< ZPET"));
    else if (nr == 1)
        strcpy_P(card.longFilename, PSTR("Akcelerace"));
    else if (nr == 2)
        strcpy_P(card.longFilename, PSTR("X/Y Jerk"));
    else if (nr == 3)
        strcpy_P(card.longFilename, PSTR("Max rychlost X"));
    else if (nr == 4)
        strcpy_P(card.longFilename, PSTR("Max rychlost Y"));
    else if (nr == 5)
        strcpy_P(card.longFilename, PSTR("Max rychlost Z"));
    else if (nr == 6)
        strcpy_P(card.longFilename, PSTR("Proud motory X/Y"));
    else if (nr == 7)
        strcpy_P(card.longFilename, PSTR("Proud motorem Z"));
    else if (nr == 8)
        strcpy_P(card.longFilename, PSTR("Proud motorem E"));
    else
        strcpy_P(card.longFilename, PSTR("???"));
    return card.longFilename;
}

static void lcd_motion_details(uint8_t nr)
{
    char buffer[16];
    if (nr == 0)
        return;
    else if(nr == 1)
        int_to_string(acceleration, buffer, PSTR("mm/s^2"));
    else if(nr == 2)
        int_to_string(max_xy_jerk, buffer, PSTR("mm/s"));
    else if(nr == 3)
        int_to_string(max_feedrate[X_AXIS], buffer, PSTR("mm/s"));
    else if(nr == 4)
        int_to_string(max_feedrate[Y_AXIS], buffer, PSTR("mm/s"));
    else if(nr == 5)
        int_to_string(max_feedrate[Z_AXIS], buffer, PSTR("mm/s"));
    else if(nr == 6)
        int_to_string(motor_current_setting[0], buffer, PSTR("mA"));
    else if(nr == 7)
        int_to_string(motor_current_setting[1], buffer, PSTR("mA"));
    else if(nr == 8)
        int_to_string(motor_current_setting[2], buffer, PSTR("mA"));
    lcd_lib_draw_string(5, 53, buffer);
}

static void lcd_menu_maintenance_motion()
{
    lcd_scroll_menu(PSTR("MOTORY"), 9, lcd_motion_item, lcd_motion_details);
    if (lcd_lib_button_pressed)
    {
        if (IS_SELECTED_SCROLL(0))
        {
            digipot_current(0, motor_current_setting[0]);
            digipot_current(1, motor_current_setting[1]);
            digipot_current(2, motor_current_setting[2]);
            Config_StoreSettings();
            lcd_change_to_menu(lcd_menu_maintenance_advanced, SCROLL_MENU_ITEM_POS(7));
        }
        else if (IS_SELECTED_SCROLL(1))
            LCD_EDIT_SETTING_FLOAT100(acceleration, "Akcelerace", "mm/s^2", 0, 20000);
        else if (IS_SELECTED_SCROLL(2))
            LCD_EDIT_SETTING_FLOAT1(max_xy_jerk, "X/Y Jerk", "mm/s", 0, 100);
        else if (IS_SELECTED_SCROLL(3))
            LCD_EDIT_SETTING_FLOAT1(max_feedrate[X_AXIS], "Max rychlost X", "mm/s", 0, 1000);
        else if (IS_SELECTED_SCROLL(4))
            LCD_EDIT_SETTING_FLOAT1(max_feedrate[Y_AXIS], "Max rychlost Y", "mm/s", 0, 1000);
        else if (IS_SELECTED_SCROLL(5))
            LCD_EDIT_SETTING_FLOAT1(max_feedrate[Z_AXIS], "Max rychlost Z", "mm/s", 0, 1000);
        else if (IS_SELECTED_SCROLL(6))
            LCD_EDIT_SETTING(motor_current_setting[0], "Proud motorem X/Y", "mA", 0, 1300);
        else if (IS_SELECTED_SCROLL(7))
            LCD_EDIT_SETTING(motor_current_setting[1], "Proud motorem Z", "mA", 0, 1300);
        else if (IS_SELECTED_SCROLL(8))
            LCD_EDIT_SETTING(motor_current_setting[2], "Proud motorem E", "mA", 0, 1300);
    }
}

static char* lcd_led_item(uint8_t nr)
{
    if (nr == 0)
        strcpy_P(card.longFilename, PSTR("< ZPET"));
    else if (nr == 1)
        strcpy_P(card.longFilename, PSTR("Jas"));
    else if (nr == 2)
        strcpy_P(card.longFilename, PSTR(" Vzdy zapnuty"));
    else if (nr == 3)
        strcpy_P(card.longFilename, PSTR(" Vzdy vypnuty"));
    else if (nr == 4)
        strcpy_P(card.longFilename, PSTR(" Zapnout pri tisku"));
    else if (nr == 5)
        strcpy_P(card.longFilename, PSTR(" Zapnout po vytisteni"));
    else
        strcpy_P(card.longFilename, PSTR("???"));
    if (nr - 2 == led_mode)
        card.longFilename[0] = '>';
    return card.longFilename;
}

static void lcd_led_details(uint8_t nr)
{
    char buffer[16];
    if (nr == 0)
        return;
    else if(nr == 1)
    {
        int_to_string(led_brightness_level, buffer, PSTR("%"));
        lcd_lib_draw_string(5, 53, buffer);
    }
}

static void lcd_menu_maintenance_led()
{
    analogWrite(LED_PIN, 255 * int(led_brightness_level) / 100);
    lcd_scroll_menu(PSTR("LED"), 6, lcd_led_item, lcd_led_details);
    if (lcd_lib_button_pressed)
    {
        if (IS_SELECTED_SCROLL(0))
        {
            if (led_mode != LED_MODE_ALWAYS_ON)
                analogWrite(LED_PIN, 0);
            Config_StoreSettings();
            lcd_change_to_menu(lcd_menu_maintenance_advanced, SCROLL_MENU_ITEM_POS(1));
        }
        else if (IS_SELECTED_SCROLL(1))
        {
            LCD_EDIT_SETTING(led_brightness_level, "Jas", "%", 0, 100);
        }
        else if (IS_SELECTED_SCROLL(2))
        {
            led_mode = LED_MODE_ALWAYS_ON;
            lcd_lib_beep();
        }
        else if (IS_SELECTED_SCROLL(3))
        {
            led_mode = LED_MODE_ALWAYS_OFF;
            lcd_lib_beep();
        }
        else if (IS_SELECTED_SCROLL(4))
        {
            led_mode = LED_MODE_WHILE_PRINTING;
            lcd_lib_beep();
        }
        else if (IS_SELECTED_SCROLL(5))
        {
            led_mode = LED_MODE_BLINK_ON_DONE;
            lcd_lib_beep();
        }
    }
}

#endif//ENABLE_ULTILCD2
