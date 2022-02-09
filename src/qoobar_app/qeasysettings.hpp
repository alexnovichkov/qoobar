/*****************************************************************************
 * qeasysettings.hpp
 *
 * Created: 12/27/2020 2020 by mguludag
 *
 * Copyright 2020 mguludag. All rights reserved.
 *
 * This file may be distributed under the terms of GNU Public License version
 * 3 (GPL v3) as defined by the Free Software Foundation (FSF). A copy of the
 * license should have been included with this file, or the project in which
 * this file belongs to. You may also find the details of GPL v3 at:
 * http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * If you have any questions regarding the use of this file, feel free to
 * contact the author of this file, or the owner of the project in which
 * this file belongs to.
 *****************************************************************************/
#ifndef QEASYSETTINGS_HPP
#define QEASYSETTINGS_HPP

#include <QSettings>

class QPalette;
class QTimer;

class QEasySettings : public QObject {
    Q_OBJECT
public:
    /*********************************ENUMS************************************/
    enum class Style { autoFusion, vista, classic, lightFusion, darkFusion };
    enum class Format { regFormat, iniFormat };


    /****************************PUBLIC FUNCTIONS******************************/
    /*!
     * \fn void init
     * \brief Inits the settings object with desired format and name
     * \param format
     * \param name
     */
    static void init(Format format, const QString &name);

    static QStringList supportedStyles();

    ~QEasySettings();

#ifndef QT_QUICK_LIB
    /*!
     * \fn int loadStyle
     * \brief Loads current application style from settings file or registry
     */
    static Style readStyle();

    /*!
     * \fn void setStyle
     * \brief Apply a given style to application
     */
    static void setStyle(const Style val=Style::lightFusion);

    /*!
     * \fn QVariant readSettings
     * \brief Read a value stored in settings given group and key
     * \param group
     * \param key
     */
    static QVariant readSettings(const QString group,
                                             const QString key);

    template <typename T>
    /*!
     * \fn void writeSettings
     * \brief Write a value in settings given group and key
     */
    static void writeSettings(const QString group, const QString key,
                              const T &option) {
        m_instance->m_settingsObj->beginGroup(group);
        m_instance->m_settingsObj->setValue(key, option);
        m_instance->m_settingsObj->endGroup();
    }

    /*!
     * \fn void writeStyle
     * \brief Write the style settings in file or registry
     * \param option
     */
    static void writeStyle(const enum Style &option);
#else
    /*!
     * \fn QVariant readSettings
     * \brief Read a value stored in settings given group and key
     * \param group
     * \param key
     */
    Q_INVOKABLE static QVariant readSettings(const QString group,
                                             const QString key);

    /*!
     * \fn void writeSettings
     * \brief Write a value in settings given group and key
     * \param group
     * \param key
     * \param option
     */
    Q_INVOKABLE static void writeSettings(const QString group, const QString key,
                                          const QVariant &option);
    /*!
     * \fn void writeStyle
     * \brief Write the style settings in file or registry
     * \param style
     * \param theme
     */
    Q_INVOKABLE static void writeStyle(const QString &style, const QString &theme);

    /*!
     * \fn void loadStyle
     * \brief Load and apply QML style from settings file or registry or 
     * if not retrieve this setting it uses the parameter
     */
    Q_INVOKABLE static void setStyle(const QString &style="Default");

    /*!
     * \fn QString readTheme
     * \brief Read QML Theme name from settings file or registry
     * \return QString
     */
    Q_INVOKABLE static QString readTheme();

    /*!
     * \fn QString readStyle
     * \brief Read QML Style name from settings file or registry
     * \return
     */
    Q_INVOKABLE static QString readStyle();
#endif

    /*!
     * \fn QEasySettings* instance
     * \brief Returns instance object for forwarding to QML
     * \return
     */
    static QEasySettings *instance();

    /*!
     * \fn void setAutoPalette
     * \brief Set true to auto follow Windows 10 dark/light mode changes
     * \param autoPalette
     */
    Q_INVOKABLE static void setAutoPalette(bool autoPalette);



    /*************************PRIVATE MEMBERS**********************************/
private:
    enum class Palette { dark, light };
    static QEasySettings *m_instance;
    QSettings *m_settingsObj = nullptr;
#ifndef QT_QUICK_LIB
    static QPalette mPalette;
#endif
    static bool m_autoPalette;
    static QSettings m_settings;
    static QTimer mTimer;
    static bool m_islight;

    QEasySettings() = delete;
    QEasySettings(Format format, const QString &name);
#ifndef QT_QUICK_LIB
    static void changePalette(enum Palette _palette);
#endif
    static void sConnectionCallback(QEasySettings &s);

    /********************************SIGNALS***********************************/
Q_SIGNALS:
    /*!
     * \fn void notifyPalette
     * \brief Signal for notify Windows 10 dark/light mode changes
     * \param isLight bool
     */
    void notifyPalette(bool isLight);

    /****************************PRIVATE SLOTS*********************************/
private Q_SLOTS:
#ifndef QT_QUICK_LIB
    static void bool2PaletteHelper(bool b);
#endif
    static void setupEventLoop(const bool &event);
    static void eventLoop();
    static void sigHandler(bool b, QEasySettings &s);
};

#endif // QEASYSETTINGS_HPP
