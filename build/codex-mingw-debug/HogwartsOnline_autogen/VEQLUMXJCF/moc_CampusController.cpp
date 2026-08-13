/****************************************************************************
** Meta object code from reading C++ file 'CampusController.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/application/controller/CampusController.hpp"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CampusController.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN6arcane11application10controller16CampusControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto arcane::application::controller::CampusController::qt_create_metaobjectdata<qt_meta_tag_ZN6arcane11application10controller16CampusControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "arcane::application::controller::CampusController",
        "loginAccepted",
        "",
        "studentName",
        "house",
        "campusMessageProduced",
        "channel",
        "speaker",
        "text",
        "playerLocationChanged",
        "location",
        "state",
        "feedbackProduced",
        "handleLogin",
        "handleChat",
        "handleMove",
        "locationId",
        "handleCourseJoin",
        "courseId",
        "handleCourseDetails",
        "handleProfessorInfo",
        "professorId",
        "handleItemUse",
        "itemId",
        "handleItemInspect",
        "handleItemGift",
        "npcId",
        "handleStudy",
        "handleActivity",
        "activityId",
        "handleMemberSelected",
        "memberId",
        "handlePrivateChat",
        "handleProfile"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'loginAccepted'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'campusMessageProduced'
        QtMocHelpers::SignalData<void(const QString &, const QString &, const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::QString, 7 }, { QMetaType::QString, 8 },
        }}),
        // Signal 'playerLocationChanged'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 }, { QMetaType::QString, 11 },
        }}),
        // Signal 'feedbackProduced'
        QtMocHelpers::SignalData<void(const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Slot 'handleLogin'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QString, 4 },
        }}),
        // Slot 'handleChat'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::QString, 8 },
        }}),
        // Slot 'handleMove'
        QtMocHelpers::SlotData<void(const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 16 },
        }}),
        // Slot 'handleCourseJoin'
        QtMocHelpers::SlotData<void(const QString &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 18 },
        }}),
        // Slot 'handleCourseDetails'
        QtMocHelpers::SlotData<void(const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 18 },
        }}),
        // Slot 'handleProfessorInfo'
        QtMocHelpers::SlotData<void(const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 },
        }}),
        // Slot 'handleItemUse'
        QtMocHelpers::SlotData<void(const QString &)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 23 },
        }}),
        // Slot 'handleItemInspect'
        QtMocHelpers::SlotData<void(const QString &)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 23 },
        }}),
        // Slot 'handleItemGift'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 23 }, { QMetaType::QString, 26 },
        }}),
        // Slot 'handleStudy'
        QtMocHelpers::SlotData<void(const QString &)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 16 },
        }}),
        // Slot 'handleActivity'
        QtMocHelpers::SlotData<void(const QString &)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 29 },
        }}),
        // Slot 'handleMemberSelected'
        QtMocHelpers::SlotData<void(const QString &)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 31 },
        }}),
        // Slot 'handlePrivateChat'
        QtMocHelpers::SlotData<void(const QString &)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 31 },
        }}),
        // Slot 'handleProfile'
        QtMocHelpers::SlotData<void(const QString &)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 31 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CampusController, qt_meta_tag_ZN6arcane11application10controller16CampusControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject arcane::application::controller::CampusController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6arcane11application10controller16CampusControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6arcane11application10controller16CampusControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6arcane11application10controller16CampusControllerE_t>.metaTypes,
    nullptr
} };

void arcane::application::controller::CampusController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CampusController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->loginAccepted((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->campusMessageProduced((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 2: _t->playerLocationChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->feedbackProduced((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->handleLogin((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->handleChat((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 6: _t->handleMove((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->handleCourseJoin((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->handleCourseDetails((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->handleProfessorInfo((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->handleItemUse((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->handleItemInspect((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->handleItemGift((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 13: _t->handleStudy((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->handleActivity((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->handleMemberSelected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->handlePrivateChat((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->handleProfile((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CampusController::*)(const QString & , const QString & )>(_a, &CampusController::loginAccepted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CampusController::*)(const QString & , const QString & , const QString & )>(_a, &CampusController::campusMessageProduced, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CampusController::*)(const QString & , const QString & )>(_a, &CampusController::playerLocationChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CampusController::*)(const QString & )>(_a, &CampusController::feedbackProduced, 3))
            return;
    }
}

const QMetaObject *arcane::application::controller::CampusController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *arcane::application::controller::CampusController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6arcane11application10controller16CampusControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int arcane::application::controller::CampusController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void arcane::application::controller::CampusController::loginAccepted(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void arcane::application::controller::CampusController::campusMessageProduced(const QString & _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3);
}

// SIGNAL 2
void arcane::application::controller::CampusController::playerLocationChanged(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void arcane::application::controller::CampusController::feedbackProduced(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
