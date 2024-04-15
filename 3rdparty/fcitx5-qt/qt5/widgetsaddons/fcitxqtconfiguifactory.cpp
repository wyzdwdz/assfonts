/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "fcitxqtconfiguifactory.h"
#include "fcitxqtconfiguifactory_p.h"
#include "fcitxqtconfiguiplugin.h"

#include <QDebug>
#include <QDir>
#include <QLibrary>
#include <QPluginLoader>
#include <QStandardPaths>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>

namespace fcitx {

namespace {

constexpr char addonConfigPrefix[] = "fcitx://config/addon/";

QString normalizePath(const QString &file) {
    auto path = file;
    if (path.startsWith(addonConfigPrefix)) {
        path.remove(0, sizeof(addonConfigPrefix) - 1);
    }
    return path;
}

} // namespace

FcitxQtConfigUIFactoryPrivate::FcitxQtConfigUIFactoryPrivate(
    FcitxQtConfigUIFactory *factory)
    : QObject(factory), q_ptr(factory) {}

FcitxQtConfigUIFactoryPrivate::~FcitxQtConfigUIFactoryPrivate() {}

FcitxQtConfigUIFactory::FcitxQtConfigUIFactory(QObject *parent)
    : QObject(parent), d_ptr(new FcitxQtConfigUIFactoryPrivate(this)) {
    Q_D(FcitxQtConfigUIFactory);
    d->scan();
}

FcitxQtConfigUIFactory::~FcitxQtConfigUIFactory() {}

FcitxQtConfigUIWidget *FcitxQtConfigUIFactory::create(const QString &file) {
    Q_D(FcitxQtConfigUIFactory);

    auto path = normalizePath(file);
    auto loader = d->plugins_.value(path);
    if (!loader) {
        return nullptr;
    }

    auto instance =
        qobject_cast<FcitxQtConfigUIFactoryInterface *>(loader->instance());
    if (!instance) {
        return nullptr;
    }
    return instance->create(path.section('/', 1));
}

bool FcitxQtConfigUIFactory::test(const QString &file) {
    Q_D(FcitxQtConfigUIFactory);

    auto path = normalizePath(file);
    return d->plugins_.contains(path);
}

void FcitxQtConfigUIFactoryPrivate::scan() {
    fcitx::StandardPath::global().scanFiles(
        fcitx::StandardPath::Type::Addon, "qt5",
        [this](const std::string &path, const std::string &dirPath, bool user) {
            do {
                if (user) {
                    break;
                }

                QDir dir(QString::fromLocal8Bit(dirPath.c_str()));
                QFileInfo fi(
                    dir.filePath(QString::fromLocal8Bit(path.c_str())));

                QString filePath = fi.filePath(); // file name with path
                QString fileName = fi.fileName(); // just file name

                if (!QLibrary::isLibrary(filePath)) {
                    break;
                }

                QPluginLoader *loader = new QPluginLoader(filePath, this);
                if (loader->metaData().value("IID") !=
                    QLatin1String(FcitxQtConfigUIFactoryInterface_iid)) {
                    delete loader;
                    break;
                }
                auto metadata = loader->metaData().value("MetaData").toObject();
                auto files = metadata.value("files").toVariant().toStringList();
                auto addon = metadata.value("addon").toVariant().toString();
                for (const auto &file : files) {
                    plugins_[addon + "/" + file] = loader;
                }
            } while (0);
            return true;
        });
}
} // namespace fcitx
