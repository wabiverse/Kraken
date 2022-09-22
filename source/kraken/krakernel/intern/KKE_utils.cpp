/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "KLI_math_inline.h"
#include "KLI_string_utils.h"

#include "KKE_utils.h"


KRAKEN_NAMESPACE_BEGIN


std::string kraken_exe_path_init()
{
  return ArchGetExecutablePath();
}

std::string kraken_datafiles_path_init()
{
#if defined(ARCH_OS_WINDOWS)
  return STRCAT(G.main->exe_path, G.main->kraken_version_decimal + "/datafiles/");
#elif defined(ARCH_OS_DARWIN)
  return STRCAT(G.main->exe_path,
                "../../Resources/" + G.main->kraken_version_decimal + "/datafiles/");
#else
  /**
   * On Linux, datafiles directory lies outside of BIN
   * ex. BIN DATAFILES INCLUDE LIB PYTHON */
  return STRCAT(G.main->exe_path, "../datafiles/");
#endif
}

std::string kraken_python_path_init()
{
#if defined(ARCH_OS_WINDOWS)
  return STRCAT(G.main->exe_path, G.main->kraken_version_decimal + "/python/lib/");
#elif defined(ARCH_OS_DARWIN)
  return STRCAT(G.main->exe_path,
                "../../Resources/" + G.main->kraken_version_decimal + "/python/lib/");
#else
  return STRCAT(G.main->exe_path, "../python/lib/python3.9/site-packages");
#endif
}

std::string kraken_fonts_path_init()
{
#if defined(ARCH_OS_WINDOWS)
  return STRCAT(G.main->exe_path, G.main->kraken_version_decimal + "/datafiles/fonts/");
#elif defined(ARCH_OS_DARWIN)
  return STRCAT(G.main->exe_path,
                "../../Resources/" + G.main->kraken_version_decimal + "/datafiles/fonts/");
#else
  return STRCAT(G.main->exe_path, "../datafiles/fonts/");
#endif
}

std::string kraken_icon_path_init()
{
#if defined(ARCH_OS_WINDOWS)
  return STRCAT(G.main->exe_path, G.main->kraken_version_decimal + "/datafiles/icons/");
#elif defined(ARCH_OS_DARWIN)
  return STRCAT(G.main->exe_path,
                "../../Resources/" + G.main->kraken_version_decimal + "/datafiles/icons/");
#else
  return STRCAT(G.main->exe_path, "../datafiles/icons/");
#endif
}

std::string kraken_startup_file_init()
{
#if defined(ARCH_OS_WINDOWS)
  return STRCAT(G.main->exe_path, G.main->kraken_version_decimal + "/datafiles/startup.usda");
#elif defined(ARCH_OS_DARWIN)
  /* The 1 is for NSUserDomainMask */
  static char tempPath[512] = "";
  return STRCAT(
    AnchorSystemPathsCocoa::GetApplicationSupportDir(G.main->kraken_version_decimal.c_str(),
                                                     1,
                                                     tempPath,
                                                     sizeof(tempPath)),
    "config/userpref.usda");
#else
  return STRCAT(G.main->exe_path, "../datafiles/startup.usda");
#endif
}

std::string kraken_ocio_file_init()
{
#if defined(ARCH_OS_WINDOWS)
  return STRCAT(G.main->exe_path,
                G.main->kraken_version_decimal + "/datafiles/colormanagement/config.ocio");
#elif defined(ARCH_OS_DARWIN)
  return STRCAT(G.main->exe_path,
                "../../Resources/" + G.main->kraken_version_decimal +
                  "/datafiles/colormanagement/config.ocio");
#else
  return STRCAT(G.main->exe_path, "../datafiles/colormanagement/config.ocio");
#endif
}

std::string kraken_system_tempdir_path()
{
  return std::filesystem::temp_directory_path().string();
}


/* clang-format off */

#define TEMP_STR_SIZE 256

#define SEP_CHR     '#'
#define SEP_STR     "#"

#define EPS 0.001

#define UN_SC_KM    1000.0f
#define UN_SC_HM    100.0f
#define UN_SC_DAM   10.0f
#define UN_SC_M     1.0f
#define UN_SC_DM    0.1f
#define UN_SC_CM    0.01f
#define UN_SC_MM    0.001f
#define UN_SC_UM    0.000001f

#define UN_SC_MI    1609.344f
#define UN_SC_FUR   201.168f
#define UN_SC_CH    20.1168f
#define UN_SC_YD    0.9144f
#define UN_SC_FT    0.3048f
#define UN_SC_IN    0.0254f
#define UN_SC_MIL   0.0000254f

#define UN_SC_MTON  1000.0f /* Metric ton. */
#define UN_SC_QL    100.0f
#define UN_SC_KG    1.0f
#define UN_SC_HG    0.1f
#define UN_SC_DAG   0.01f
#define UN_SC_G     0.001f
#define UN_SC_MG    0.000001f

#define UN_SC_ITON  907.18474f /* Imperial ton. */
#define UN_SC_CWT   45.359237f
#define UN_SC_ST    6.35029318f
#define UN_SC_LB    0.45359237f
#define UN_SC_OZ    0.028349523125f

