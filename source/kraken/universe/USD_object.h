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

#pragma once

/**
 * @file USD_object.h
 * @ingroup UNI
 * The @a central foundation for @a all data access.
 */

#include "USD_wm_types.h"
#include "USD_ID.h"

#include "KLI_string.h"

#ifdef __cplusplus
#  include <wabi/usd/usd/attribute.h>
#  include <wabi/usd/usd/property.h>
#  include <wabi/usd/usd/collectionAPI.h>
#  include <wabi/usd/usd/prim.h>
#endif /* __cplusplus */

typedef struct IDProperty **(*IDPropertiesFunc)(struct KrakenPRIM *ptr);

enum PropertyType
{
  PROP_BOOLEAN = 0,
  PROP_INT = 1,
  PROP_FLOAT = 2,
  PROP_STRING = 3,
  PROP_ENUM = 4,
  PROP_POINTER = 5,
  PROP_COLLECTION = 6,
};

enum FunctionFlag
{
  FUNC_USE_SELF_ID = (1 << 11),
  FUNC_NO_SELF = (1 << 0),
  FUNC_USE_SELF_TYPE = (1 << 1),
  FUNC_USE_MAIN = (1 << 2),
  FUNC_USE_CONTEXT = (1 << 3),
  FUNC_USE_REPORTS = (1 << 4),
  FUNC_REGISTER = (1 << 5),
  FUNC_REGISTER_OPTIONAL = FUNC_REGISTER | (1 << 6),
  FUNC_ALLOW_WRITE = (1 << 12),
  FUNC_RUNTIME = (1 << 9),
};

/* Make sure enums are updated with these */
/* HIGHEST FLAG IN USE: 1 << 31
 * FREE FLAGS: 2, 9, 11, 13, 14, 15. */
enum PropertyFlag
{
  /**
   * Editable means the property is editable in the user
   * interface, properties are editable by default except
   * for pointers and collections.
   */
  PROP_EDITABLE = (1 << 0),
  /**
   * This property is editable even if it is lib linked,
   * meaning it will get lost on reload, but it's useful
   * for editing.
   */
  PROP_LIB_EXCEPTION = (1 << 16),
  /**
   * Animatable means the property can be driven by some
   * other input, be it animation curves, expressions, ..
   * properties are animatable by default except for pointers
   * and collections.
   */
  PROP_ANIMATABLE = (1 << 1),
  /**
   * This flag means when the property's widget is in 'text-edit' mode, it will be updated
   * after every typed char, instead of waiting final validation. Used e.g. for text search-box.
   * It will also cause UI_BUT_VALUE_CLEAR to be set for text buttons. We could add an own flag
   * for search/filter properties, but this works just fine for now.
   */
  PROP_TEXTEDIT_UPDATE = (1u << 31),

  /* icon */
  PROP_ICONS_CONSECUTIVE = (1 << 12),
  PROP_ICONS_REVERSE = (1 << 8),

  /** Hidden in the user interface. */
  PROP_HIDDEN = (1 << 19),
  /** Do not write in presets. */
  PROP_SKIP_SAVE = (1 << 28),

  /* numbers */

  /** Each value is related proportionally (object scale, image size). */
  PROP_PROPORTIONAL = (1 << 26),

  /* pointers */
  PROP_ID_REFCOUNT = (1 << 6),

  /**
   * Disallow assigning a variable to itself, eg an object tracking itself
   * only apply this to types that are derived from an ID ().
   */
  PROP_ID_SELF_CHECK = (1 << 20),
  /**
   * Use for...
   * - pointers: in the UI and python so unsetting or setting to None won't work.
   * - strings: so our internal generated get/length/set
   *   functions know to do NULL checks before access T30865.
   */
  PROP_NEVER_NULL = (1 << 18),
  /**
   * Currently only used for UI, this is similar to PROP_NEVER_NULL
   * except that the value may be NULL at times, used for ObData, where an Empty's will be NULL
   * but setting NULL on a mesh object is not possible.
   * So if it's not NULL, setting NULL can't be done!
   */
  PROP_NEVER_UNLINK = (1 << 25),

