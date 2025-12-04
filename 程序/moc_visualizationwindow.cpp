/****************************************************************************
** Meta object code from reading C++ file 'visualizationwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "visualizationwindow.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'visualizationwindow.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN19VisualizationWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto VisualizationWindow::qt_create_metaobjectdata<qt_meta_tag_ZN19VisualizationWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "VisualizationWindow",
        "graphDataChanged",
        "",
        "calcResultReady",
        "resultText",
        "updateGraph",
        "highlightPath",
        "QList<long>",
        "path",
        "startAnimation",
        "startNode",
        "endNode",
        "onNodeLabelChanged",
        "onLayoutChanged",
        "zoomIn",
        "zoomOut",
        "resetZoom",
        "onQuickCalculate",
        "onQuickAddEdge",
        "showCalculationResult",
        "startId",
        "endId",
        "distance",
        "onShowHelp",
        "onAnimationStep",
        "onAnimationFinished",
        "onSelectionChanged",
        "onSaveLayoutToDatabase",
        "onCalcInputChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'graphDataChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'calcResultReady'
        QtMocHelpers::SignalData<void(const QString &)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 4 },
        }}),
        // Slot 'updateGraph'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'highlightPath'
        QtMocHelpers::SlotData<void(const QVector<long> &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Slot 'startAnimation'
        QtMocHelpers::SlotData<void(long, long)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Long, 10 }, { QMetaType::Long, 11 },
        }}),
        // Slot 'onNodeLabelChanged'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onLayoutChanged'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'zoomIn'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'zoomOut'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'resetZoom'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onQuickCalculate'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onQuickAddEdge'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'showCalculationResult'
        QtMocHelpers::SlotData<void(long, long, long, const QVector<long> &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Long, 20 }, { QMetaType::Long, 21 }, { QMetaType::Long, 22 }, { 0x80000000 | 7, 8 },
        }}),
        // Slot 'onShowHelp'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onAnimationStep'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAnimationFinished'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSelectionChanged'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSaveLayoutToDatabase'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCalcInputChanged'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<VisualizationWindow, qt_meta_tag_ZN19VisualizationWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject VisualizationWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19VisualizationWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19VisualizationWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN19VisualizationWindowE_t>.metaTypes,
    nullptr
} };

void VisualizationWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<VisualizationWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->graphDataChanged(); break;
        case 1: _t->calcResultReady((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->updateGraph(); break;
        case 3: _t->highlightPath((*reinterpret_cast<std::add_pointer_t<QList<long>>>(_a[1]))); break;
        case 4: _t->startAnimation((*reinterpret_cast<std::add_pointer_t<long>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<long>>(_a[2]))); break;
        case 5: _t->onNodeLabelChanged(); break;
        case 6: _t->onLayoutChanged(); break;
        case 7: _t->zoomIn(); break;
        case 8: _t->zoomOut(); break;
        case 9: _t->resetZoom(); break;
        case 10: _t->onQuickCalculate(); break;
        case 11: _t->onQuickAddEdge(); break;
        case 12: _t->showCalculationResult((*reinterpret_cast<std::add_pointer_t<long>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<long>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<long>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QList<long>>>(_a[4]))); break;
        case 13: _t->onShowHelp(); break;
        case 14: _t->onAnimationStep(); break;
        case 15: _t->onAnimationFinished(); break;
        case 16: _t->onSelectionChanged(); break;
        case 17: _t->onSaveLayoutToDatabase(); break;
        case 18: _t->onCalcInputChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<long> >(); break;
            }
            break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 3:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<long> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (VisualizationWindow::*)()>(_a, &VisualizationWindow::graphDataChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (VisualizationWindow::*)(const QString & )>(_a, &VisualizationWindow::calcResultReady, 1))
            return;
    }
}

const QMetaObject *VisualizationWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VisualizationWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19VisualizationWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int VisualizationWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void VisualizationWindow::graphDataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void VisualizationWindow::calcResultReady(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}
QT_WARNING_POP
