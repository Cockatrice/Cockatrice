/****************************************************************************
** Meta object code from reading C++ file 'dlg_settings.h'
**
** Created: Fri Jul 31 01:27:03 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/dlg_settings.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dlg_settings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GeneralSettingsPage[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      26,   21,   20,   20, 0x05,
      51,   21,   20,   20, 0x05,
      84,   21,   20,   20, 0x05,
     126,  119,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
     150,   20,   20,   20, 0x08,
     174,   20,   20,   20, 0x08,
     198,   20,   20,   20, 0x08,
     230,   20,   20,   20, 0x08,
     270,  264,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_GeneralSettingsPage[] = {
    "GeneralSettingsPage\0\0path\0"
    "picsPathChanged(QString)\0"
    "cardDatabasePathChanged(QString)\0"
    "cardBackgroundPathChanged(QString)\0"
    "qmFile\0changeLanguage(QString)\0"
    "deckPathButtonClicked()\0picsPathButtonClicked()\0"
    "cardDatabasePathButtonClicked()\0"
    "cardBackgroundPathButtonClicked()\0"
    "index\0languageBoxChanged(int)\0"
};

const QMetaObject GeneralSettingsPage::staticMetaObject = {
    { &AbstractSettingsPage::staticMetaObject, qt_meta_stringdata_GeneralSettingsPage,
      qt_meta_data_GeneralSettingsPage, 0 }
};

const QMetaObject *GeneralSettingsPage::metaObject() const
{
    return &staticMetaObject;
}

void *GeneralSettingsPage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GeneralSettingsPage))
        return static_cast<void*>(const_cast< GeneralSettingsPage*>(this));
    return AbstractSettingsPage::qt_metacast(_clname);
}

int GeneralSettingsPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractSettingsPage::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: picsPathChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: cardDatabasePathChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: cardBackgroundPathChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: changeLanguage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: deckPathButtonClicked(); break;
        case 5: picsPathButtonClicked(); break;
        case 6: cardDatabasePathButtonClicked(); break;
        case 7: cardBackgroundPathButtonClicked(); break;
        case 8: languageBoxChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void GeneralSettingsPage::picsPathChanged(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GeneralSettingsPage::cardDatabasePathChanged(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void GeneralSettingsPage::cardBackgroundPathChanged(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void GeneralSettingsPage::changeLanguage(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
static const uint qt_meta_data_AppearanceSettingsPage[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

       0        // eod
};

static const char qt_meta_stringdata_AppearanceSettingsPage[] = {
    "AppearanceSettingsPage\0"
};

const QMetaObject AppearanceSettingsPage::staticMetaObject = {
    { &AbstractSettingsPage::staticMetaObject, qt_meta_stringdata_AppearanceSettingsPage,
      qt_meta_data_AppearanceSettingsPage, 0 }
};

const QMetaObject *AppearanceSettingsPage::metaObject() const
{
    return &staticMetaObject;
}

void *AppearanceSettingsPage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AppearanceSettingsPage))
        return static_cast<void*>(const_cast< AppearanceSettingsPage*>(this));
    return AbstractSettingsPage::qt_metacast(_clname);
}

int AppearanceSettingsPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractSettingsPage::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_MessagesSettingsPage[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x08,
      31,   21,   21,   21, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MessagesSettingsPage[] = {
    "MessagesSettingsPage\0\0actAdd()\0"
    "actRemove()\0"
};

const QMetaObject MessagesSettingsPage::staticMetaObject = {
    { &AbstractSettingsPage::staticMetaObject, qt_meta_stringdata_MessagesSettingsPage,
      qt_meta_data_MessagesSettingsPage, 0 }
};

const QMetaObject *MessagesSettingsPage::metaObject() const
{
    return &staticMetaObject;
}

void *MessagesSettingsPage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MessagesSettingsPage))
        return static_cast<void*>(const_cast< MessagesSettingsPage*>(this));
    return AbstractSettingsPage::qt_metacast(_clname);
}

int MessagesSettingsPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractSettingsPage::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: actAdd(); break;
        case 1: actRemove(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_DlgSettings[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      30,   13,   12,   12, 0x08,
      83,   76,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DlgSettings[] = {
    "DlgSettings\0\0current,previous\0"
    "changePage(QListWidgetItem*,QListWidgetItem*)\0"
    "qmFile\0changeLanguage(QString)\0"
};

const QMetaObject DlgSettings::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DlgSettings,
      qt_meta_data_DlgSettings, 0 }
};

const QMetaObject *DlgSettings::metaObject() const
{
    return &staticMetaObject;
}

void *DlgSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DlgSettings))
        return static_cast<void*>(const_cast< DlgSettings*>(this));
    return QDialog::qt_metacast(_clname);
}

int DlgSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: changePage((*reinterpret_cast< QListWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QListWidgetItem*(*)>(_a[2]))); break;
        case 1: changeLanguage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