  /**
   * Pointers to data that is not owned by the struct.
   * Typical example: Bone.parent, Bone.child, etc., and nearly all ID pointers.
   * This is crucial information for processes that walk the whole data of an ID e.g.
   * (like library override).
   * Note that all ID pointers are enforced to this by default,
   * this probably will need to be rechecked
   * (see ugly infamous node-trees of material/texture/scene/etc.).
   */
  PROP_PTR_NO_OWNERSHIP = (1 << 7),

  /**
   * flag contains multiple enums.
   * NOTE: not to be confused with `prop->enumbitflags`
   * this exposes the flag as multiple options in python and the UI.
   *
   * @note These can't be animated so use with care.
   */
  PROP_ENUM_FLAG = (1 << 21),

  /* need context for update function */
  PROP_CONTEXT_UPDATE = (1 << 22),
  PROP_CONTEXT_PROPERTY_UPDATE = PROP_CONTEXT_UPDATE | (1 << 27),

  /* registering */
  PROP_REGISTER = (1 << 4),
  PROP_REGISTER_OPTIONAL = PROP_REGISTER | (1 << 5),

  /**
   * Use for allocated function return values of arrays or strings
   * for any data that should not have a reference kept.
   *
   * It can be used for properties which are dynamically allocated too.
   *
   * @note Currently dynamic sized thick wrapped data isn't supported.
   * This would be a useful addition and avoid a fixed maximum sized as in done at the moment.
   */
  PROP_THICK_WRAP = (1 << 23),

  /** This is an IDProperty, not a DNA one. */
  PROP_IDPROPERTY = (1 << 10),
  /** For dynamic arrays, and retvals of type string. */
  PROP_DYNAMIC = (1 << 17),
  /** For enum that shouldn't be contextual */
  PROP_ENUM_NO_CONTEXT = (1 << 24),
  /** For enums not to be translated (e.g. viewlayers' names in nodes). */
  PROP_ENUM_NO_TRANSLATE = (1 << 29),
  PROP_NO_DEG_UPDATE = (1 << 30),
};

enum PropertyUnit
{
  PROP_UNIT_NONE = (0 << 16),
  PROP_UNIT_LENGTH = (1 << 16),        /* m */
  PROP_UNIT_AREA = (2 << 16),          /* m^2 */
  PROP_UNIT_VOLUME = (3 << 16),        /* m^3 */
  PROP_UNIT_MASS = (4 << 16),          /* kg */
  PROP_UNIT_ROTATION = (5 << 16),      /* radians */
  PROP_UNIT_TIME = (6 << 16),          /* frame */
  PROP_UNIT_TIME_ABSOLUTE = (7 << 16), /* time in seconds (independent of scene) */
  PROP_UNIT_VELOCITY = (8 << 16),      /* m/s */
  PROP_UNIT_ACCELERATION = (9 << 16),  /* m/(s^2) */
  PROP_UNIT_CAMERA = (10 << 16),       /* mm */
  PROP_UNIT_POWER = (11 << 16),        /* W */
  PROP_UNIT_TEMPERATURE = (12 << 16),  /* C */
};

enum PropertySubType
{
  PROP_NONE = 0,

  /* strings */
  PROP_FILEPATH = 1,
  PROP_DIRPATH = 2,
  PROP_FILENAME = 3,
  /** A string which should be represented as bytes in python, NULL terminated though. */
  PROP_BYTESTRING = 4,
  /* 5 was used by "PROP_TRANSLATE" sub-type, which is now a flag. */
  /** A string which should not be displayed in UI. */
  PROP_PASSWORD = 6,

