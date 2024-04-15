/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _DBUSADDONS_FCITXQTDBUSTYPES_H_
#define _DBUSADDONS_FCITXQTDBUSTYPES_H_

#include "fcitx5qt4dbusaddons_export.h"

#include <QDBusArgument>
#include <QList>
#include <QMetaType>
#include <type_traits>

namespace fcitx {

FCITX5QT4DBUSADDONS_EXPORT void registerFcitxQtDBusTypes();

#define FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(TYPE)                                \
    class FCITX5QT4DBUSADDONS_EXPORT TYPE {                                    \
    public:

#define FCITX5_QT_DECLARE_FIELD(TYPE, GETTER, SETTER)                          \
public:                                                                        \
    std::conditional_t<std::is_class<TYPE>::value, const TYPE &, TYPE>         \
    GETTER() const {                                                           \
        return GETTER##_;                                                      \
    }                                                                          \
    void SETTER(                                                               \
        std::conditional_t<std::is_class<TYPE>::value, const TYPE &, TYPE>     \
            value) {                                                           \
        GETTER##_ = value;                                                     \
    }                                                                          \
                                                                               \
private:                                                                       \
    TYPE GETTER##_ = TYPE();

#define FCITX5_QT_END_DECLARE_DBUS_TYPE(TYPE)                                  \
    }                                                                          \
    ;                                                                          \
    typedef QList<TYPE> TYPE##List;                                            \
    FCITX5QT4DBUSADDONS_EXPORT QDBusArgument &operator<<(                      \
        QDBusArgument &argument, const TYPE &value);                           \
    FCITX5QT4DBUSADDONS_EXPORT const QDBusArgument &operator>>(                \
        const QDBusArgument &argument, TYPE &value);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtFormattedPreedit);
FCITX5_QT_DECLARE_FIELD(QString, string, setString);
FCITX5_QT_DECLARE_FIELD(qint32, format, setFormat);

public:
bool operator==(const FcitxQtFormattedPreedit &preedit) const;
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtFormattedPreedit);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtStringKeyValue);
FCITX5_QT_DECLARE_FIELD(QString, key, setKey);
FCITX5_QT_DECLARE_FIELD(QString, value, setValue);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtStringKeyValue);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtInputMethodEntry);
FCITX5_QT_DECLARE_FIELD(QString, uniqueName, setUniqueName);
FCITX5_QT_DECLARE_FIELD(QString, name, setName);
FCITX5_QT_DECLARE_FIELD(QString, nativeName, setNativeName);
FCITX5_QT_DECLARE_FIELD(QString, icon, setIcon);
FCITX5_QT_DECLARE_FIELD(QString, label, setLabel);
FCITX5_QT_DECLARE_FIELD(QString, languageCode, setLanguageCode);
FCITX5_QT_DECLARE_FIELD(bool, configurable, setConfigurable);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtInputMethodEntry);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtVariantInfo);
FCITX5_QT_DECLARE_FIELD(QString, variant, setVariant);
FCITX5_QT_DECLARE_FIELD(QString, description, setDescription);
FCITX5_QT_DECLARE_FIELD(QStringList, languages, setLanguages);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtVariantInfo);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtLayoutInfo);
FCITX5_QT_DECLARE_FIELD(QString, layout, setLayout);
FCITX5_QT_DECLARE_FIELD(QString, description, setDescription);
FCITX5_QT_DECLARE_FIELD(QStringList, languages, setLanguages);
FCITX5_QT_DECLARE_FIELD(FcitxQtVariantInfoList, variants, setVariants);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtLayoutInfo);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtConfigOption);
FCITX5_QT_DECLARE_FIELD(QString, name, setName);
FCITX5_QT_DECLARE_FIELD(QString, type, setType);
FCITX5_QT_DECLARE_FIELD(QString, description, setDescription);
FCITX5_QT_DECLARE_FIELD(QDBusVariant, defaultValue, setDefaultValue);
FCITX5_QT_DECLARE_FIELD(QVariantMap, properties, setProperties);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtConfigOption);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtConfigType);
FCITX5_QT_DECLARE_FIELD(QString, name, setName);
FCITX5_QT_DECLARE_FIELD(FcitxQtConfigOptionList, options, setOptions);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtConfigType);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtAddonInfo);
FCITX5_QT_DECLARE_FIELD(QString, uniqueName, setUniqueName);
FCITX5_QT_DECLARE_FIELD(QString, name, setName);
FCITX5_QT_DECLARE_FIELD(QString, comment, setComment);
FCITX5_QT_DECLARE_FIELD(int, category, setCategory);
FCITX5_QT_DECLARE_FIELD(bool, configurable, setConfigurable);
FCITX5_QT_DECLARE_FIELD(bool, enabled, setEnabled);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtAddonInfo);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtAddonInfoV2);
FCITX5_QT_DECLARE_FIELD(QString, uniqueName, setUniqueName);
FCITX5_QT_DECLARE_FIELD(QString, name, setName);
FCITX5_QT_DECLARE_FIELD(QString, comment, setComment);
FCITX5_QT_DECLARE_FIELD(int, category, setCategory);
FCITX5_QT_DECLARE_FIELD(bool, configurable, setConfigurable);
FCITX5_QT_DECLARE_FIELD(bool, enabled, setEnabled);
FCITX5_QT_DECLARE_FIELD(bool, onDemand, setOnDemand);
FCITX5_QT_DECLARE_FIELD(QStringList, dependencies, setDependencies);
FCITX5_QT_DECLARE_FIELD(QStringList, optionalDependencies,
                        setOptionalDependencies);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtAddonInfoV2);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtAddonState);
FCITX5_QT_DECLARE_FIELD(QString, uniqueName, setUniqueName);
FCITX5_QT_DECLARE_FIELD(bool, enabled, setEnabled);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtAddonState);
} // namespace fcitx

Q_DECLARE_METATYPE(fcitx::FcitxQtFormattedPreedit)
Q_DECLARE_METATYPE(fcitx::FcitxQtFormattedPreeditList)

Q_DECLARE_METATYPE(fcitx::FcitxQtStringKeyValue)
Q_DECLARE_METATYPE(fcitx::FcitxQtStringKeyValueList)

Q_DECLARE_METATYPE(fcitx::FcitxQtInputMethodEntry)
Q_DECLARE_METATYPE(fcitx::FcitxQtInputMethodEntryList)

Q_DECLARE_METATYPE(fcitx::FcitxQtVariantInfo)
Q_DECLARE_METATYPE(fcitx::FcitxQtVariantInfoList)

Q_DECLARE_METATYPE(fcitx::FcitxQtLayoutInfo)
Q_DECLARE_METATYPE(fcitx::FcitxQtLayoutInfoList)

Q_DECLARE_METATYPE(fcitx::FcitxQtConfigOption)
Q_DECLARE_METATYPE(fcitx::FcitxQtConfigOptionList)

Q_DECLARE_METATYPE(fcitx::FcitxQtConfigType)
Q_DECLARE_METATYPE(fcitx::FcitxQtConfigTypeList)

Q_DECLARE_METATYPE(fcitx::FcitxQtAddonInfo)
Q_DECLARE_METATYPE(fcitx::FcitxQtAddonInfoList)

Q_DECLARE_METATYPE(fcitx::FcitxQtAddonInfoV2)
Q_DECLARE_METATYPE(fcitx::FcitxQtAddonInfoV2List)

Q_DECLARE_METATYPE(fcitx::FcitxQtAddonState)
Q_DECLARE_METATYPE(fcitx::FcitxQtAddonStateList)

#endif // _DBUSADDONS_FCITXQTDBUSTYPES_H_