#define UN_SC_FAH   0.555555555555f

/* clang-format on */


struct kUnitDef
{
  const char *name;
  /** Abused a bit for the display name. */
  const char *name_plural;
  /** This is used for display. */
  const char *name_short;
  /**
   * Keyboard-friendly ASCII-only version of name_short, can be NULL.
   * If name_short has non-ASCII chars, name_alt should be present.
   */
  const char *name_alt;

  /** Can be NULL. */
  const char *name_display;
  /** When NULL, a transformed version of the name will be taken in some cases. */
  const char *identifier;

  double scalar;
  /** Needed for converting temperatures. */
  double bias;
  int flag;
};

enum
{
  K_UNIT_DEF_NONE = 0,
  /** Use for units that are not used enough to be translated into for common use. */
  K_UNIT_DEF_SUPPRESS = 1,
  /** Display a unit even if its value is 0.1, eg 0.1mm instead of 100um. */
  K_UNIT_DEF_TENTH = 2,
  /** Short unit name is case sensitive, for example to distinguish mW and MW. */
  K_UNIT_DEF_CASE_SENSITIVE = 4,
  /** Short unit name does not have space between it and preceding number. */
  K_UNIT_DEF_NO_SPACE = 8,
};

/* Define a single unit system. */
struct kUnitCollection
{
  const struct kUnitDef *units;
  /** Basic unit index (when user doesn't specify unit explicitly). */
  int base_unit;
  /** Options for this system. */
  int flag;
  /** To quickly find the last item. */
  int length;
};

struct PreferredUnits
{
  int system;
  int rotation;
  int length;
  int mass;
  int time;
  int temperature;
};

static PreferredUnits preferred_units_from_UnitSettings(const UnitSettings *settings)
{
  PreferredUnits units = {0};
  units.system = settings->system;
  units.rotation = settings->system_rotation;
  units.length = settings->length_unit;
  units.mass = settings->mass_unit;
  units.time = settings->time_unit;
  units.temperature = settings->temperature_unit;
  return units;
}


/* clang-format off */

#define UNIT_COLLECTION_LENGTH(def) (ARRAY_SIZE(def) - 1)
#define NULL_UNIT {NULL, NULL, NULL, NULL, NULL, NULL, 0.0, 0.0}

/* Dummy */
static struct kUnitDef kuDummyDef[] = { {"", NULL, "", NULL, NULL, NULL, 1.0, 0.0}, NULL_UNIT};
static struct kUnitCollection kuDummyCollection = {kuDummyDef, 0, 0, sizeof(kuDummyDef)};

/* Lengths. */
static struct kUnitDef kuMetricLenDef[] = {
  {"kilometer",  "kilometers",  "km",  NULL, "Kilometers",     "KILOMETERS",  UN_SC_KM,  0.0, K_UNIT_DEF_NONE},
  {"hectometer", "hectometers", "hm",  NULL, "100 Meters",     "HECTOMETERS", UN_SC_HM,  0.0, K_UNIT_DEF_SUPPRESS},
  {"dekameter",  "dekameters",  "dam", NULL, "10 Meters",      "DEKAMETERS",  UN_SC_DAM, 0.0, K_UNIT_DEF_SUPPRESS},
  {"meter",      "meters",      "m",   NULL, "Meters",         "METERS",      UN_SC_M,   0.0, K_UNIT_DEF_NONE},     /* Base unit. */
  {"decimeter",  "decimeters",  "dm",  NULL, "10 Centimeters", "DECIMETERS",  UN_SC_DM,  0.0, K_UNIT_DEF_SUPPRESS},
  {"centimeter", "centimeters", "cm",  NULL, "Centimeters",    "CENTIMETERS", UN_SC_CM,  0.0, K_UNIT_DEF_NONE},
  {"millimeter", "millimeters", "mm",  NULL, "Millimeters",    "MILLIMETERS", UN_SC_MM,  0.0, K_UNIT_DEF_NONE | K_UNIT_DEF_TENTH},
  {"micrometer", "micrometers", "µm",  "um", "Micrometers",    "MICROMETERS", UN_SC_UM,  0.0, K_UNIT_DEF_NONE},

  /* These get displayed because of float precision problems in the transform header,
   * could work around, but for now probably people won't use these. */
#if 0
  {"nanometer", "Nanometers",     "nm", NULL, 0.000000001, 0.0,   K_UNIT_DEF_NONE},
  {"picometer", "Picometers",     "pm", NULL, 0.000000000001, 0.0, K_UNIT_DEF_NONE},
#endif
  NULL_UNIT,
};
static const struct kUnitCollection kuMetricLenCollection = {kuMetricLenDef, 3, 0, UNIT_COLLECTION_LENGTH(kuMetricLenDef)};