  /* numbers */
  /** A dimension in pixel units, possibly before DPI scaling (so value may not be the final pixel
   * value but the one to apply DPI scale to). */
  PROP_PIXEL = 12,
  PROP_UNSIGNED = 13,
  PROP_PERCENTAGE = 14,
  PROP_FACTOR = 15,
  PROP_ANGLE = 16 | PROP_UNIT_ROTATION,
  PROP_TIME = 17 | PROP_UNIT_TIME,
  PROP_TIME_ABSOLUTE = 17 | PROP_UNIT_TIME_ABSOLUTE,
  /** Distance in 3d space, don't use for pixel distance for eg. */
  PROP_DISTANCE = 18 | PROP_UNIT_LENGTH,
  PROP_DISTANCE_CAMERA = 19 | PROP_UNIT_CAMERA,

  /* number arrays */
  PROP_COLOR = 20,
  PROP_TRANSLATION = 21 | PROP_UNIT_LENGTH,
  PROP_DIRECTION = 22,
  PROP_VELOCITY = 23 | PROP_UNIT_VELOCITY,
  PROP_ACCELERATION = 24 | PROP_UNIT_ACCELERATION,
  PROP_MATRIX = 25,
  PROP_EULER = 26 | PROP_UNIT_ROTATION,
  PROP_QUATERNION = 27,
  PROP_AXISANGLE = 28,
  PROP_XYZ = 29,
  PROP_XYZ_LENGTH = 29 | PROP_UNIT_LENGTH,
  /** Used for colors which would be color managed before display. */
  PROP_COLOR_GAMMA = 30,
  /** Generic array, no units applied, only that x/y/z/w are used (Python vector). */
  PROP_COORDS = 31,

  /* booleans */
  PROP_LAYER = 40,
  PROP_LAYER_MEMBER = 41,

  /** Light */
  PROP_POWER = 42 | PROP_UNIT_POWER,

  /* temperature */
  PROP_TEMPERATURE = 43 | PROP_UNIT_TEMPERATURE,
};

struct KrakenPROP : public wabi::UsdAttribute
{
  KrakenPROP(const wabi::UsdAttribute &attr = wabi::UsdAttribute())
    : wabi::UsdAttribute(attr)
  {}

  /**
   * Create a KrakenPROP from a UsdProperty.
   * 
   * @NOTE:
   * Not actually a valid Kraken "PROP" as it is just a relational
   * property. So it has no specified type and is just used for the
   * purposes of relational schematics.
   */
  KrakenPROP(const wabi::UsdProperty &prop)
    : wabi::UsdAttribute(wabi::UsdAttribute()),
      intern_prop(prop)
  {}

  wabi::TfToken name;
  PropertyType type;
  PropertyFlag flag;
  PropertySubType subtype;
  int icon;
  
  wabi::UsdProperty intern_prop;
};

typedef std::vector<KrakenPROP *> PropertyVectorLUXO;

typedef int (*ObjectValidateFunc)(const wabi::UsdPrim &ptr, void *data, int *have_function);
typedef int (*ObjectCallbackFunc)(struct kContext *C,
                                  const wabi::UsdPrim &ptr,
                                  void *func,
                                  ListBase list);
typedef void (*ObjectFreeFunc)(void *data);
typedef struct KrakenPRIM *(*ObjectRegisterFunc)(struct Main *kmain,
                                                 struct ReportList *reports,
                                                 void *data,
                                                 const char *identifier,
                                                 ObjectValidateFunc validate,
                                                 ObjectCallbackFunc call,
                                                 ObjectFreeFunc free);
typedef void (*ObjectUnregisterFunc)(struct Main *kmain, const wabi::UsdPrim &type);
typedef void **(*ObjectInstanceFunc)(struct KrakenPRIM *ptr);

typedef void (*PropStringGetFunc)(struct KrakenPRIM *ptr, char *value);
typedef int (*PropStringLengthFunc)(struct KrakenPRIM *ptr);
typedef void (*PropStringSetFunc)(struct KrakenPRIM *ptr, const char *value);
typedef int (*PropEnumGetFunc)(struct KrakenPRIM *ptr);
typedef void (*PropStringGetFuncEx)(struct KrakenPRIM *ptr, KrakenPROP *prop, char *value);
typedef int (*PropStringLengthFuncEx)(struct KrakenPRIM *ptr, KrakenPROP *prop);
typedef void (*PropStringSetFuncEx)(struct KrakenPRIM *ptr, KrakenPROP *prop, const char *value);
typedef int (*PropEnumGetFuncEx)(struct KrakenPRIM *ptr, KrakenPROP *prop);
typedef void (*PropEnumSetFuncEx)(struct KrakenPRIM *ptr, KrakenPROP *prop, int value);

