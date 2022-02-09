/*****************************************************************************
 * qeasysettings.cpp
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
#include "qeasysettings.hpp"
#include <QTimer>

#ifdef NDEBUG
#undef NDEBUG
#include <cassert>
#endif

// Use (void) to silent unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

#ifndef QT_QUICK_LIB
#include <QApplication>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QDesktopWidget>
#endif
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QToolTip>
#else
#include <QQuickStyle>
#endif

QEasySettings *QEasySettings::m_instance = nullptr;
bool QEasySettings::m_autoPalette = false;
bool QEasySettings::m_islight;
#ifndef QT_QUICK_LIB
QPalette QEasySettings::mPalette;
#endif
QSettings QEasySettings::m_settings{
    "HKEY_CURRENT_"
    "USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
    QSettings::NativeFormat};
QTimer QEasySettings::mTimer;

void QEasySettings::init(QEasySettings::Format format, const QString &name) {
  if (!m_instance) {
    m_instance = new QEasySettings(format, name);
    sConnectionCallback(*m_instance);
  }
}

QStringList QEasySettings::supportedStyles()
{
    QStringList result;
    result << "auto Fusion" << "Vista" << "Classic" << "Light Fusion" << "Dark Fusion";
    return result;
}

QEasySettings::~QEasySettings() {
  delete m_settingsObj;
  delete m_instance;
}

#ifndef QT_QUICK_LIB
QEasySettings::Style QEasySettings::readStyle() {
  assertm(m_instance, "Call QEasySettings::init() function first!");
  int val;
  m_instance->m_settingsObj->beginGroup("Style");
  val = m_instance->m_settingsObj
            ->value("Style", static_cast<int>(Style::lightFusion))
            .toInt(); // default Style is lightFusion
  m_instance->m_settingsObj->endGroup();
  return static_cast<Style>(val);
}

void QEasySettings::setStyle(const QEasySettings::Style val) {
  switch (val) {
  case Style::autoFusion:
    qApp->setStyle(QStyleFactory::create("Fusion"));
    setAutoPalette(true);
    break;
  case Style::vista:
    qApp->setStyle(QStyleFactory::create("windowsvista"));
    setAutoPalette(false);
    changePalette(Palette::light);
    break;
  case Style::classic:
    qApp->setStyle(QStyleFactory::create("windows"));
    setAutoPalette(false);
    changePalette(Palette::light);
    break;
  case Style::lightFusion:
    qApp->setStyle(QStyleFactory::create("Fusion"));
    setAutoPalette(false);
    changePalette(Palette::light);
    break;
  case Style::darkFusion:
    qApp->setStyle(QStyleFactory::create("Fusion"));
    setAutoPalette(false);
    changePalette(Palette::dark);
    break;
  default:
    break;
  }
}
#endif

QVariant QEasySettings::readSettings(const QString group, const QString key) {
  assertm(m_instance, "Call QEasySettings::init() function first!");
  QVariant val;
  m_instance->m_settingsObj->beginGroup(group);
  val = m_instance->m_settingsObj->value(key);
  m_instance->m_settingsObj->endGroup();
  return val;
}

#ifdef QT_QUICK_LIB
void QEasySettings::writeSettings(const QString group, const QString key,
                                  const QVariant &option) {
  assertm(m_instance, "Call QEasySettings::init() function first!");
  m_instance->m_settingsObj->beginGroup(group);
  m_instance->m_settingsObj->setValue(key, option);
  m_instance->m_settingsObj->endGroup();
}
#endif

QEasySettings *QEasySettings::instance() { 
    assertm(m_instance, "Call QEasySettings::init() function first!");
    return m_instance; 
}

#ifndef QT_QUICK_LIB
void QEasySettings::writeStyle(const QEasySettings::Style &option) {
  assertm(m_instance, "Call QEasySettings::init() function first!");
  m_instance->m_settingsObj->beginGroup("Style");
  m_instance->m_settingsObj->setValue("Style", static_cast<int>(option));
  m_instance->m_settingsObj->endGroup();
}
#endif

QEasySettings::QEasySettings(QEasySettings::Format format,
                             const QString &name) {
  (static_cast<bool>(format))
      ? m_settingsObj = new QSettings(name, QSettings::IniFormat)
      : m_settingsObj = new QSettings(name, name);
}

void QEasySettings::setAutoPalette(bool autoPalette) {
  m_autoPalette = autoPalette;
  setupEventLoop(autoPalette);
}
#ifdef QT_QUICK_LIB
void QEasySettings::writeStyle(const QString &style, const QString &theme) {
  assertm(m_instance, "Call QEasySettings::init() function first!");
  m_instance->m_settingsObj->beginGroup("Controls");
  m_instance->m_settingsObj->setValue("Style", style);
  m_instance->m_settingsObj->endGroup();

  m_instance->m_settingsObj->beginGroup(style);
  m_instance->m_settingsObj->setValue("Theme", theme);
  m_instance->m_settingsObj->endGroup();
}

void QEasySettings::setStyle(const QString &style) {
  QQuickStyle::setStyle(style);
}

QString QEasySettings::readTheme() {
  assertm(m_instance, "Call QEasySettings::init() function first!");
  m_instance->m_settingsObj->beginGroup("Controls");
  auto val = m_instance->m_settingsObj->value("Style", "Default").toString();
  m_instance->m_settingsObj->endGroup();
  m_instance->m_settingsObj->beginGroup(val);
  val = m_instance->m_settingsObj->value("Theme", "System").toString();
  m_instance->m_settingsObj->endGroup();
  return val;
}

QString QEasySettings::readStyle() {
  assertm(m_instance, "Call QEasySettings::init() function first!");
  m_instance->m_settingsObj->beginGroup("Controls");
  auto val = m_instance->m_settingsObj->value("Style", "Default").toString();
  m_instance->m_settingsObj->endGroup();
  return val;
}
#endif

#ifndef QT_QUICK_LIB
void QEasySettings::changePalette(QEasySettings::Palette _palette) {
  if (static_cast<bool>(_palette)) {
    mPalette = qApp->style()->standardPalette();
  } else {
    mPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    mPalette.setColor(QPalette::WindowText, Qt::white);
    mPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    mPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    mPalette.setColor(QPalette::ToolTipBase, QColor(53, 53, 53));
    mPalette.setColor(QPalette::ToolTipText, Qt::white);
    mPalette.setColor(QPalette::Text, Qt::white);
    mPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    mPalette.setColor(QPalette::ButtonText, Qt::white);
    mPalette.setColor(QPalette::BrightText, Qt::red);
    mPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    mPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    mPalette.setColor(QPalette::HighlightedText, Qt::black);
    mPalette.setColor(QPalette::Disabled, QPalette::Text,
                      QColor(164, 166, 168));
    mPalette.setColor(QPalette::Disabled, QPalette::WindowText,
                      QColor(164, 166, 168));
    mPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                      QColor(164, 166, 168));
    mPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                      QColor(164, 166, 168));
    mPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(68, 68, 68));
    mPalette.setColor(QPalette::Disabled, QPalette::Window, QColor(68, 68, 68));
    mPalette.setColor(QPalette::Disabled, QPalette::Highlight,
                      QColor(68, 68, 68));
  }

  QToolTip::setPalette(mPalette);
  qApp->setPalette(mPalette);
}
#endif

void QEasySettings::sConnectionCallback(QEasySettings &s) {
#ifndef QT_QUICK_LIB
  connect(&s, SIGNAL(notifyPalette(bool)), &s, SLOT(bool2PaletteHelper(bool)),
          Qt::QueuedConnection);
#endif
  connect(&mTimer, &QTimer::timeout, &s, &QEasySettings::eventLoop);
  mTimer.setInterval(100);
}

#ifndef QT_QUICK_LIB
void QEasySettings::bool2PaletteHelper(bool b) {
  changePalette(static_cast<enum Palette>(b));
}
#endif

void QEasySettings::setupEventLoop(const bool &event) {
  m_autoPalette = event;
  event ? mTimer.start() : mTimer.stop();
}

void QEasySettings::eventLoop() {
  if (m_autoPalette) {
    auto const temp = m_settings.value("AppsUseLightTheme", true).toBool();
    if (temp == m_islight)
      sigHandler(temp, *m_instance);
    m_islight = temp;
  }
}

void QEasySettings::sigHandler(bool b, QEasySettings &s) {
  Q_EMIT s.notifyPalette(b);
}
