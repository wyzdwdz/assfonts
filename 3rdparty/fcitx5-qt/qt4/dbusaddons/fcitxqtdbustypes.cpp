/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <QDBusMetaType>

#include "fcitxqtdbustypes.h"

namespace fcitx {

#define FCITX5_QT_DEFINE_DBUS_TYPE(TYPE)                                       \
    qRegisterMetaType<TYPE>(#TYPE);                                            \
    qDBusRegisterMetaType<TYPE>();                                             \
    qRegisterMetaType<TYPE##List>(#TYPE "List");                               \
    qDBusRegisterMetaType<TYPE##List>();

void registerFcitxQtDBusTypes() {
    FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtFormattedPreedit);
    FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtStringKeyValue);
    FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtInputMethodEntry);
    FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtLayoutInfo);
    FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtVariantInfo);
    FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtConfigOption);
    FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtConfigType);
    FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtAddonInfo);
    FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtAddonState);
    FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtAddonInfoV2);
}

bool FcitxQtFormattedPreedit::operator==(
    const FcitxQtFormattedPreedit &preedit) const {
    return (preedit.format_ == format_) && (preedit.string_ == string_);
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtFormattedPreedit &preedit) {
    argument.beginStructure();
    argument << preedit.string();
    argument << preedit.format();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtFormattedPreedit &preedit) {
    QString str;
    qint32 format;
    argument.beginStructure();
    argument >> str >> format;
    argument.endStructure();
    preedit.setString(str);
    preedit.setFormat(format);
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtStringKeyValue &arg) {
    argument.beginStructure();
    argument << arg.key();
    argument << arg.value();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtStringKeyValue &arg) {
    QString key, value;
    argument.beginStructure();
    argument >> key >> value;
    argument.endStructure();
    arg.setKey(key);
    arg.setValue(value);
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtInputMethodEntry &arg) {
    argument.beginStructure();
    argument << arg.uniqueName();
    argument << arg.name();
    argument << arg.nativeName();
    argument << arg.icon();
    argument << arg.label();
    argument << arg.languageCode();
    argument << arg.configurable();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtInputMethodEntry &arg) {
    QString uniqueName, name, nativeName, icon, label, languageCode;
    bool configurable;
    argument.beginStructure();
    argument >> uniqueName >> name >> nativeName >> icon >> label >>
        languageCode >> configurable;
    argument.endStructure();
    arg.setUniqueName(uniqueName);
    arg.setName(name);
    arg.setNativeName(nativeName);
    arg.setIcon(icon);
    arg.setLabel(label);
    arg.setLanguageCode(languageCode);
    arg.setConfigurable(configurable);
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtVariantInfo &arg) {
    argument.beginStructure();
    argument << arg.variant();
    argument << arg.description();
    argument << arg.languages();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtVariantInfo &arg) {
    QString variant, description;
    QStringList languages;
    argument.beginStructure();
    argument >> variant >> description >> languages;
    argument.endStructure();
    arg.setVariant(variant);
    arg.setDescription(description);
    arg.setLanguages(languages);
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtLayoutInfo &arg) {
    argument.beginStructure();
    argument << arg.layout();
    argument << arg.description();
    argument << arg.languages();
    argument << arg.variants();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtLayoutInfo &arg) {
    QString layout, description;
    QStringList languages;
    FcitxQtVariantInfoList variants;
    argument.beginStructure();
    argument >> layout >> description >> languages >> variants;
    argument.endStructure();
    arg.setLayout(layout);
    arg.setDescription(description);
    arg.setLanguages(languages);
    arg.setVariants(variants);
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtConfigOption &arg) {
    argument.beginStructure();
    argument << arg.name();
    argument << arg.type();
    argument << arg.description();
    argument << arg.defaultValue();
    argument << arg.properties();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtConfigOption &arg) {
    QString name, description, type;
    QDBusVariant defaultValue;
    QVariantMap properties;
    argument.beginStructure();
    argument >> name >> type >> description >> defaultValue >> properties;
    argument.endStructure();
    arg.setName(name);
    arg.setType(type);
    arg.setDescription(description);
    arg.setDefaultValue(defaultValue);
    arg.setProperties(properties);
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtConfigType &arg) {
    argument.beginStructure();
    argument << arg.name();
    argument << arg.options();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtConfigType &arg) {
    QString name;
    FcitxQtConfigOptionList options;
    argument.beginStructure();
    argument >> name >> options;
    argument.endStructure();
    arg.setName(name);
    arg.setOptions(options);
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtAddonInfo &arg) {
    argument.beginStructure();
    argument << arg.uniqueName();
    argument << arg.name();
    argument << arg.comment();
    argument << arg.category();
    argument << arg.configurable();
    argument << arg.enabled();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtAddonInfo &arg) {
    QString uniqueName, name, comment;
    int category;
    bool configurable, enabled;
    argument.beginStructure();
    argument >> uniqueName >> name >> comment >> category >> configurable >>
        enabled;
    argument.endStructure();
    arg.setUniqueName(uniqueName);
    arg.setName(name);
    arg.setComment(comment);
    arg.setCategory(category);
    arg.setConfigurable(configurable);
    arg.setEnabled(enabled);
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtAddonInfoV2 &arg) {
    argument.beginStructure();
    argument << arg.uniqueName();
    argument << arg.name();
    argument << arg.comment();
    argument << arg.category();
    argument << arg.configurable();
    argument << arg.enabled();
    argument << arg.onDemand();
    argument << arg.dependencies();
    argument << arg.optionalDependencies();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtAddonInfoV2 &arg) {
    QString uniqueName, name, comment;
    int category;
    bool configurable, enabled, onDemand;
    QStringList dependencies, optionalDependencies;
    argument.beginStructure();
    argument >> uniqueName >> name >> comment >> category >> configurable >>
        enabled >> onDemand >> dependencies >> optionalDependencies;
    argument.endStructure();
    arg.setUniqueName(uniqueName);
    arg.setName(name);
    arg.setComment(comment);
    arg.setCategory(category);
    arg.setConfigurable(configurable);
    arg.setEnabled(enabled);
    arg.setOnDemand(onDemand);
    arg.setDependencies(dependencies);
    arg.setOptionalDependencies(optionalDependencies);
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtAddonState &arg) {
    argument.beginStructure();
    argument << arg.uniqueName();
    argument << arg.enabled();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtAddonState &arg) {
    QString uniqueName;
    bool enabled;
    argument.beginStructure();
    argument >> uniqueName >> enabled;
    argument.endStructure();
    arg.setUniqueName(uniqueName);
    arg.setEnabled(enabled);
    return argument;
}
} // namespace fcitx