typedef std::vector<wabi::UsdCollectionAPI> UsdCollectionsVector;

typedef struct StringPropertySearchVisitParams
{
  /** Text being searched for (never NULL). */
  const char *text;
  /** Additional information to display (optional, may be NULL). */
  const char *info;
} StringPropertySearchVisitParams;
typedef void (*StringPropertySearchVisitFunc)(void *visit_user_data,
                                              const StringPropertySearchVisitParams *params);
typedef void (*StringPropertySearchFunc)(const struct kContext *C,
                                         struct KrakenPRIM *ptr,
                                         KrakenPROP *prop,
                                         const char *edit_text,
                                         StringPropertySearchVisitFunc visit_fn,
                                         void *visit_user_data);

typedef enum eStringPropertySearchFlag
{
  PROP_STRING_SEARCH_SUPPORTED = (1 << 0),
  PROP_STRING_SEARCH_SORT = (1 << 1),
  PROP_STRING_SEARCH_SUGGESTION = (1 << 2),
} eStringPropertySearchFlag;

struct KrakenPROPString
{
  KrakenPROP property;

  PropStringGetFunc get;
  PropStringLengthFunc length;
  PropStringSetFunc set;

  PropStringGetFuncEx get_ex;
  PropStringLengthFuncEx length_ex;
  PropStringSetFuncEx set_ex;

  StringPropertySearchFunc search;
  eStringPropertySearchFlag search_flag;

  int maxlength;

  const char *defaultvalue;
};

struct ParameterList
{
  void *data;
  struct KrakenFUNC *func;
  int alloc_size;
};

typedef struct KrakenPRIM *(*StructRefineFunc)(struct KrakenPRIM *ptr);

typedef void (*CallFunc)(struct kContext *C,
                         struct ReportList *reports,
                         struct KrakenPRIM *ptr,
                         struct ParameterList *parms);


struct KrakenFUNC
{
  std::vector<KrakenFUNC> cont;

  const char *identifier;

  int flag;

  const char *description;

  CallFunc call;

  KrakenPRIM *c_ret;
};

struct KrakenPRIM : public wabi::UsdPrim
{
  KrakenPRIM(const wabi::UsdPrim &prim = wabi::UsdPrim()) : wabi::UsdPrim(prim) {}

  KrakenPRIM(const KrakenPRIM *prim) : wabi::UsdPrim(prim->GetPrim()) {}

  struct ID *owner_id;
  wabi::TfToken identifier;
  wabi::UsdCollectionAPI collection;
  KrakenPRIM *type;

  /**
   * context (C) */
  void *data;

  KrakenPRIM *base;
  StructRefineFunc refine;

  void *py_type;

  short flag;
  int icon;

  /** Return the location of the struct's pointer to the root group IDProperty. */
  IDPropertiesFunc idproperties;

  PropertyVectorLUXO props;

  ObjectRegisterFunc reg;
  ObjectUnregisterFunc unreg;
  ObjectInstanceFunc instance;

  std::vector<KrakenPRIM *> functions;
};

/* is this ID type used as object data */
#define OB_DATA_SUPPORT_ID(_id_type) \
  (ELEM(_id_type,                    \
        ID_ME,                       \
        ID_CU_LEGACY,                \
        ID_MB,                       \
        ID_LA,                       \
        ID_SPK,                      \
        ID_LP,                       \
        ID_CA,                       \
        ID_LT,                       \
        ID_GD,                       \
        ID_AR,                       \
        ID_CV,                       \
        ID_PT,                       \
        ID_VO))