static struct kUnitDef kuImperialLenDef[] = {
  {"mile",    "miles",    "mi",   NULL, "Miles",    "MILES",    UN_SC_MI,  0.0, K_UNIT_DEF_NONE},
  {"furlong", "furlongs", "fur",  NULL, "Furlongs", "FURLONGS", UN_SC_FUR, 0.0, K_UNIT_DEF_SUPPRESS},
  {"chain",   "chains",   "ch",   NULL, "Chains",   "CHAINS",   UN_SC_CH,  0.0, K_UNIT_DEF_SUPPRESS},
  {"yard",    "yards",    "yd",   NULL, "Yards",    "YARDS",    UN_SC_YD,  0.0, K_UNIT_DEF_SUPPRESS},
  {"foot",    "feet",     "'",    "ft", "Feet",     "FEET",     UN_SC_FT,  0.0, K_UNIT_DEF_NONE | K_UNIT_DEF_NO_SPACE}, /* Base unit. */
  {"inch",    "inches",   "\"",   "in", "Inches",   "INCHES",   UN_SC_IN,  0.0, K_UNIT_DEF_NONE | K_UNIT_DEF_NO_SPACE},
  {"thou",    "thou",     "thou", "mil", "Thou",    "THOU",     UN_SC_MIL, 0.0, K_UNIT_DEF_NONE}, /* Plural for "thou" has no 's'. */
  NULL_UNIT,
};
static struct kUnitCollection kuImperialLenCollection = {kuImperialLenDef, 4, 0, UNIT_COLLECTION_LENGTH(kuImperialLenDef)};

/* Areas. */
static struct kUnitDef kuMetricAreaDef[] = {
  {"square kilometer",  "square kilometers",  "km²",  "km2",  "Square Kilometers",  NULL, UN_SC_KM * UN_SC_KM,   0.0, K_UNIT_DEF_NONE},
  {"square hectometer", "square hectometers", "hm²",  "hm2",  "Square Hectometers", NULL, UN_SC_HM * UN_SC_HM,   0.0, K_UNIT_DEF_SUPPRESS},   /* Hectare. */
  {"square dekameter",  "square dekameters",  "dam²", "dam2", "Square Dekameters",  NULL, UN_SC_DAM * UN_SC_DAM, 0.0, K_UNIT_DEF_SUPPRESS},  /* are */
  {"square meter",      "square meters",      "m²",   "m2",   "Square Meters",      NULL, UN_SC_M * UN_SC_M,     0.0, K_UNIT_DEF_NONE},   /* Base unit. */
  {"square decimeter",  "square decimetees",  "dm²",  "dm2",  "Square Decimeters",  NULL, UN_SC_DM * UN_SC_DM,   0.0, K_UNIT_DEF_SUPPRESS},
  {"square centimeter", "square centimeters", "cm²",  "cm2",  "Square Centimeters", NULL, UN_SC_CM * UN_SC_CM,   0.0, K_UNIT_DEF_NONE},
  {"square millimeter", "square millimeters", "mm²",  "mm2",  "Square Millimeters", NULL, UN_SC_MM * UN_SC_MM,   0.0, K_UNIT_DEF_NONE | K_UNIT_DEF_TENTH},
  {"square micrometer", "square micrometers", "µm²",  "um2",  "Square Micrometers", NULL, UN_SC_UM * UN_SC_UM,   0.0, K_UNIT_DEF_NONE},
  NULL_UNIT,
};
static struct kUnitCollection kuMetricAreaCollection = {kuMetricAreaDef, 3, 0, UNIT_COLLECTION_LENGTH(kuMetricAreaDef)};

