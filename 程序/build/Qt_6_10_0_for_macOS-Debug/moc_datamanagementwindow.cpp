/****************************************************************************
** Meta object code from reading C++ file 'datamanagementwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../datamanagementwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'datamanagementwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.0. It"
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
struct qt_meta_tag_ZN20DataManagementWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto DataManagementWindow::qt_create_metaobjectdata<qt_meta_tag_ZN20DataManagementWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DataManagementWindow",
        "refreshData",
        "",
        "onNodeFilterChanged",
        "onEdgeFilterChanged",
        "onAddNode",
        "onEditNode",
        "onDeleteNode",
        "onAddEdge",
        "onEditEdge",
        "onDeleteEdge",
        "onExportData",
        "onImportData",
        "onBatchDelete",
        "onLoadFromDatabase",
        "onSaveToDatabase",
        "onPasteImport",
        "onNodeTableSelectionChanged",
        "onEdgeTableSelectionChanged",
        "onNodeTableDoubleClicked",
        "row",
        "column",
        "onEdgeTableDoubleClicked",
        "onNodePrevPage",
        "onNodeNextPage",
        "onEdgePrevPage",
        "onEdgeNextPage"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'refreshData'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onNodeFilterChanged'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onEdgeFilterChanged'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onAddNode'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onEditNode'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onDeleteNode'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onAddEdge'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onEditEdge'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onDeleteEdge'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onExportData'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onImportData'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onBatchDelete'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onLoadFromDatabase'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onSaveToDatabase'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onPasteImport'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onNodeTableSelectionChanged'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEdgeTableSelectionChanged'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNodeTableDoubleClicked'
        QtMocHelpers::SlotData<void(int, int)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 20 }, { QMetaType::Int, 21 },
        }}),
        // Slot 'onEdgeTableDoubleClicked'
        QtMocHelpers::SlotData<void(int, int)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 20 }, { QMetaType::Int, 21 },
        }}),
        // Slot 'onNodePrevPage'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNodeNextPage'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEdgePrevPage'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEdgeNextPage'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DataManagementWindow, qt_meta_tag_ZN20DataManagementWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DataManagementWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20DataManagementWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20DataManagementWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN20DataManagementWindowE_t>.metaTypes,
    nullptr
} };

void DataManagementWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DataManagementWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->refreshData(); break;
        case 1: _t->onNodeFilterChanged(); break;
        case 2: _t->onEdgeFilterChanged(); break;
        case 3: _t->onAddNode(); break;
        case 4: _t->onEditNode(); break;
        case 5: _t->onDeleteNode(); break;
        case 6: _t->onAddEdge(); break;
        case 7: _t->onEditEdge(); break;
        case 8: _t->onDeleteEdge(); break;
        case 9: _t->onExportData(); break;
        case 10: _t->onImportData(); break;
        case 11: _t->onBatchDelete(); break;
        case 12: _t->onLoadFromDatabase(); break;
        case 13: _t->onSaveToDatabase(); break;
        case 14: _t->onPasteImport(); break;
        case 15: _t->onNodeTableSelectionChanged(); break;
        case 16: _t->onEdgeTableSelectionChanged(); break;
        case 17: _t->onNodeTableDoubleClicked((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 18: _t->onEdgeTableDoubleClicked((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 19: _t->onNodePrevPage(); break;
        case 20: _t->onNodeNextPage(); break;
        case 21: _t->onEdgePrevPage(); break;
        case 22: _t->onEdgeNextPage(); break;
        default: ;
        }
    }
}

const QMetaObject *DataManagementWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DataManagementWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20DataManagementWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int DataManagementWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 23)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 23;
    }
    return _id;
}
QT_WARNING_POP