static struct kUnitDef kuImperialAreaDef[] = {
  {"square mile",    "square miles",    "sq mi", "sq m", "Square Miles",    NULL, UN_SC_MI * UN_SC_MI,   0.0, K_UNIT_DEF_NONE},
  {"square furlong", "square furlongs", "sq fur", NULL,  "Square Furlongs", NULL, UN_SC_FUR * UN_SC_FUR, 0.0, K_UNIT_DEF_SUPPRESS},
  {"square chain",   "square chains",   "sq ch",  NULL,  "Square Chains",   NULL, UN_SC_CH * UN_SC_CH,   0.0, K_UNIT_DEF_SUPPRESS},
  {"square yard",    "square yards",    "sq yd",  NULL,  "Square Yards",    NULL, UN_SC_YD * UN_SC_YD,   0.0, K_UNIT_DEF_NONE},
  {"square foot",    "square feet",     "sq ft",  NULL,  "Square Feet",     NULL, UN_SC_FT * UN_SC_FT,   0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  {"square inch",    "square inches",   "sq in",  NULL,  "Square Inches",   NULL, UN_SC_IN * UN_SC_IN,   0.0, K_UNIT_DEF_NONE},
  {"square thou",    "square thou",     "sq mil", NULL,  "Square Thou",     NULL, UN_SC_MIL * UN_SC_MIL, 0.0, K_UNIT_DEF_NONE},
  NULL_UNIT,
};
static struct kUnitCollection kuImperialAreaCollection = {kuImperialAreaDef, 4, 0, UNIT_COLLECTION_LENGTH(kuImperialAreaDef)};

/* Volumes. */
static struct kUnitDef kuMetricVolDef[] = {
  {"cubic kilometer",  "cubic kilometers",  "km³",  "km3",  "Cubic Kilometers",  NULL, UN_SC_KM * UN_SC_KM * UN_SC_KM,    0.0, K_UNIT_DEF_NONE},
  {"cubic hectometer", "cubic hectometers", "hm³",  "hm3",  "Cubic Hectometers", NULL, UN_SC_HM * UN_SC_HM * UN_SC_HM,    0.0, K_UNIT_DEF_SUPPRESS},
  {"cubic dekameter",  "cubic dekameters",  "dam³", "dam3", "Cubic Dekameters",  NULL, UN_SC_DAM * UN_SC_DAM * UN_SC_DAM, 0.0, K_UNIT_DEF_SUPPRESS},
  {"cubic meter",      "cubic meters",      "m³",   "m3",   "Cubic Meters",      NULL, UN_SC_M * UN_SC_M * UN_SC_M,       0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  {"cubic decimeter",  "cubic decimeters",  "dm³",  "dm3",  "Cubic Decimeters",  NULL, UN_SC_DM * UN_SC_DM * UN_SC_DM,    0.0, K_UNIT_DEF_SUPPRESS},
  {"cubic centimeter", "cubic centimeters", "cm³",  "cm3",  "Cubic Centimeters", NULL, UN_SC_CM * UN_SC_CM * UN_SC_CM,    0.0, K_UNIT_DEF_NONE},
  {"cubic millimeter", "cubic millimeters", "mm³",  "mm3",  "Cubic Millimeters", NULL, UN_SC_MM * UN_SC_MM * UN_SC_MM,    0.0, K_UNIT_DEF_NONE | K_UNIT_DEF_TENTH},
  {"cubic micrometer", "cubic micrometers", "µm³",  "um3",  "Cubic Micrometers", NULL, UN_SC_UM * UN_SC_UM * UN_SC_UM,    0.0, K_UNIT_DEF_NONE},
  NULL_UNIT,
};
static struct kUnitCollection kuMetricVolCollection = {kuMetricVolDef, 3, 0, UNIT_COLLECTION_LENGTH(kuMetricVolDef)};

static struct kUnitDef kuImperialVolDef[] = {
  {"cubic mile",    "cubic miles",    "cu mi",  "cu m", "Cubic Miles",    NULL, UN_SC_MI * UN_SC_MI * UN_SC_MI,    0.0, K_UNIT_DEF_NONE},
  {"cubic furlong", "cubic furlongs", "cu fur", NULL,   "Cubic Furlongs", NULL, UN_SC_FUR * UN_SC_FUR * UN_SC_FUR, 0.0, K_UNIT_DEF_SUPPRESS},
  {"cubic chain",   "cubic chains",   "cu ch",  NULL,   "Cubic Chains",   NULL, UN_SC_CH * UN_SC_CH * UN_SC_CH,    0.0, K_UNIT_DEF_SUPPRESS},
  {"cubic yard",    "cubic yards",    "cu yd",  NULL,   "Cubic Yards",    NULL, UN_SC_YD * UN_SC_YD * UN_SC_YD,    0.0, K_UNIT_DEF_NONE},
  {"cubic foot",    "cubic feet",     "cu ft",  NULL,   "Cubic Feet",     NULL, UN_SC_FT * UN_SC_FT * UN_SC_FT,    0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  {"cubic inch",    "cubic inches",   "cu in",  NULL,   "Cubic Inches",   NULL, UN_SC_IN * UN_SC_IN * UN_SC_IN,    0.0, K_UNIT_DEF_NONE},
  {"cubic thou",    "cubic thou",     "cu mil", NULL,   "Cubic Thou",     NULL, UN_SC_MIL * UN_SC_MIL * UN_SC_MIL, 0.0, K_UNIT_DEF_NONE},
  NULL_UNIT,
};
static struct kUnitCollection kuImperialVolCollection = {kuImperialVolDef, 4, 0, UNIT_COLLECTION_LENGTH(kuImperialVolDef)};

/* Mass. */
static struct kUnitDef kuMetricMassDef[] = {
  {"ton",       "tonnes",     "ton", "t",  "Tonnes",        "TONNES",     UN_SC_MTON, 0.0, K_UNIT_DEF_NONE},
  {"quintal",   "quintals",   "ql",  "q",  "100 Kilograms", "QUINTALS",   UN_SC_QL,   0.0, K_UNIT_DEF_SUPPRESS},
  {"kilogram",  "kilograms",  "kg",  NULL, "Kilograms",     "KILOGRAMS",  UN_SC_KG,   0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  {"hectogram", "hectograms", "hg",  NULL, "Hectograms",    "HECTOGRAMS", UN_SC_HG,   0.0, K_UNIT_DEF_SUPPRESS},
  {"dekagram",  "dekagrams",  "dag", NULL, "10 Grams",      "DEKAGRAMS",  UN_SC_DAG,  0.0, K_UNIT_DEF_SUPPRESS},
  {"gram",      "grams",      "g",   NULL, "Grams",         "GRAMS",      UN_SC_G,    0.0, K_UNIT_DEF_NONE},
  {"milligram", "milligrams", "mg",  NULL, "Milligrams",    "MILLIGRAMS", UN_SC_MG,   0.0, K_UNIT_DEF_NONE},
  NULL_UNIT,
};
static struct kUnitCollection kuMetricMassCollection = {kuMetricMassDef, 2, 0, UNIT_COLLECTION_LENGTH(kuMetricMassDef)};

static struct kUnitDef kuImperialMassDef[] = {
  {"ton",           "tonnes",         "ton", "t",  "Tonnes",         "TONNES",         UN_SC_ITON, 0.0, K_UNIT_DEF_NONE},
  {"centum weight", "centum weights", "cwt", NULL, "Centum weights", "CENTUM_WEIGHTS", UN_SC_CWT,  0.0, K_UNIT_DEF_NONE},
  {"stone",         "stones",         "st",  NULL, "Stones",         "STONES",         UN_SC_ST,   0.0, K_UNIT_DEF_NONE},
  {"pound",         "pounds",         "lb",  NULL, "Pounds",         "POUNDS",         UN_SC_LB,   0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  {"ounce",         "ounces",         "oz",  NULL, "Ounces",         "OUNCES",         UN_SC_OZ,   0.0, K_UNIT_DEF_NONE},
  NULL_UNIT,
};
static struct kUnitCollection kuImperialMassCollection = {kuImperialMassDef, 3, 0, UNIT_COLLECTION_LENGTH(kuImperialMassDef)};

/* Even if user scales the system to a point where km^3 is used, velocity and
 * acceleration aren't scaled: that's why we have so few units for them. */

/* Velocity. */
static struct kUnitDef kuMetricVelDef[] = {
  {"meter per second",   "meters per second",   "m/s",  NULL, "Meters per second",   NULL, UN_SC_M,            0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  {"kilometer per hour", "kilometers per hour", "km/h", NULL, "Kilometers per hour", NULL, UN_SC_KM / 3600.0f, 0.0, K_UNIT_DEF_SUPPRESS},
  NULL_UNIT,
};
static struct kUnitCollection kuMetricVelCollection = {kuMetricVelDef, 0, 0, UNIT_COLLECTION_LENGTH(kuMetricVelDef)};

static struct kUnitDef kuImperialVelDef[] = {
  {"foot per second", "feet per second", "ft/s", "fps", "Feet per second", NULL, UN_SC_FT,           0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  {"mile per hour",   "miles per hour",  "mph",  NULL,  "Miles per hour",  NULL, UN_SC_MI / 3600.0f, 0.0, K_UNIT_DEF_SUPPRESS},
  NULL_UNIT,
};
static struct kUnitCollection kuImperialVelCollection = {kuImperialVelDef, 0, 0, UNIT_COLLECTION_LENGTH(kuImperialVelDef)};

/* Acceleration. */
static struct kUnitDef kuMetricAclDef[] = {
  {"meter per second squared", "meters per second squared", "m/s²", "m/s2", "Meters per second squared", NULL, UN_SC_M, 0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  NULL_UNIT,
};
static struct kUnitCollection kuMetricAclCollection = {kuMetricAclDef, 0, 0, UNIT_COLLECTION_LENGTH(kuMetricAclDef)};

static struct kUnitDef kuImperialAclDef[] = {
  {"foot per second squared", "feet per second squared", "ft/s²", "ft/s2", "Feet per second squared", NULL, UN_SC_FT, 0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  NULL_UNIT,
};
static struct kUnitCollection kuImperialAclCollection = {kuImperialAclDef, 0, 0, UNIT_COLLECTION_LENGTH(kuImperialAclDef)};

/* Time. */
static struct kUnitDef kuNaturalTimeDef[] = {
  /* Weeks? - probably not needed for Blender. */
  {"day",         "days",         "d",   NULL, "Days",         "DAYS",     86400.0,      0.0, K_UNIT_DEF_NONE},
  {"hour",        "hours",        "hr",  "h",  "Hours",        "HOURS",     3600.0,      0.0, K_UNIT_DEF_NONE},
  {"minute",      "minutes",      "min", "m",  "Minutes",      "MINUTES",     60.0,      0.0, K_UNIT_DEF_NONE},
  {"second",      "seconds",      "sec", "s",  "Seconds",      "SECONDS",      1.0,      0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  {"millisecond", "milliseconds", "ms",  NULL, "Milliseconds", "MILLISECONDS", 0.001,    0.0, K_UNIT_DEF_NONE},
  {"microsecond", "microseconds", "µs",  "us", "Microseconds", "MICROSECONDS", 0.000001, 0.0, K_UNIT_DEF_NONE},
  NULL_UNIT,
};
static struct kUnitCollection kuNaturalTimeCollection = {kuNaturalTimeDef, 3, 0, UNIT_COLLECTION_LENGTH(kuNaturalTimeDef)};


static struct kUnitDef kuNaturalRotDef[] = {
  {"degree",    "degrees",     "°",  "d",   "Degrees",    "DEGREES",    M_PI / 180.0,             0.0,  K_UNIT_DEF_NONE | K_UNIT_DEF_NO_SPACE},
  /* arcminutes/arcseconds are used in Astronomy/Navigation areas... */
  {"arcminute", "arcminutes",  "'",  NULL,  "Arcminutes", "ARCMINUTES", (M_PI / 180.0) / 60.0,    0.0,  K_UNIT_DEF_SUPPRESS | K_UNIT_DEF_NO_SPACE},
  {"arcsecond", "arcseconds",  "\"", NULL,  "Arcseconds", "ARCSECONDS", (M_PI / 180.0) / 3600.0,  0.0,  K_UNIT_DEF_SUPPRESS | K_UNIT_DEF_NO_SPACE},
  {"radian",    "radians",     "r",  NULL,  "Radians",    "RADIANS",    1.0,                      0.0,  K_UNIT_DEF_NONE},
#if 0
  {"turn",    "turns",         "t",  NULL,  "Turns",      NULL,         1.0 / (M_PI * 2.0),       0.0,  K_UNIT_DEF_NONE},
#endif
  NULL_UNIT,
};
static struct kUnitCollection kuNaturalRotCollection = {kuNaturalRotDef, 0, 0, UNIT_COLLECTION_LENGTH(kuNaturalRotDef)};

/* Camera Lengths. */
static struct kUnitDef kuCameraLenDef[] = {
  {"meter",      "meters",      "m",   NULL, "Meters",         NULL, UN_SC_KM,  0.0, K_UNIT_DEF_NONE},     /* Base unit. */
  {"decimeter",  "decimeters",  "dm",  NULL, "10 Centimeters", NULL, UN_SC_HM,  0.0, K_UNIT_DEF_SUPPRESS},
  {"centimeter", "centimeters", "cm",  NULL, "Centimeters",    NULL, UN_SC_DAM, 0.0, K_UNIT_DEF_SUPPRESS},
  {"millimeter", "millimeters", "mm",  NULL, "Millimeters",    NULL, UN_SC_M,   0.0, K_UNIT_DEF_NONE},
  {"micrometer", "micrometers", "µm",  "um",  "Micrometers",    NULL, UN_SC_MM,  0.0, K_UNIT_DEF_SUPPRESS},
  NULL_UNIT,
};
static struct kUnitCollection kuCameraLenCollection = {kuCameraLenDef, 3, 0, UNIT_COLLECTION_LENGTH(kuCameraLenDef)};

/* (Light) Power. */
static struct kUnitDef kuPowerDef[] = {
  {"gigawatt",  "gigawatts",  "GW", NULL, "Gigawatts",  NULL, 1e9f,  0.0, K_UNIT_DEF_NONE},
  {"megawatt",  "megawatts",  "MW", NULL, "Megawatts",  NULL, 1e6f,  0.0, K_UNIT_DEF_CASE_SENSITIVE},
  {"kilowatt",  "kilowatts",  "kW", NULL, "Kilowatts",  NULL, 1e3f,  0.0, K_UNIT_DEF_SUPPRESS},
  {"watt",      "watts",      "W",  NULL, "Watts",      NULL, 1.0f,  0.0, K_UNIT_DEF_NONE}, /* Base unit. */
  {"milliwatt", "milliwatts", "mW", NULL, "Milliwatts", NULL, 1e-3f, 0.0, K_UNIT_DEF_CASE_SENSITIVE},
  {"microwatt", "microwatts", "µW", "uW", "Microwatts", NULL, 1e-6f, 0.0, K_UNIT_DEF_NONE},
  {"nanowatt",  "nanowatts",  "nW", NULL, "Nanowatts",  NULL, 1e-9f, 0.0, K_UNIT_DEF_NONE},
  NULL_UNIT,
};
static struct kUnitCollection kuPowerCollection = {kuPowerDef, 3, 0, UNIT_COLLECTION_LENGTH(kuPowerDef)};

/* Temperature */
static struct kUnitDef kuMetricTempDef[] = {
  {"kelvin",  "kelvin",  "K",  NULL, "Kelvin",  "KELVIN",  1.0f, 0.0,    K_UNIT_DEF_NONE}, /* Base unit. */
  {"celsius", "celsius", "°C", "C",  "Celsius", "CELSIUS", 1.0f, 273.15, K_UNIT_DEF_NONE},
  NULL_UNIT,
};
static struct kUnitCollection kuMetricTempCollection = {kuMetricTempDef, 0, 0, UNIT_COLLECTION_LENGTH(kuMetricTempDef)};

static struct kUnitDef kuImperialTempDef[] = {
  {"kelvin",     "kelvin",     "K",  NULL, "Kelvin",     "KELVIN",     1.0f,      0.0,    K_UNIT_DEF_NONE}, /* Base unit. */
  {"fahrenheit", "fahrenheit", "°F", "F",  "Fahrenheit", "FAHRENHEIT", UN_SC_FAH, 459.67, K_UNIT_DEF_NONE},
  NULL_UNIT,
};
static struct kUnitCollection kuImperialTempCollection = {
    kuImperialTempDef, 1, 0, UNIT_COLLECTION_LENGTH(kuImperialTempDef)};


#define UNIT_SYSTEM_TOT (((sizeof(kUnitSystems) / K_UNIT_TYPE_TOT) / sizeof(void *)) - 1)
static const struct kUnitCollection *kUnitSystems[][K_UNIT_TYPE_TOT] = {
    /* Natural. */
    {NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     &kuNaturalRotCollection,
     &kuNaturalTimeCollection,
     &kuNaturalTimeCollection,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL},
    /* Metric. */
    {NULL,
     &kuMetricLenCollection,
     &kuMetricAreaCollection,
     &kuMetricVolCollection,
     &kuMetricMassCollection,
     &kuNaturalRotCollection,
     &kuNaturalTimeCollection,
     &kuNaturalTimeCollection,
     &kuMetricVelCollection,
     &kuMetricAclCollection,
     &kuCameraLenCollection,
     &kuPowerCollection,
     &kuMetricTempCollection},
    /* Imperial. */
    {NULL,
     &kuImperialLenCollection,
     &kuImperialAreaCollection,
     &kuImperialVolCollection,
     &kuImperialMassCollection,
     &kuNaturalRotCollection,
     &kuNaturalTimeCollection,
     &kuNaturalTimeCollection,
     &kuImperialVelCollection,
     &kuImperialAclCollection,
     &kuCameraLenCollection,
     &kuPowerCollection,
     &kuImperialTempCollection},
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
};

/* clang-format on */

static const kUnitCollection *unit_get_system(int system, int type)
{
  KLI_assert((system > -1) && (system < UNIT_SYSTEM_TOT) && (type > -1) &&
             (type < K_UNIT_TYPE_TOT));
  return kUnitSystems[system][type]; /* Select system to use: metric/imperial/other? */
}

static bool is_valid_unit_collection(const kUnitCollection *usys)
{
  return usys != NULL && usys->units[0].name != NULL;
}

static const kUnitDef *get_preferred_display_unit_if_used(int type, PreferredUnits units)
{
  const kUnitCollection *usys = unit_get_system(units.system, type);
  if (!is_valid_unit_collection(usys)) {
    return NULL;
  }

  int max_offset = usys->length - 1;

  switch (type) {
    case K_UNIT_LENGTH:
    case K_UNIT_AREA:
    case K_UNIT_VOLUME:
      if (units.length == USER_UNIT_ADAPTIVE) {
        return NULL;
      }
      return usys->units + MIN2(units.length, max_offset);
    case K_UNIT_MASS:
      if (units.mass == USER_UNIT_ADAPTIVE) {
        return NULL;
      }
      return usys->units + MIN2(units.mass, max_offset);
    case K_UNIT_TIME:
      if (units.time == USER_UNIT_ADAPTIVE) {
        return NULL;
      }
      return usys->units + MIN2(units.time, max_offset);
    case K_UNIT_ROTATION:
      if (units.rotation == 0) {
        return usys->units + 0;
      } else if (units.rotation == USER_UNIT_ROT_RADIANS) {
        return usys->units + 3;
      }
      break;
    case K_UNIT_TEMPERATURE:
      if (units.temperature == USER_UNIT_ADAPTIVE) {
        return NULL;
      }
      return usys->units + MIN2(units.temperature, max_offset);
    default:
      break;
  }
  return NULL;
}

static bool unit_should_be_split(int type)
{
  return ELEM(type, K_UNIT_LENGTH, K_UNIT_MASS, K_UNIT_TIME, K_UNIT_CAMERA);
}

static const kUnitDef *unit_default(const kUnitCollection *usys)
{
  return &usys->units[usys->base_unit];
}

static const kUnitDef *unit_best_fit(double value,
                                     const kUnitCollection *usys,
                                     const kUnitDef *unit_start,
                                     int suppress)
{
  double value_abs = value > 0.0 ? value : -value;

  for (const kUnitDef *unit = unit_start ? unit_start : usys->units; unit->name; unit++) {

    if (suppress && (unit->flag & K_UNIT_DEF_SUPPRESS)) {
      continue;
    }

    /* Scale down scalar so 1cm doesn't convert to 10mm because of float error. */
    if (ARCH_UNLIKELY(unit->flag & K_UNIT_DEF_TENTH)) {
      if (value_abs >= unit->scalar * (0.1 - EPS)) {
        return unit;
      }
    } else {
      if (value_abs >= unit->scalar * (1.0 - EPS)) {
        return unit;
      }
    }
  }

  return unit_default(usys);
}

/* Convert into 2 units and 2 values for "2ft, 3inch" syntax. */
static void unit_dual_convert(double value,
                              const kUnitCollection *usys,
                              kUnitDef const **r_unit_a,
                              kUnitDef const **r_unit_b,
                              double *r_value_a,
                              double *r_value_b,
                              const kUnitDef *main_unit)
{
  const kUnitDef *unit = (main_unit) ? main_unit : unit_best_fit(value, usys, NULL, 1);

  *r_value_a = (value < 0.0) ? (ceil(value / unit->scalar) * unit->scalar) :
                               (floor(value / unit->scalar) * unit->scalar);
  *r_value_b = value - (*r_value_a);

  *r_unit_a = unit;
  *r_unit_b = unit_best_fit(*r_value_b, usys, *r_unit_a, 1);
}

static size_t unit_as_string(char *str,
                             int len_max,
                             double value,
                             int prec,
                             const kUnitCollection *usys,
                             /* Non exposed options. */
                             const kUnitDef *unit,
                             char pad)
{
  if (unit == NULL) {
    if (value == 0.0) {
      /* Use the default units since there is no way to convert. */
      unit = unit_default(usys);
    } else {
      unit = unit_best_fit(value, usys, NULL, 1);
    }
  }

  double value_conv = (value / unit->scalar) - unit->bias;
  bool strip_skip = false;

  /* Negative precision is used to disable stripping of zeroes.
   * This reduces text jumping when changing values. */
  if (prec < 0) {
    strip_skip = true;
    prec *= -1;
  }

  /* Adjust precision to expected number of significant digits.
   * Note that here, we shall not have to worry about very big/small numbers, units are expected
   * to replace 'scientific notation' in those cases. */
  prec -= integer_digits_d(value_conv);

  CLAMP(prec, 0, 6);

  /* Convert to a string. */
  size_t len = KLI_snprintf_rlen(str, len_max, "%.*f", prec, value_conv);

  /* Add unit prefix and strip zeros. */

  /* Replace trailing zero's with spaces so the number
   * is less complicated but alignment in a button won't
   * jump about while dragging. */
  size_t i = len - 1;

  if (prec > 0) {
    if (!strip_skip) {
      while (i > 0 && str[i] == '0') { /* 4.300 -> 4.3 */
        str[i--] = pad;
      }

      if (i > 0 && str[i] == '.') { /* 10. -> 10 */
        str[i--] = pad;
      }
    }
  }

  /* Now add a space for all units except foot, inch, degree, arcminute, arcsecond. */
  if (!(unit->flag & K_UNIT_DEF_NO_SPACE)) {
    str[++i] = ' ';
  }

  /* Now add the suffix. */
  if (i < len_max) {
    int j = 0;
    i++;
    while (unit->name_short[j] && (i < len_max)) {
      str[i++] = unit->name_short[j++];
    }
  }

  /* Terminate no matter what's done with padding above. */
  if (i >= len_max) {
    i = len_max - 1;
  }

  str[i] = '\0';
  return i;
}

static size_t unit_as_string_split_pair(char *str,
                                        int len_max,
                                        double value,
                                        int prec,
                                        const kUnitCollection *usys,
                                        const kUnitDef *main_unit)
{
  const kUnitDef *unit_a, *unit_b;
  double value_a, value_b;
  unit_dual_convert(value, usys, &unit_a, &unit_b, &value_a, &value_b, main_unit);

  /* Check the 2 is a smaller unit. */
  if (unit_b > unit_a) {
    size_t i = unit_as_string(str, len_max, value_a, prec, usys, unit_a, '\0');

    prec -= integer_digits_d(value_a / unit_b->scalar) -
            integer_digits_d(value_b / unit_b->scalar);
    prec = max_ii(prec, 0);

    /* Is there enough space for at least 1 char of the next unit? */
    if (i + 2 < len_max) {
      str[i++] = ' ';

      /* Use low precision since this is a smaller unit. */
      i += unit_as_string(str + i, len_max - i, value_b, prec, usys, unit_b, '\0');
    }
    return i;
  }

  return -1;
}

/* Return the length of the generated string. */
static size_t unit_as_string_main(char *str,
                                  int len_max,
                                  double value,
                                  int prec,
                                  int type,
                                  bool split,
                                  bool pad,
                                  PreferredUnits units)
{
  const kUnitCollection *usys = unit_get_system(units.system, type);
  const kUnitDef *main_unit = NULL;

  if (!is_valid_unit_collection(usys)) {
    usys = &kuDummyCollection;
  } else {
    main_unit = get_preferred_display_unit_if_used(type, units);
  }

  if (split && unit_should_be_split(type)) {
    int length = unit_as_string_split_pair(str, len_max, value, prec, usys, main_unit);
    /* Failed when length is negative, fallback to no split. */
    if (length >= 0) {
      return length;
    }
  }

  return unit_as_string(str, len_max, value, prec, usys, main_unit, pad ? ' ' : '\0');
}

size_t KKE_unit_value_as_string(char *str,
                                int len_max,
                                double value,
                                int prec,
                                int type,
                                const UnitSettings *settings,
                                bool pad)
{
  bool do_split = (settings->flag & USER_UNIT_OPT_SPLIT) != 0;
  PreferredUnits units = preferred_units_from_UnitSettings(settings);
  return unit_as_string_main(str, len_max, value, prec, type, do_split, pad, units);
}

KRAKEN_NAMESPACE_END